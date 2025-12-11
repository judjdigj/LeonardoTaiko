import serial
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import threading
import queue
from matplotlib.widgets import Slider, Button

class AdvancedSerialPlotter:
    def __init__(self, port, baud_rate=9600, max_data_points=1000, initial_time_window=30):
        self.port = port
        self.baud_rate = baud_rate
        self.max_data_points = max_data_points
        self.time_window = initial_time_window  # 初始时间窗口（秒）
        
        # 初始化数据存储队列
        self.time_data = deque(maxlen=max_data_points)
        self.A0_data = deque(maxlen=max_data_points)
        self.A1_data = deque(maxlen=max_data_points)
        self.A2_data = deque(maxlen=max_data_points)
        self.A3_data = deque(maxlen=max_data_points)
        
        # 数据队列用于线程安全
        self.data_queue = queue.Queue()
        
        # 初始化时间戳
        self.start_time = time.time()
        self.last_update_time = self.start_time
        
        # 创建图形和布局
        self.fig = plt.figure(figsize=(14, 10))
        
        # 主图表区域（留出空间给控件）
        self.ax = plt.axes([0.1, 0.25, 0.85, 0.65])
        
        # 设置图形属性
        self.fig.suptitle('Arduino Analog Input Real-time Monitoring', fontsize=16)
        self.ax.set_xlabel('Time (seconds)', fontsize=12)
        self.ax.set_ylabel('Analog Value (0-1024)', fontsize=12)
        self.ax.set_ylim(-50, 300)
        self.ax.grid(True, alpha=0.3)
        
        # 创建4条曲线，不同颜色
        self.lines = []
        colors = ['blue', 'red', 'green', 'orange']
        labels = ['A0', 'A1', 'A2', 'A3']
        
        for color, label in zip(colors, labels):
            line, = self.ax.plot([], [], color=color, linewidth=2, label=label)
            self.lines.append(line)
        
        # 添加图例
        self.ax.legend(loc='upper right', fontsize=10)
        
        # 添加文本显示当前值
        self.text = self.ax.text(0.02, 0.95, '', transform=self.ax.transAxes, 
                                 fontsize=10, verticalalignment='top',
                                 bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
        
        # 添加统计信息文本
        self.stats_text = self.ax.text(0.02, 0.05, '', transform=self.ax.transAxes,
                                       fontsize=9, verticalalignment='bottom',
                                       bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.5))
        
        # 添加时间窗口滑块
        self.ax_slider = plt.axes([0.1, 0.15, 0.65, 0.03])
        self.time_slider = Slider(
            ax=self.ax_slider,
            label='Time Window (seconds)',
            valmin=2,
            valmax=300,  # 最大5分钟
            valinit=self.time_window,
            valstep=1
        )
        self.time_slider.on_changed(self.update_time_window)
        
        # 添加按钮
        self.ax_button = plt.axes([0.8, 0.15, 0.1, 0.03])
        self.reset_button = Button(self.ax_button, 'Reset View')
        self.reset_button.on_clicked(self.reset_view)
        
        # 添加缩放按钮
        self.ax_zoom_in = plt.axes([0.8, 0.10, 0.045, 0.03])
        self.ax_zoom_out = plt.axes([0.855, 0.10, 0.045, 0.03])
        self.zoom_in_button = Button(self.ax_zoom_in, '+')
        self.zoom_out_button = Button(self.ax_zoom_out, '-')
        self.zoom_in_button.on_clicked(self.zoom_in)
        self.zoom_out_button.on_clicked(self.zoom_out)
        
        # 初始化串口连接
        self.ser = None
        self.connect_serial()
        
        # 启动串口读取线程
        self.running = True
        self.serial_thread = threading.Thread(target=self.read_serial)
        self.serial_thread.daemon = True
        self.serial_thread.start()
    
    def connect_serial(self):
        """连接串口"""
        try:
            self.ser = serial.Serial(self.port, self.baud_rate, timeout=1)
            time.sleep(2)  # 等待Arduino重启
            print(f"Connected to {self.port} at {self.baud_rate} baud")
            print("Reading data...")
        except serial.SerialException as e:
            print(f"Error opening serial port: {e}")
            exit(1)
    
    def parse_data(self, data_string):
        """解析数据字符串，格式: 123||456||789||101"""
        try:
            parts = data_string.split('||')
            if len(parts) == 4:
                A0 = int(parts[0].strip())
                A1 = int(parts[1].strip())
                A2 = int(parts[2].strip())
                A3 = int(parts[3].strip())
                return A0, A1, A2, A3
        except (ValueError, IndexError) as e:
            print(f"Parse error: {e}, data: {data_string}")
        return None
    
    def read_serial(self):
        """在单独线程中读取串口数据"""
        while self.running:
            try:
                if self.ser and self.ser.in_waiting > 0:
                    # 读取一行数据
                    line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        # 解析数据并放入队列
                        parsed = self.parse_data(line)
                        if parsed:
                            self.data_queue.put(parsed)
            except Exception as e:
                print(f"Serial read error: {e}")
                time.sleep(0.1)
    
    def update_time_window(self, val):
        """更新时间窗口"""
        self.time_window = val
        print(f"Time window updated to {val} seconds")
    
    def reset_view(self, event):
        """重置视图"""
        self.time_slider.reset()
        self.time_window = self.time_slider.valinit
        print("View reset")
    
    def zoom_in(self, event):
        """放大时间窗口"""
        current_val = self.time_slider.val
        new_val = max(self.time_slider.valmin, current_val * 0.8)
        self.time_slider.set_val(new_val)
    
    def zoom_out(self, event):
        """缩小时间窗口"""
        current_val = self.time_slider.val
        new_val = min(self.time_slider.valmax, current_val * 1.2)
        self.time_slider.set_val(new_val)
    
    def calculate_statistics(self):
        """计算统计信息"""
        stats = []
        data_sets = [self.A0_data, self.A1_data, self.A2_data, self.A3_data]
        labels = ['A0', 'A1', 'A2', 'A3']
        
        for i, data in enumerate(data_sets):
            if len(data) > 0:
                avg = sum(data) / len(data)
                min_val = min(data)
                max_val = max(data)
                stats.append(f"{labels[i]}: Avg={avg:.1f}, Min={min_val}, Max={max_val}")
        
        return '\n'.join(stats)
    
    def update_plot(self, frame):
        """更新图形"""
        current_time = time.time()
        
        # 从队列中获取所有新数据
        new_data_count = 0
        while not self.data_queue.empty():
            try:
                A0, A1, A2, A3 = self.data_queue.get_nowait()
                
                # 添加到数据队列
                self.time_data.append(current_time - self.start_time)
                self.A0_data.append(A0)
                self.A1_data.append(A1)
                self.A2_data.append(A2)
                self.A3_data.append(A3)
                
                new_data_count += 1
                
            except queue.Empty:
                break
        
        # 如果有数据，更新曲线
        if self.time_data:
            time_list = list(self.time_data)
            
            # 更新四条曲线
            self.lines[0].set_data(time_list, list(self.A0_data))
            self.lines[1].set_data(time_list, list(self.A1_data))
            self.lines[2].set_data(time_list, list(self.A2_data))
            self.lines[3].set_data(time_list, list(self.A3_data))
            
            # 更新文本显示当前值
            if self.A0_data:
                current_values = (f"A0: {self.A0_data[-1]:4d}\n"
                                 f"A1: {self.A1_data[-1]:4d}\n"
                                 f"A2: {self.A2_data[-1]:4d}\n"
                                 f"A3: {self.A3_data[-1]:4d}")
                self.text.set_text(current_values)
            
            # 更新统计信息
            stats = self.calculate_statistics()
            self.stats_text.set_text(stats)
            
            # 根据时间窗口设置X轴范围
            if len(time_list) > 0:
                current_t = time_list[-1]
                window_start = max(0, current_t - self.time_window)
                window_end = current_t + 0.1  # 稍微向前一点
                
                self.ax.set_xlim(window_start, window_end)
            
            # 更新标题显示更多信息
            data_points = len(time_list)
            time_span = time_list[-1] - time_list[0] if len(time_list) > 1 else 0
            self.fig.suptitle(f'Arduino Analog Input Monitoring | Data Points: {data_points} | Time Span: {time_span:.1f}s | Window: {self.time_window}s', 
                             fontsize=14)
        
        # 定期更新屏幕（约每秒更新一次）
        if current_time - self.last_update_time > 0.1:
            self.last_update_time = current_time
            self.fig.canvas.draw_idle()
        
        return self.lines + [self.text, self.stats_text]
    
    def run(self):
        """运行绘图"""
        print("Starting advanced real-time plot...")
        print("Controls:")
        print("  - Use slider to adjust time window")
        print("  - Click Reset View to restore default")
        print("  - Use +/- buttons to zoom in/out")
        print("Close the plot window to stop.")
        
        # 创建动画
        ani = animation.FuncAnimation(self.fig, self.update_plot, 
                                      interval=50, blit=False, cache_frame_data=False)
        
        # 显示图形
        plt.show()
    
    def close(self):
        """关闭资源"""
        self.running = False
        if self.ser:
            self.ser.close()
        print("Closed serial connection.")

# 主程序
if __name__ == "__main__":
    # Mac系统上的串口地址
    PORT = '/dev/cu.usbmodemHIDPC1'
    BAUD_RATE = 9600
    
    # 创建并运行绘图器
    plotter = AdvancedSerialPlotter(
        port=PORT, 
        baud_rate=BAUD_RATE,
        max_data_points=5000,  # 存储更多数据点
        initial_time_window=60  # 初始显示60秒窗口
    )
    
    try:
        plotter.run()
    except KeyboardInterrupt:
        print("\nProgram interrupted by user")
    finally:
        plotter.close()
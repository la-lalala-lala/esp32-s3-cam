# ESP32-S3-CAM RTSP视频流项目

这是一个基于ESP32-S3-CAM开发板的RTSP视频流项目，使用Micro-RTSP库实现实时视频流传输功能。

## 硬件要求

- ESP32-S3-CAM开发板
- USB-C数据线
- 802.11b/g/n无线网络

## 软件要求

- [PlatformIO](https://platformio.org/) IDE或命令行工具
- [Arduino ESP32 框架](https://github.com/espressif/arduino-esp32)
- [Micro-RTSP库](https://github.com/geeksville/Micro-RTSP)（通过PlatformIO自动安装）

## 快速开始

### 1. 克隆项目

```bash
git clone <项目仓库地址>
cd esp32-s3-cam
```

### 2. 配置WiFi

在`src/main.cpp`中修改WiFi网络配置：

```cpp
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";
```

### 3. 编译和上传

使用PlatformIO编译并上传到ESP32-S3-CAM开发板：

```bash
pio run --target upload
```

### 4. 查看串口输出

```bash
pio device monitor
```

在串口输出中查找类似以下信息，获取RTSP流地址：

```
RTSP Stream available at: rtsp://192.168.1.100:554/
```

## 项目结构

```
esp32-s3-cam/
├── src/
│   ├── main.cpp           # 主程序代码
│   ├── rtsp_camera.h      # RTSP 摄像头服务器头文件
│   └── rtsp_camera.cpp    # RTSP 摄像头服务器实现文件
├── include/               # 头文件目录
├── lib/                   # 库目录
├── data/                  # 上传文件系统
├── platformio.ini        # PlatformIO 配置文件
└── README.md             # 本文档
```

## RTSP流媒体使用方法

### 连接RTSP流

项目启动后，RTSP服务器会自动启动。您可以使用以下地址连接RTSP流：

```
rtsp://<设备IP地址>:554/
```

### 客户端工具

您可以使用以下工具查看RTSP流：

- **VLC媒体播放器**：
  ```
  vlc rtsp://192.168.1.100:554/
  ```

- **FFplay**：
  ```
  ffplay rtsp://192.168.1.100:554/
  ```

## 配置选项

### RTSP服务器配置

您可以在`rtsp_camera.h`和`rtsp_camera.cpp`中修改RTSP服务器的配置：

```cpp
// 修改默认端口（默认554）
RTSPCameraServer rtspServer(8554);

// 修改默认帧率（默认10 FPS）
rtspServer.setFrameRate(15);
```

### 摄像头配置

在`main.cpp`中可以配置摄像头的分辨率和图像质量：

```cpp
camera_config_t config;
config.frame_size = FRAMESIZE_VGA;  // 分辨率
config.jpeg_quality = 10;           // 图像质量（0-63，值越小质量越高）
```

## 技术规格

- **默认分辨率**：640x480 (VGA)
- **默认帧率**：10 FPS
- **RTSP端口**：554
- **视频编码**：JPEG
- **网络协议**：WiFi (802.11b/g/n)

## 故障排除

### 无法连接RTSP流

1. 确认设备已成功连接到WiFi网络
2. 检查设备IP地址是否正确
3. 确认防火墙未阻止RTSP端口（默认554）
4. 尝试使用不同的RTSP客户端工具

### 视频流卡顿

1. 降低摄像头分辨率或提高JPEG质量值
2. 降低RTSP帧率
3. 确保设备与WiFi路由器距离较近，信号良好

### 编译错误

1. 确保PlatformIO已正确安装Micro-RTSP库
2. 检查ESP32-S3-CAM开发板选择是否正确
3. 确认PSRAM支持已在platformio.ini中启用

## 许可证

[MIT License](LICENSE)

## 贡献

欢迎提交Issue和Pull Request！

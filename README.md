# ESP32-S3-CAM Camera Streaming Project

本项目是一个基于 ESP32-S3和舵机的摄像头流媒体服务器，使用 Arduino 框架开发，支持 WiFi 连接和实时视频流传输。

## 硬件要求

- ESP32-S3 开发板（推荐 ESP32-S3-DevKitC-1）
- 支持的摄像头模块：
  - ESP32-S3-EYE（推荐）
  - AI Thinker
  - M5STACK PSRAM
  - M5STACK WITHOUT PSRAM
- 16MB Flash
- PSRAM（推荐，用于高分辨率图像）

## 软件要求

- PlatformIO
- Arduino Framework for ESP32
- 支持的操作系统：Windows / macOS / Linux

## 快速开始

### 1. 安装 PlatformIO

推荐使用 VSCode 配合 PlatformIO 插件：

```bash
code --install-extension platformio.platformio-ide
```

### 2. 克隆项目

```bash
git clone <your-repo-url>
cd esp32-s3-cam
```

### 3. 配置 WiFi 凭证

编辑 `src/main.cpp`，修改以下内容：

```cpp
const char* ssid = "你的WiFi名称";
const char* password = "你的WiFi密码";
```

### 4. 选择摄像头型号

在 `src/main.cpp` 中，根据你的硬件选择对应的摄像头型号：

```cpp
// 选择一个摄像头型号，取消注释
#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_AI_THINKER
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WITHOUT_PSRAM
```

### 5. 编译和上传

```bash
# 编译项目
pio run -e esp32-s3-devkitc-1 -t build

# 上传固件
pio run -e esp32-s3-devkitc-1 -t upload

# 打开串口监视器
pio device monitor
```

## 项目结构

```
esp32-s3-cam/
├── src/
│   └── main.cpp           # 主程序代码
├── include/               # 头文件目录
├── lib/                   # 库目录
├── data/                  # 上传文件系统
├── platformio.ini        # PlatformIO 配置文件
└── README.md             # 本文档
```

## 配置选项

### 摄像头分辨率

在 `initCameraServer()` 函数中可以配置摄像头分辨率：

```cpp
config.frame_size = FRAMESIZE_UXGA;  // 最大分辨率
// 其他选项：FRAMESIZE_SVGA, FRAMESIZE_QVGA, FRAMESIZE_240X240
```

### 图像质量

```cpp
config.jpeg_quality = 10;  // 0-63，数值越小质量越高
```

### 帧缓冲区数量

```cpp
config.fb_count = 2;  // PSRAM 可用时建议设置为 2
```

## 使用方法

### 1. 启动设备

上传固件后，设备会自动连接 WiFi，并在串口监视器中显示 IP 地址：

```
Camera Stream Ready! Go to: http://192.168.1.100
```

### 2. 访问视频流

在浏览器中打开显示的 IP 地址即可查看实时视频流。

### 3. API 端点

| 端点 | 方法 | 描述 |
|------|------|------|
| `/` | GET | 实时视频流 |
| `/jpg` | GET | 获取单张 JPEG 图片 |

## 引脚映射

### ESP32-S3-EYE（默认配置）

| 功能 | GPIO 编号 |
|------|----------|
| XCLK | 15 |
| SIOD | 4 |
| SIOC | 5 |
| Y2 | 11 |
| Y3 | 9 |
| Y4 | 8 |
| Y5 | 10 |
| Y6 | 12 |
| Y7 | 18 |
| Y8 | 17 |
| Y9 | 16 |
| VSYNC | 6 |
| HREF | 7 |
| PCLK | 13 |

## 故障排除

### 摄像头初始化失败

1. 检查摄像头连接是否正确
2. 确认选择了正确的摄像头型号
3. 检查电源供应是否充足

### WiFi 连接失败

1. 确认 WiFi 名称和密码正确
2. 检查 WiFi 信号强度
3. 尝试重启路由器

### 图像质量差

1. 确保开发板有 PSRAM
2. 调整 `jpeg_quality` 参数
3. 降低 `frame_size` 分辨率

## 性能优化

- 使用 PSRAM 可以获得更高分辨率和更流畅的帧率
- 适当调整 JPEG 质量可以减少带宽占用
- 确保稳定的电源供应以避免图像闪烁

## 许可证

本项目基于 MIT 许可证开源。

## 参考资源

- [ESP-IDF 文档](https://docs.espressif.com/projects/esp-idf/)
- [Arduino-ESP32 文档](https://docs.platformio.org/en/latest/frameworks/arduino.html)
- [ESP32-S3 数据手册](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)

#include "esp_camera.h"
#include <WiFi.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include "esp_http_server.h"
#include <AsyncTCP.h>

//Replace with your network credentials
const char* ssid = "优锘科技";
const char* password = "uinnova123";
#define REMOTE_IP   "192.168.125.116"
#define REMOTE_PORT 1234

#define PART_BOUNDARY "123456789000000000000987654321"

// This project was tested with the AI Thinker Model, M5STACK PSRAM Model and M5STACK WITHOUT PSRAM
#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_AI_THINKER
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WITHOUT_PSRAM

// Not tested with this model
//#define CAMERA_MODEL_WROVER_KIT

#if defined(CAMERA_MODEL_WROVER_KIT)
  #define PWDN_GPIO_NUM    -1
  #define RESET_GPIO_NUM   -1
  #define XCLK_GPIO_NUM    21
  #define SIOD_GPIO_NUM    26
  #define SIOC_GPIO_NUM    27
  
  #define Y9_GPIO_NUM      35
  #define Y8_GPIO_NUM      34
  #define Y7_GPIO_NUM      39
  #define Y6_GPIO_NUM      36
  #define Y5_GPIO_NUM      19
  #define Y4_GPIO_NUM      18
  #define Y3_GPIO_NUM       5
  #define Y2_GPIO_NUM       4
  #define VSYNC_GPIO_NUM   25
  #define HREF_GPIO_NUM    23
  #define PCLK_GPIO_NUM    22
#elif defined(CAMERA_MODEL_ESP32S3_EYE)
  #define PWDN_GPIO_NUM -1
  #define RESET_GPIO_NUM -1
  #define XCLK_GPIO_NUM 15
  #define SIOD_GPIO_NUM 4
  #define SIOC_GPIO_NUM 5

  #define Y2_GPIO_NUM 11
  #define Y3_GPIO_NUM 9
  #define Y4_GPIO_NUM 8
  #define Y5_GPIO_NUM 10
  #define Y6_GPIO_NUM 12
  #define Y7_GPIO_NUM 18
  #define Y8_GPIO_NUM 17
  #define Y9_GPIO_NUM 16

  #define VSYNC_GPIO_NUM 6
  #define HREF_GPIO_NUM 7
  #define PCLK_GPIO_NUM 13
#elif defined(CAMERA_MODEL_M5STACK_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       32
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_M5STACK_WITHOUT_PSRAM)
  #define PWDN_GPIO_NUM     -1
  #define RESET_GPIO_NUM    15
  #define XCLK_GPIO_NUM     27
  #define SIOD_GPIO_NUM     25
  #define SIOC_GPIO_NUM     23
  
  #define Y9_GPIO_NUM       19
  #define Y8_GPIO_NUM       36
  #define Y7_GPIO_NUM       18
  #define Y6_GPIO_NUM       39
  #define Y5_GPIO_NUM        5
  #define Y4_GPIO_NUM       34
  #define Y3_GPIO_NUM       35
  #define Y2_GPIO_NUM       17
  #define VSYNC_GPIO_NUM    22
  #define HREF_GPIO_NUM     26
  #define PCLK_GPIO_NUM     21

#elif defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
#else
  #error "Camera model not selected"
#endif

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
AsyncClient client;

// 0 == disconnected
// 1 == connecting
// 2 == connected
static uint8_t state = 0;
// 存储MAC地址字符串
char device_id[13]; 

void initDeviceID() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    // 格式化为 00112233AABB 这种紧凑格式
    sprintf(device_id, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.printf("设备唯一ID: %s\n", device_id);
}

// 处理视频流请求
static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return res;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

// 启动视频流服务器
void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  
  //Serial.printf("Starting web server on port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &index_uri);
  }
}

// 初始化相机服务器
void initCameraServer(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    // PSRAM is available, use it
    Serial.println("PSRAM available...");
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

// 连接视频流TCP服务器
void connectTcpServer(){
  Serial.println("connect stream channel");
  // register a callback when the client disconnects
  client.onDisconnect([](void *arg, AsyncClient *client) {
    Serial.printf("Disconnected.\n");
    state = 0;
  });

  // register a callback when an error occurs
  client.onError([](void *arg, AsyncClient *client, int8_t error) {
    Serial.printf("Error: %s\n", client->errorToString(error));
  });

  // register a callback when data arrives, to accumulate it
  client.onData([](void *arg, AsyncClient *client, void *data, size_t len) {
    // 将收到的数据转为字符串
    String msg = "";
    for(size_t i=0; i<len; i++){
        msg += (char)((uint8_t*)data)[i];
    }
    msg.trim();
    Serial.printf("Received Data: [%s]\n", msg.c_str());
    if (msg == "LED_ON") {
        digitalWrite(4, HIGH); // 打开 ESP32-CAM 闪光灯
    } else {
        digitalWrite(4, LOW);
    }
  });

  // register a callback when we are connected
  client.onConnect([](void *arg, AsyncClient *client) {
    Serial.printf("Connected!\n");
    state = 2;
  });

  // 注册连接成功回调
  client.onConnect([](void *arg, AsyncClient *client) {
    Serial.printf("Connected!\n");
      // --- 核心优化：发送注册包 ---
      // 格式约定为 "REG:[MAC_ADDRESS]\n"
      char reg_msg[32];
      int len = sprintf(reg_msg, "REG:%s\n", device_id);
      
      client->add(reg_msg, len);
      client->send(); 
      
      state = 2; // 进入推流状态
  }, NULL);

  client.onAck([](void *arg, AsyncClient *client, size_t len, uint32_t time) {
    Serial.printf("Acked %u bytes in %" PRIu32 " ms\n", len, time);
  });

  client.setRxTimeout(20000);
  client.setNoDelay(true);
}

// 发送JPEG帧到TCP服务器
void sendJpegFrame(){
  static uint32_t lastFrameTime = 0;
  const uint32_t frameInterval = 40; // 保持 25 FPS

  while (2 == state) {
      uint32_t now = millis();
      if (now - lastFrameTime >= frameInterval) {
          lastFrameTime = now;

          camera_fb_t* fb = esp_camera_fb_get();
          if (!fb) continue;

          // 检查异步缓冲区是否积压，避免内存崩溃
          if (client.canSend()) {
              // 如果服务器需要每帧都校验，可以在这里再加一个小包头
              // 但为了效率，建议只在连接时注册一次，后续由服务器维护 Socket 句柄与 ID 的映射
              client.add((const char*)fb->buf, fb->len);
              
              uint8_t end[5] = {'j', 'p', 'e', 'g', '\n'};
              client.add((const char*)end, 5);
              client.send();
          }

          esp_camera_fb_return(fb);
      }
      yield(); // 必须调用，防止触发看门狗并允许处理异步任务
  }
}

void setup() {
  // 关闭 ESP32 的欠压保护，防止模组因电压波动而反复重启
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  
  initCameraServer();

  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.print(WiFi.localIP());
  initDeviceID();
  // 启动视频流服务器
  // startCameraServer();
  // 连接视频流TCP服务器
  connectTcpServer();
}

void loop() {
  switch (state) {
    case 0:{
      // 已经断开
      Serial.printf("Connecting...\n");
      if (!client.connect(REMOTE_IP, REMOTE_PORT)) {
        Serial.printf("Failed to connect!\n");
        delay(1000);  // to not flood logs
      } else {
        state = 1;
      }
      break;
    }
    case 1:{
      // 正在连接
      Serial.printf("Still connecting...\n");
      delay(500);  // to not flood logs
      break;
    }
    case 2:{
      // 连接成功
      Serial.printf("Connected!\n");
      // 发送JPEG帧
      sendJpegFrame();
      break;
    }
    default: break;
  }
}
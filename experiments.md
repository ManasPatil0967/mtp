09/12/2025

Devices used: 
1. STM32F103C8T6, apparently called Blue Pill board.
2. ESP32 DOIT Devkit V1
3. STLink V2
4. OLED SSD1306 128 * 32

Connections for UART Bridge (STM32 -> ESP32):
A9 -> RX2
A10 -> TX2
G -> GND

Connections for SSD1306 (STM32 -> SSD1306):
B6 -> SCL
B7 -> SDA
G -> GND
3.3 -> VCC

Steps and Code for SSD1306:
1. Download headers and C files from [https://github.com/afiskon/stm32-ssd1306](afiskon/stm32-ssd1306) and put the headers in Core/Inc and C files in Core/Src.
2. Change the name of ssd_config_template.h to ssd_config.h.
3. Change the height of screen in ssd1306.h.
4. Use the snippet: 
```cpp
#include <string.h>
#include <stdarg.h>
#include "ssd1306.h"
#include "ssd1306_fonts.h"
// In the user space of code in while(1)
ssd1306_Init();
ssd1306_SetCursor(10, 10);
ssd1306_WriteString("Resampling...", Font_7x10, White);
ssd1306_UpdateScreen();
```
---

10/12/2025

Profiling:
Transmitting data: 1 KB Lorem Ipsum at 8 MHz
1. Blocking on UART: 715089 cycles on average
2. DMA: 6527 cycles

Blocking Code: 
```cpp
HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
```
DMA Code:
```cpp
volatile uint8_t uart_tx_done = 1;
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        uart_tx_done = 1;
    }
}
// In main()
//Claim channel
uart_tx_done = 0;
//Transmit data
HAL_UART_Transmit_DMA(&huart1, (uint8_t*)msg, strlen(msg));
// Wait for DMA to complete (measuring full TX time)
while (!uart_tx_done);
```

Cycle Counter code:
```cpp
uint32_t var;
// Enable Cycle Counter
CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
DWT->CYCCNT = 0;
var = DWT->CYCCNT;
```
After removing strlen(msg):
1. Blocking: 708930 cycles
2. DMA: 350

DMA Setup Tips: 
1. Set up DMA in IOC Core
2. ENABLE GLOBAL USART1 NVIC GLOBAL INTERRUPT!
3. Ensure DMA init is before USART init.

The benchmark UART function is as follows:
```cpp
uint32_t Benchmark_UART(void)
{
    static const char msg[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam dolor ligula, sollicitudin tincidunt aliquam ac, mattis faucibus sem. Etiam commodo, sem semper consequat scelerisque, risus felis malesuada neque, vel faucibus nibh risus ut erat. Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum nunc magna, tristique vitae tempus quis, egestas eu lectus. Mauris eget ultrices leo. Sed a velit a ante auctor pellentesque eu a mauris. Phasellus eget suscipit metus, vitae sodales libero. Morbi pharetra consequat felis. Sed luctus urna vel arcu hendrerit semper. Vivamus sem tortor, commodo eget quam nec, mollis iaculis diam. Ut placerat non mauris quis rutrum. Ut sit amet ipsum urna. Aenean ultricies ipsum quis mauris pulvinar, vitae vehicula nisl tempus. Praesent luctus est nec sollicitudin mattis. Integer non turpis iaculis, lacinia leo quis, euismod lacus. Curabitur lobortis lorem dolor, vitae volutpat mauris facilisis et. Etiam lacinia in quam eget fringilla. Ut aenean.\r\n";
    uint32_t start, end;
    uint32_t len = strlen(msg);
    // Enable Cycle Counter
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    // 1. BLOCKING UART (HAL_UART_Transmit)
    DWT->CYCCNT = 0;
    start = DWT->CYCCNT;
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, len, HAL_MAX_DELAY);
    end = DWT->CYCCNT;
    uint32_t cycles_blocking = end - start;

    HAL_Delay(5);
    UART_Printf("Blocking UART cycles:       %lu\r\n", cycles_blocking);
    HAL_Delay(5);

    // 2. DMA UART (HAL_UART_Transmit_DMA)
    uart_tx_done = 0;
    DWT->CYCCNT = 0;
    start = DWT->CYCCNT;
    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)msg, len);
    end = DWT->CYCCNT;
    uint32_t cycles_dma_start = end - start;

    // Wait for DMA to complete (measuring full TX time)
    while (!uart_tx_done);
    // Measure full end-to-end DMA transmission time
    uint32_t cycles_dma_total = DWT->CYCCNT;
    UART_Printf("DMA UART start cost cycles: %lu\r\n", cycles_dma_start);
    UART_Printf("DMA total cycles:           %lu\r\n", cycles_dma_total);

    return cycles_dma_start;
}
```
---

10/01/2026

D-SUN USB-to-TTL connections with the ESP32 CAM OV3660 module:

D-SUN Pin	ESP32-CAM Pin	Purpose \
5V	5V	Power (Do not use 3.3V) \
GND	GND	Ground \
TX	U0R (GPIO 3)	Data from PC to ESP \
RX	U0T (GPIO 1)	Data from ESP to PC 

To flash code on the ESP32 CAM, connect GPIO 0 to the GND besides it and upload the code. 
Then reset and the code works. Before uploading, reset the CAM as well.
Code used to stream camera to webserver: CameraWebServer example.
Tactile button added with GPIO 13 to Button terminal (springing side) one to GND.
Add the following code to the CameraWebServer example main sketch. This code connects to the mobile WiFi and renders the captured image there:
```cpp
#include "esp_camera.h"
#include "Arduino.h"
#include "WiFi.h"
#include "esp_http_server.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

const char* ssid = "";
const char* password = "";

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

#define LED_GPIO_NUM      33
#define BUTTON_PIN        13

httpd_handle_t server = NULL;
camera_fb_t *last_capture = NULL;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;
int captureCount = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM OV3660</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background: #1a1a2e;
      color: #eee;
      margin: 0;
      padding: 20px;
    }
    h1 { color: #00d4ff; }
    .container {
      max-width: 800px;
      margin: 0 auto;
    }
    img {
      max-width: 100%;
      border: 3px solid #00d4ff;
      border-radius: 10px;
      margin: 20px 0;
    }
    .btn {
      background: #00d4ff;
      color: #1a1a2e;
      border: none;
      padding: 15px 30px;
      font-size: 18px;
      border-radius: 5px;
      cursor: pointer;
      margin: 10px;
    }
    .btn:hover { background: #00a8cc; }
    .info {
      background: #16213e;
      padding: 15px;
      border-radius: 10px;
      margin: 20px 0;
    }
    #status { color: #00ff88; }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32-CAM OV3660</h1>
    <div class="info">
      <p>Captures: <span id="count">0</span></p>
      <p id="status">Ready</p>
    </div>
    <img id="photo" src="/capture" alt="Press button or click Capture">
    <br>
    <button class="btn" onclick="capturePhoto()">Capture</button>
    <button class="btn" onclick="toggleStream()">Toggle Stream</button>
  </div>
  <script>
    let streaming = false;
    let streamInterval;
    
    function capturePhoto() {
      document.getElementById('status').innerText = 'Capturing...';
      fetch('/trigger')
        .then(response => response.text())
        .then(data => {
          document.getElementById('photo').src = '/capture?' + Date.now();
          document.getElementById('count').innerText = data;
          document.getElementById('status').innerText = 'Ready';
        });
    }
    
    function toggleStream() {
      streaming = !streaming;
      if (streaming) {
        document.getElementById('status').innerText = 'Streaming...';
        streamInterval = setInterval(() => {
          document.getElementById('photo').src = '/stream?' + Date.now();
        }, 100);
      } else {
        clearInterval(streamInterval);
        document.getElementById('status').innerText = 'Ready';
      }
    }
    
    // Auto-refresh image every 2 seconds to catch button presses
    setInterval(() => {
      if (!streaming) {
        fetch('/count').then(r => r.text()).then(c => {
          if (document.getElementById('count').innerText !== c) {
            document.getElementById('photo').src = '/capture?' + Date.now();
            document.getElementById('count').innerText = c;
          }
        });
      }
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, index_html, strlen(index_html));
}

esp_err_t capture_handler(httpd_req_t *req) {
  if (last_capture == NULL) {
    // No image yet, capture one
    last_capture = esp_camera_fb_get();
    if (!last_capture) {
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
  }
  
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
  return httpd_resp_send(req, (const char *)last_capture->buf, last_capture->len);
}

esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }
  
  httpd_resp_set_type(req, "image/jpeg");
  esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  return res;
}

esp_err_t trigger_handler(httpd_req_t *req) {
  // Release old frame
  if (last_capture) {
    esp_camera_fb_return(last_capture);
    last_capture = NULL;
  }

  camera_fb_t *flush = esp_camera_fb_get();
  if (flush) esp_camera_fb_return(flush);
  delay(50);
  
  digitalWrite(LED_GPIO_NUM, LOW);
  last_capture = esp_camera_fb_get();
  digitalWrite(LED_GPIO_NUM, HIGH);
  
  if (last_capture) {
    captureCount++;
    Serial.printf("Web capture #%d\n", captureCount);
  }
  
  char count_str[16];
  sprintf(count_str, "%d", captureCount);
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, count_str, strlen(count_str));
}

esp_err_t count_handler(httpd_req_t *req) {
  char count_str[16];
  sprintf(count_str, "%d", captureCount);
  httpd_resp_set_type(req, "text/plain");
  return httpd_resp_send(req, count_str, strlen(count_str));
}

void startWebServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_uri_t index_uri = {
      .uri       = "/",
      .method    = HTTP_GET,
      .handler   = index_handler,
      .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &index_uri);

    httpd_uri_t capture_uri = {
      .uri       = "/capture",
      .method    = HTTP_GET,
      .handler   = capture_handler,
      .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &capture_uri);

    httpd_uri_t stream_uri = {
      .uri       = "/stream",
      .method    = HTTP_GET,
      .handler   = stream_handler,
      .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &stream_uri);

    httpd_uri_t trigger_uri = {
      .uri       = "/trigger",
      .method    = HTTP_GET,
      .handler   = trigger_handler,
      .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &trigger_uri);

    httpd_uri_t count_uri = {
      .uri       = "/count",
      .method    = HTTP_GET,
      .handler   = count_handler,
      .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &count_uri);

    Serial.println("Web server started!");
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=================================");
  Serial.println("ESP32-CAM OV3660 Web Server");
  Serial.println("=================================");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_GPIO_NUM, OUTPUT);
  digitalWrite(LED_GPIO_NUM, HIGH);

  if (psramFound()) {
    Serial.println("PSRAM: OK");
  } else {
    Serial.println("PSRAM: Not found!");
  }

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
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;   // 640x480
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  Serial.print("Camera: ");
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("FAILED!");
    return;
  }
  Serial.println("OK");

  sensor_t *s = esp_camera_sensor_get();
  Serial.printf("Sensor PID: 0x%x\n", s->id.PID);
  if (s->id.PID == 0x3660) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
  }

  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
    digitalWrite(LED_GPIO_NUM, !digitalRead(LED_GPIO_NUM));
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi connection failed!");
    return;
  }
  
  digitalWrite(LED_GPIO_NUM, HIGH);  // LED off
  Serial.println("\nWiFi connected!");
  Serial.println();
  Serial.println("=================================");
  Serial.print("Open browser: http://");
  Serial.println(WiFi.localIP());
  Serial.println("=================================");

  startWebServer();

  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_GPIO_NUM, LOW);
    delay(100);
    digitalWrite(LED_GPIO_NUM, HIGH);
    delay(100);
  }
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis();
      
      if (last_capture) {
        esp_camera_fb_return(last_capture);
        last_capture = NULL;
      }

      camera_fb_t *flush = esp_camera_fb_get();
      if (flush) {
        esp_camera_fb_return(flush);
      }
      delay(50);
      
      digitalWrite(LED_GPIO_NUM, LOW);
      last_capture = esp_camera_fb_get();
      digitalWrite(LED_GPIO_NUM, HIGH);
      
      if (last_capture) {
        captureCount++;
        Serial.printf("Button capture #%d (%u bytes)\n", captureCount, last_capture->len);
      }
      
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }
  
  delay(10);
}
```
```
---

11/01/2026



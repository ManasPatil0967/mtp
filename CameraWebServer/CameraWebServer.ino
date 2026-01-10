// #include "esp_camera.h"
// #include <WiFi.h>

// // ===========================
// // Select camera model in board_config.h
// // ===========================
// #include "board_config.h"

// // ===========================
// // Enter your WiFi credentials
// // ===========================
// const char *ssid = "Galaxy A316A42";
// const char *password = "patilmanas";

// void startCameraServer();
// void setupLedFlash();

// void setup() {
//   Serial.begin(115200);
//   Serial.setDebugOutput(true);
//   Serial.println();

//   camera_config_t config;
//   config.ledc_channel = LEDC_CHANNEL_0;
//   config.ledc_timer = LEDC_TIMER_0;
//   config.pin_d0 = Y2_GPIO_NUM;
//   config.pin_d1 = Y3_GPIO_NUM;
//   config.pin_d2 = Y4_GPIO_NUM;
//   config.pin_d3 = Y5_GPIO_NUM;
//   config.pin_d4 = Y6_GPIO_NUM;
//   config.pin_d5 = Y7_GPIO_NUM;
//   config.pin_d6 = Y8_GPIO_NUM;
//   config.pin_d7 = Y9_GPIO_NUM;
//   config.pin_xclk = XCLK_GPIO_NUM;
//   config.pin_pclk = PCLK_GPIO_NUM;
//   config.pin_vsync = VSYNC_GPIO_NUM;
//   config.pin_href = HREF_GPIO_NUM;
//   config.pin_sccb_sda = SIOD_GPIO_NUM;
//   config.pin_sccb_scl = SIOC_GPIO_NUM;
//   config.pin_pwdn = PWDN_GPIO_NUM;
//   config.pin_reset = RESET_GPIO_NUM;
//   config.xclk_freq_hz = 20000000;
//   config.frame_size = FRAMESIZE_UXGA;
//   config.pixel_format = PIXFORMAT_JPEG;  // for streaming
//   //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
//   config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
//   config.fb_location = CAMERA_FB_IN_PSRAM;
//   config.jpeg_quality = 12;
//   config.fb_count = 1;

//   // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
//   //                      for larger pre-allocated frame buffer.
//   if (config.pixel_format == PIXFORMAT_JPEG) {
//     if (psramFound()) {
//       config.jpeg_quality = 10;
//       config.fb_count = 2;
//       config.grab_mode = CAMERA_GRAB_LATEST;
//     } else {
//       // Limit the frame size when PSRAM is not available
//       config.frame_size = FRAMESIZE_SVGA;
//       config.fb_location = CAMERA_FB_IN_DRAM;
//     }
//   } else {
//     // Best option for face detection/recognition
//     config.frame_size = FRAMESIZE_240X240;
// #if CONFIG_IDF_TARGET_ESP32S3
//     config.fb_count = 2;
// #endif
//   }

// #if defined(CAMERA_MODEL_ESP_EYE)
//   pinMode(13, INPUT_PULLUP);
//   pinMode(14, INPUT_PULLUP);
// #endif

//   // camera init
//   esp_err_t err = esp_camera_init(&config);
//   if (err != ESP_OK) {
//     Serial.printf("Camera init failed with error 0x%x", err);
//     return;
//   }

//   sensor_t *s = esp_camera_sensor_get();
//   // initial sensors are flipped vertically and colors are a bit saturated
//   if (s->id.PID == OV3660_PID) {
//     s->set_vflip(s, 1);        // flip it back
//     s->set_brightness(s, 1);   // up the brightness just a bit
//     s->set_saturation(s, -2);  // lower the saturation
//   }
//   // drop down frame size for higher initial frame rate
//   if (config.pixel_format == PIXFORMAT_JPEG) {
//     s->set_framesize(s, FRAMESIZE_QVGA);
//   }

// #if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
//   s->set_vflip(s, 1);
//   s->set_hmirror(s, 1);
// #endif

// #if defined(CAMERA_MODEL_ESP32S3_EYE)
//   s->set_vflip(s, 1);
// #endif

// // Setup LED FLash if LED pin is defined in camera_pins.h
// #if defined(LED_GPIO_NUM)
//   setupLedFlash();
// #endif

//   WiFi.begin(ssid, password);
//   WiFi.setSleep(false);

//   Serial.print("WiFi connecting");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("");
//   Serial.println("WiFi connected");

//   startCameraServer();

//   Serial.print("Camera Ready! Use 'http://");
//   Serial.print(WiFi.localIP());
//   Serial.println("' to connect");
// }

// void loop() {
//   // Do nothing. Everything is done in another task by the web server
//   delay(10000);
// }

// #include "esp_camera.h"
// #include "Arduino.h"
// #include "FS.h"
// #include "SD_MMC.h"
// #include "soc/soc.h"
// #include "soc/rtc_cntl_reg.h"
// #include "driver/rtc_io.h"

// // ===================
// // AI Thinker ESP32-CAM Pin Definitions
// // Do NOT include camera_pins.h - we define everything here
// // ===================
// #define PWDN_GPIO_NUM     32
// #define RESET_GPIO_NUM    -1
// #define XCLK_GPIO_NUM      0
// #define SIOD_GPIO_NUM     26
// #define SIOC_GPIO_NUM     27

// #define Y9_GPIO_NUM       35
// #define Y8_GPIO_NUM       34
// #define Y7_GPIO_NUM       39
// #define Y6_GPIO_NUM       36
// #define Y5_GPIO_NUM       21
// #define Y4_GPIO_NUM       19
// #define Y3_GPIO_NUM       18
// #define Y2_GPIO_NUM        5
// #define VSYNC_GPIO_NUM    25
// #define HREF_GPIO_NUM     23
// #define PCLK_GPIO_NUM     22

// // Flash LED pin (shared with SD card data line)
// #define FLASH_GPIO_NUM     4

// // Status LED (active low on most AI-Thinker boards)
// #define LED_GPIO_NUM      33

// void setup() {
//   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Disable brownout detector
  
//   Serial.begin(115200);
//   Serial.setDebugOutput(true);
//   delay(1000); // Give serial time to initialize
//   Serial.println();
//   Serial.println("=================================");
//   Serial.println("ESP32-CAM OV3660 Test Starting...");
//   Serial.println("=================================");

//   // Check for PSRAM - OV3660 at high resolutions REQUIRES it
//   if (psramFound()) {
//     Serial.println("PSRAM found and initialized");
//   } else {
//     Serial.println("WARNING: No PSRAM found! High resolutions may fail.");
//   }

//   // Camera configuration
//   camera_config_t config;
//   config.ledc_channel = LEDC_CHANNEL_0;
//   config.ledc_timer = LEDC_TIMER_0;
//   config.pin_d0 = Y2_GPIO_NUM;
//   config.pin_d1 = Y3_GPIO_NUM;
//   config.pin_d2 = Y4_GPIO_NUM;
//   config.pin_d3 = Y5_GPIO_NUM;
//   config.pin_d4 = Y6_GPIO_NUM;
//   config.pin_d5 = Y7_GPIO_NUM;
//   config.pin_d6 = Y8_GPIO_NUM;
//   config.pin_d7 = Y9_GPIO_NUM;
//   config.pin_xclk = XCLK_GPIO_NUM;
//   config.pin_pclk = PCLK_GPIO_NUM;
//   config.pin_vsync = VSYNC_GPIO_NUM;
//   config.pin_href = HREF_GPIO_NUM;
//   config.pin_sccb_sda = SIOD_GPIO_NUM;  // Fixed: was pin_sscb_sda
//   config.pin_sccb_scl = SIOC_GPIO_NUM;  // Fixed: was pin_sscb_scl
//   config.pin_pwdn = PWDN_GPIO_NUM;
//   config.pin_reset = RESET_GPIO_NUM;
//   config.xclk_freq_hz = 20000000;       // 20MHz for OV3660
//   config.pixel_format = PIXFORMAT_JPEG;
//   config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
//   config.fb_location = CAMERA_FB_IN_PSRAM;  // Use PSRAM for frame buffer

//   // OV3660 with PSRAM - can use higher resolutions
//   if (psramFound()) {
//     config.frame_size = FRAMESIZE_UXGA;   // 1600x1200
//     config.jpeg_quality = 10;              // 0-63, lower = higher quality
//     config.fb_count = 2;                   // Double buffer
//   } else {
//     // Fallback for no PSRAM
//     config.frame_size = FRAMESIZE_SVGA;   // 800x600
//     config.jpeg_quality = 12;
//     config.fb_count = 1;
//     config.fb_location = CAMERA_FB_IN_DRAM;
//   }

//   // Initialize camera
//   Serial.println("Initializing camera...");
//   esp_err_t err = esp_camera_init(&config);
//   if (err != ESP_OK) {
//     Serial.printf("Camera init failed with error 0x%x\n", err);
//     Serial.println("Common causes:");
//     Serial.println("  - Wrong camera module (not OV3660)");
//     Serial.println("  - Loose ribbon cable connection");
//     Serial.println("  - Insufficient power supply");
//     return;
//   }
//   Serial.println("Camera initialized successfully!");

//   // Get sensor reference for OV3660-specific settings
//   sensor_t *s = esp_camera_sensor_get();
//   if (s == NULL) {
//     Serial.println("Failed to get sensor reference");
//     return;
//   }

//   // Print detected sensor
//   Serial.printf("Sensor PID: 0x%x\n", s->id.PID);
  
//   // Check if it's actually an OV3660 (PID = 0x3660)
//   if (s->id.PID == 0x3660) {
//     Serial.println("OV3660 sensor detected!");
    
//     // OV3660 specific settings
//     s->set_vflip(s, 1);        // Flip vertically
//     s->set_hmirror(s, 0);      // Horizontal mirror
//     s->set_brightness(s, 1);   // Slightly increase brightness
//     s->set_saturation(s, 0);   // Default saturation
//     s->set_contrast(s, 0);     // Default contrast
//   } else if (s->id.PID == 0x2640) {
//     Serial.println("WARNING: OV2640 detected, not OV3660!");
//   } else {
//     Serial.printf("Unknown sensor detected: 0x%x\n", s->id.PID);
//   }

//   Serial.println("Camera Ready!");

//   // SD Card Initialization
//   Serial.println("Initializing SD Card...");
  
//   // Disable flash LED before SD init to avoid conflicts
//   pinMode(FLASH_GPIO_NUM, INPUT);
//   delay(10);

//   if (!SD_MMC.begin("/sdcard", true)) {  // true = 1-bit mode
//     Serial.println("SD Card Mount Failed!");
//     Serial.println("Check: card inserted, FAT32 format, 32GB or smaller");
//     return;
//   }
  
//   uint8_t cardType = SD_MMC.cardType();
//   if (cardType == CARD_NONE) {
//     Serial.println("No SD Card attached");
//     return;
//   }

//   uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
//   Serial.printf("SD Card Size: %lluMB\n", cardSize);
//   Serial.println("SD Card initialized successfully!");

//   // Wait for camera to stabilize
//   delay(500);

//   // Capture image
//   Serial.println("Capturing image...");
//   camera_fb_t *fb = esp_camera_fb_get();
//   if (!fb) {
//     Serial.println("Camera capture failed!");
//     return;
//   }

//   Serial.printf("Image captured! Size: %u bytes\n", fb->len);
//   Serial.printf("Resolution: %dx%d\n", fb->width, fb->height);

//   // Save to SD card
//   String path = "/ov3660_test.jpg";
  
//   Serial.printf("Saving to: %s\n", path.c_str());
//   File file = SD_MMC.open(path.c_str(), FILE_WRITE);
//   if (!file) {
//     Serial.println("Failed to open file for writing!");
//   } else {
//     size_t written = file.write(fb->buf, fb->len);
//     Serial.printf("File saved! (%u bytes)\n", written);
//     file.close();
//   }

//   // Return frame buffer
//   esp_camera_fb_return(fb);

//   Serial.println("=================================");
//   Serial.println("Test Complete! Check SD card.");
//   Serial.println("=================================");

//   // Blink LED to indicate success
//   pinMode(LED_GPIO_NUM, OUTPUT);
//   for (int i = 0; i < 5; i++) {
//     digitalWrite(LED_GPIO_NUM, LOW);   // LED on (active low)
//     delay(200);
//     digitalWrite(LED_GPIO_NUM, HIGH);  // LED off
//     delay(200);
//   }
// }

// void loop() {
//   delay(10000);
// }

#include "esp_camera.h"
#include "Arduino.h"
#include "WiFi.h"
#include "esp_http_server.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ===================
// WiFi Credentials
// ===================
const char* ssid = "Galaxy A316A42";
const char* password = "patilmanas";

// ===================
// Pin Definitions
// ===================
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

// Global variables
httpd_handle_t server = NULL;
camera_fb_t *last_capture = NULL;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;
int captureCount = 0;

// HTML page with auto-refresh
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

// Handler: Serve main page
esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, index_html, strlen(index_html));
}

// Handler: Return last captured image
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

// Handler: Live stream frame
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

// Handler: Trigger capture from web button
esp_err_t trigger_handler(httpd_req_t *req) {
  // Release old frame
  if (last_capture) {
    esp_camera_fb_return(last_capture);
    last_capture = NULL;
  }

  camera_fb_t *flush = esp_camera_fb_get();
  if (flush) esp_camera_fb_return(flush);
  delay(50);
  
  // Capture new frame
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

// Handler: Get capture count
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
    // Register URI handlers
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

  // Setup GPIO
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_GPIO_NUM, OUTPUT);
  digitalWrite(LED_GPIO_NUM, HIGH);

  // Check PSRAM
  if (psramFound()) {
    Serial.println("PSRAM: OK");
  } else {
    Serial.println("PSRAM: Not found!");
  }

  // Camera config
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

  // Init camera
  Serial.print("Camera: ");
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("FAILED!");
    return;
  }
  Serial.println("OK");

  // OV3660 settings
  sensor_t *s = esp_camera_sensor_get();
  Serial.printf("Sensor PID: 0x%x\n", s->id.PID);
  if (s->id.PID == 0x3660) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
  }

  // Connect to WiFi
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
    // Blink LED while connecting
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

  // Start web server
  startWebServer();

  // Ready blink
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_GPIO_NUM, LOW);
    delay(100);
    digitalWrite(LED_GPIO_NUM, HIGH);
    delay(100);
  }
}

void loop() {
  // Physical button capture
  if (digitalRead(BUTTON_PIN) == LOW) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis();
      
      // Release old frame
      if (last_capture) {
        esp_camera_fb_return(last_capture);
        last_capture = NULL;
      }

      camera_fb_t *flush = esp_camera_fb_get();
      if (flush) {
        esp_camera_fb_return(flush);
      }
      delay(50);  // Small delay to let camera capture fresh frame
      
      // Capture new
      digitalWrite(LED_GPIO_NUM, LOW);
      last_capture = esp_camera_fb_get();
      digitalWrite(LED_GPIO_NUM, HIGH);
      
      if (last_capture) {
        captureCount++;
        Serial.printf("Button captured! #%d (%u bytes)\n", captureCount, last_capture->len);
      }
      
      // Wait for release
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
  }
  
  delay(10);
}
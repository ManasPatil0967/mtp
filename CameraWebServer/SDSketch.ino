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
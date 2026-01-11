#include "esp_camera.h"
#include "Arduino.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

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
#define BUTTON_PIN_1      12
#define BUTTON_PIN_2      15

volatile bool captureRequested = false;
volatile bool otherRequested = false;
HardwareSerial STMSerial(2);

void IRAM_ATTR captureISR() {
    captureRequested = true;
}

void IRAM_ATTR otherISR() {
    otherRequested = true;
}

// Global variables
camera_fb_t *last_capture = NULL;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;
int captureCount = 0;
uint8_t decimated[1024];

uint8_t decimate(camera_fb_t* captured) {
  uint8_t result[1024];
  for(int i = 0; i < 32; i++) {
    for(int j = 0; j < 32; j++) {
      result[i * 32 + j] = captured->buf[i*4*160 + j*5]; 
      //result[i*(resolution_height) + j] = captured->buf[i*(floor(resolution_height/captured_height))*captured_width + (resolution_width/captured_width)]
    }
  }
}

void setup() {
  Serial.begin(921600);
  delay(1000);
  STM32Serial.begin(921600, SERIAL_8N1, 14, 13);
  delay(1000);
  
  if (psramFound()) {
    Serial.println("PSRAM: OK");
  } else {
    Serial.println("PSRAM: Not found!");
  }
  
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN_1), captureISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN_2), otherISR, FALLING);

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
    config.frame_size = FRAMESIZE_QQVGA;  // 160x120
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
}

void loop() {
  if(captureRequested || otherRequested) {
    while(captureRequested || otherRequested) {
      delay(10);
    }

    captureRequested = false;
    otherRequested = false;

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
      Serial.printf("Button captured! #%d (%u bytes)\n", captureCount, last_capture->len);
      decimated = decimate(last_capture);
      Serial.write(0xAA);
      Serial.write(decimated);
      Serial.write(0x55);
      esp_camera_fb_return(last_capture);
    }
  }

}

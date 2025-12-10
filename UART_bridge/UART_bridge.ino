// Flash this to your ESP32
#include <HardwareSerial.h>

// Define the pins connected to STM32
#define RX_PIN 16 // Connect to STM32 TX (PA9)
#define TX_PIN 17 // Connect to STM32 RX (PA10)

HardwareSerial MySerial(2); // Use UART2

void setup() {
  // Serial: Communication with PC via USB
  Serial.begin(115200); 
  pinMode(2, OUTPUT);
  
  // MySerial: Communication with STM32
  // Format: Baud, Config, RX Pin, TX Pin
  MySerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN); 
  
  Serial.println("ESP32 UART Bridge Started...");
}

void loop() {
  // Forward data from STM32 -> PC
  if (MySerial.available()) {
    Serial.write(MySerial.read());
  }

  // Forward data from PC -> STM32 (Optional, for commands)
  if (Serial.available()) {
    MySerial.write(Serial.read());
  }
  // digitalWrite(2, HIGH); // Turn the LED on
  // delay(1000);           // Wait for 1 second
  // digitalWrite(2, LOW);  // Turn the LED off
  // delay(1000);           // Wait for 1 second
}
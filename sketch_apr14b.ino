#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "esp_flash.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

const uint32_t FLASH_ADDRESS = 0x300000;
const int FRAME_SIZE = 1024;
const uint32_t TOTAL_SIZE = 746496; // 729 frames @ 1024 bytes

uint8_t frameBuffer[FRAME_SIZE];
uint32_t currentOffset = 0;

// Timing Variables
unsigned long lastFrameTime = 0;
const int FRAME_DELAY = 40; // 1000ms / 25fps = 40ms

void setup() {
  u8g2.begin();
  
  // Overclocking I2C: 1MHz is standard, but some SH1106 chips can handle 1.2MHz
  // Let's stick to 1MHz first. If it's still slow, try 1200000
  u8g2.setBusClock(1000000); 
}

void loop() {
  unsigned long currentTime = millis();

  // Only render if 40ms have passed since the last frame
  if (currentTime - lastFrameTime >= FRAME_DELAY) {
    lastFrameTime = currentTime;

    // Read and Render
    esp_flash_read(NULL, frameBuffer, FLASH_ADDRESS + currentOffset, FRAME_SIZE);
    
    u8g2.clearBuffer();
    u8g2.drawBitmap(0, 0, 16, 64, frameBuffer);
    u8g2.sendBuffer(); // This takes ~25-30ms

    currentOffset += FRAME_SIZE;
    if (currentOffset >= TOTAL_SIZE) currentOffset = 0;
  }
}
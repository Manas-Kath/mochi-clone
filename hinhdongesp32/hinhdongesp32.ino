/*
 * Mochi EDC - Sentient Engine v2.1 (Ultra-Smooth Variety)
 * Features: Startup Intro, 40+ Random Bitmap Moods (PRE-CONVERTED), Petting Sensor
 * Performance: Replaces GIF engine with raw bitmaps for 60FPS feel on I2C.
 */

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <ChronosESP32.h>
#include "all_frames.h"         // legacy Huykhong set
#include "250frames.h"          // legacy 250 set
#include "daichi_intro.h"       // legacy intro
#include "animation_bitmaps.h"  // NEW 29+ smooth bitmap animations

#define I2C_SDA 21
#define I2C_SCL 22
#define TOUCH_PIN 4  

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
ChronosESP32 chronos("Mochi-EDC"); 

enum RenderType { HUYKHONG_ARRAY, STRUCT_ARRAY, EXTRA_BITMAP_ARRAY };
enum MochiState { INTRO, IDLE, NOTIFICATION, MUSIC, CALL, PETTED };
MochiState currentState = INTRO;

// Comprehensive Mood Structure
struct MoodData {
  RenderType type;
  const void* data;
  int count;
};

MoodData moodHappyLegacy = { HUYKHONG_ARRAY, (const void*)frames, 772 };
MoodData moodMiscLegacy  = { STRUCT_ARRAY, (const void*)MOCHI_225FRAMES_frames, 225 };
MoodData currentMood = moodHappyLegacy;

// Metadata & Sync
String trackTitle = "Unknown", trackArtist = "Unknown";
bool musicIsPlaying = false;
String headerText = "", bodyText = "";
unsigned long stateStartTime = 0;
const unsigned long NOTIFY_TIMEOUT = 8000; 

// Animation Controller
#define FRAME_DELAY 40 
int currentFrame = 0;
unsigned long lastFrameTime = 0;

void dataCallback(uint8_t *data, int len) {
  if (len > 0 && data[0] == 0x03) {
    musicIsPlaying = (data[1] == 0x01);
    trackTitle = String((char*)(data + 2));
    trackArtist = String((char*)(data + 2 + trackTitle.length() + 1));
  }
}

void onNotification(Notification notification) {
  headerText = String(notification.app);
  bodyText = String(notification.message);
  stateStartTime = millis();
  currentState = NOTIFICATION;
}

void onCall(String callerName, bool isActive) {
  if (isActive) {
    headerText = "CALLING...";
    bodyText = callerName;
    currentState = CALL;
  } else {
    currentState = IDLE;
  }
}

void pickRandomMood() {
  int r = random(0, 100);
  if (r < 10) {
    currentMood = moodHappyLegacy;
  } else if (r < 20) {
    currentMood = moodMiscLegacy;
  } else {
    int idx = random(0, EXTRA_MOODS_COUNT);
    currentMood.type = EXTRA_BITMAP_ARRAY;
    currentMood.data = (const void*)extraMoods[idx].frames;
    currentMood.count = extraMoods[idx].count;
  }
  currentFrame = 0;
  Serial.println("New Smooth Mood Picked!");
}

void setup(void) {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  u8g2.begin();
  u8g2.setBusClock(400000);
  
  chronos.setRawDataCallback(dataCallback);
  chronos.setNotificationCallback(onNotification);
  chronos.setRingerCallback(onCall);
  chronos.begin();
  
  Serial.println("Mochi Engine v2.1 Ready.");
}

void loop() {
  chronos.loop(); 

  // Touch Sensor
  if (currentState == IDLE && touchRead(TOUCH_PIN) < 30) {
    currentState = PETTED;
    stateStartTime = millis();
  }

  // Handle Logic Overrides
  if (currentState != INTRO && currentState != CALL && currentState != NOTIFICATION && currentState != PETTED) {
    if (musicIsPlaying) currentState = MUSIC;
    else currentState = IDLE;
  }

  // Timeouts
  if ((currentState == NOTIFICATION || currentState == PETTED) && (millis() - stateStartTime > NOTIFY_TIMEOUT)) {
    currentState = IDLE;
  }

  switch (currentState) {
    case INTRO:        drawIntro(); break;
    case IDLE:         drawFace(); break;
    case MUSIC:        drawMusicMode(); break;
    case NOTIFICATION: drawNotification(); break;
    case CALL:         drawNotification(); break;
    case PETTED:       drawHappyHeart(); break;
  }
}

void drawIntro() {
  if (millis() - lastFrameTime >= 40) {
    lastFrameTime = millis();
    u8g2.clearBuffer();
    u8g2.drawBitmap(0, 0, 16, 64, daichi_intro_frames[currentFrame]);
    u8g2.sendBuffer();
    currentFrame++;
    if (currentFrame >= DAICHI_INTRO_FRAME_COUNT) {
      currentFrame = 0;
      currentState = IDLE;
    }
  }
}

void drawFace() {
  if (millis() - lastFrameTime >= FRAME_DELAY) {
    lastFrameTime = millis();
    u8g2.clearBuffer();
    
    // Smooth Fast Render
    const uint8_t* framePtr = NULL;
    if (currentMood.type == HUYKHONG_ARRAY) {
      framePtr = ((const uint8_t* const*)currentMood.data)[currentFrame];
    } else if (currentMood.type == STRUCT_ARRAY || currentMood.type == EXTRA_BITMAP_ARRAY) {
      framePtr = ((const uint8_t (*)[1024])currentMood.data)[currentFrame];
    }

    if (framePtr) u8g2.drawBitmap(0, 0, 16, 64, framePtr);
    
    // Status Clock
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(95, 62);
    u8g2.print(chronos.getHourC());
    u8g2.print(":");
    if(chronos.getMinute() < 10) u8g2.print("0");
    u8g2.print(chronos.getMinute());

    u8g2.sendBuffer();
    
    currentFrame++;
    if (currentFrame >= currentMood.count) {
      pickRandomMood();
    }
  }
}

void drawHappyHeart() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.drawGlyph(54, 32, 0x2665); 
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(48, 50, "LOVE");
  u8g2.sendBuffer();
}

void drawMusicMode() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 12, "MUSIC");
  u8g2.drawHLine(0, 15, 128);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.setCursor(0, 32);
  u8g2.print(trackTitle.substring(0, 15));
  u8g2.setCursor(0, 45);
  u8g2.print(trackArtist.substring(0, 20));
  for (int i = 0; i < 128; i += 8) {
    int h = random(5, 20);
    u8g2.drawBox(i, 64-h, 4, h);
  }
  u8g2.sendBuffer();
}

void drawNotification() {
  u8g2.clearBuffer();
  u8g2.drawRFrame(0, 0, 128, 64, 4);
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(10, 15, headerText.c_str());
  u8g2.drawHLine(5, 18, 118);
  u8g2.setFont(u8g2_font_helvB08_tr);
  u8g2.setCursor(10, 40);
  u8g2.print(bodyText.substring(0, 20));
  u8g2.sendBuffer();
}

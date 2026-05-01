/*
 * Mochi EDC - Sentient Engine v2.1.3 (Chill Edition)
 * Features: Sequential Mood Playback, Slower Framerate, Longer Transitions
 */

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <ChronosESP32.h>
#include "all_frames.h"         
#include "250frames.h"          
#include "daichi_intro.h"       
#include "animation_bitmaps.h"  

#define I2C_SDA 21
#define I2C_SCL 22
#define TOUCH_PIN 4  

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
ChronosESP32 chronos("Mochi-EDC"); 

enum RenderType { HUYKHONG_ARRAY, STRUCT_ARRAY, EXTRA_BITMAP_ARRAY };
enum MochiState { INTRO, IDLE, NOTIFICATION, MUSIC, CALL, PETTED, TRANSITION };
MochiState currentState = INTRO;

struct MoodData {
  RenderType type;
  const void* data;
  int count;
  int diagnosticIdx;
};

MoodData moodHappyLegacy = { HUYKHONG_ARRAY, (const void*)frames, 772, -1 };
MoodData currentMood = moodHappyLegacy;

int extraMoodTestCounter = 0;
unsigned long transitionStartTime = 0;
const unsigned long TRANSITION_PAUSE = 1000; // 1 second pause

// Metadata
String trackTitle = "Unknown", trackArtist = "Unknown";
bool musicIsPlaying = false;
String headerText = "", bodyText = "";
unsigned long stateStartTime = 0;
const unsigned long NOTIFY_TIMEOUT = 8000; 

#define FRAME_DELAY 85 // Much slower, premium feel
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

void pickNextMoodForTest() {
  if (extraMoodTestCounter >= EXTRA_MOODS_COUNT) {
    extraMoodTestCounter = 0;
    Serial.println("--- Completed All Extra Moods ---");
  }

  currentMood.type = EXTRA_BITMAP_ARRAY;
  currentMood.data = (const void*)extraMoods[extraMoodTestCounter].frames;
  currentMood.count = extraMoods[extraMoodTestCounter].count;
  currentMood.diagnosticIdx = extraMoodTestCounter;

  Serial.print("--- Now Playing EXTRA MOOD #");
  Serial.println(extraMoodTestCounter);

  extraMoodTestCounter++;
  currentFrame = 0;
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
}

void loop() {
  chronos.loop(); 

  // Touch
  if (currentState == IDLE && touchRead(TOUCH_PIN) < 30) {
    currentState = PETTED;
    stateStartTime = millis();
  }

  // Auto-switch states
  if (currentState != INTRO && currentState != CALL && currentState != NOTIFICATION && currentState != PETTED && currentState != TRANSITION) {
    if (musicIsPlaying) currentState = MUSIC;
    else currentState = IDLE;
  }

  // Timeouts
  if ((currentState == NOTIFICATION || currentState == PETTED) && (millis() - stateStartTime > NOTIFY_TIMEOUT)) {
    currentState = IDLE;
  }

  // Transition Logic
  if (currentState == TRANSITION && (millis() - transitionStartTime > TRANSITION_PAUSE)) {
    currentState = IDLE;
    pickNextMoodForTest();
  }

  switch (currentState) {
    case INTRO:        drawIntro(); break;
    case IDLE:         drawFace(); break;
    case TRANSITION:   drawStaticFace(); break; // Stare at user during pause
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
      pickNextMoodForTest();
    }
  }
}

void drawFace() {
  if (millis() - lastFrameTime >= FRAME_DELAY) {
    lastFrameTime = millis();
    u8g2.clearBuffer();
    
    const uint8_t* framePtr = NULL;
    if (currentMood.type == HUYKHONG_ARRAY) {
      framePtr = ((const uint8_t* const*)currentMood.data)[currentFrame];
    } else if (currentMood.type == EXTRA_BITMAP_ARRAY) {
      framePtr = ((const uint8_t (*)[1024])currentMood.data)[currentFrame];
    }

    if (framePtr) u8g2.drawBitmap(0, 0, 16, 64, framePtr);
    drawOverlays();
    u8g2.sendBuffer();
    
    currentFrame++;
    if (currentFrame >= currentMood.count) {
      currentState = TRANSITION;
      transitionStartTime = millis();
    }
  }
}

// Function to keep showing the last frame during the transition pause
void drawStaticFace() {
    u8g2.clearBuffer();
    const uint8_t* framePtr = NULL;
    // Show frame 0 of the NEXT mood or last frame of previous? 
    // Let's show frame 0 of the mood we JUST finished for a smooth look.
    if (currentMood.type == EXTRA_BITMAP_ARRAY) {
       framePtr = ((const uint8_t (*)[1024])currentMood.data)[currentMood.count - 1];
    }
    if (framePtr) u8g2.drawBitmap(0, 0, 16, 64, framePtr);
    drawOverlays();
    u8g2.sendBuffer();
}

void drawOverlays() {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(0, 62);
    u8g2.print("IDX:");
    u8g2.print(currentMood.diagnosticIdx);
    u8g2.setCursor(95, 62);
    u8g2.print(chronos.getHourC());
    u8g2.print(":");
    if(chronos.getMinute() < 10) u8g2.print("0");
    u8g2.print(chronos.getMinute());
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

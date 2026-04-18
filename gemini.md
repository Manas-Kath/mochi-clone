# Project "Mochi EDC": Technical Roadmap & Status

## 1. Project Overview
The goal is to design and build a high-polish, **Sentient EDC Companion** inspired by the Dasai Mochi aesthetic. Reverted to a high-stability bitmap engine for maximum performance on I2C hardware.

### Primary Philosophy:
* **Aesthetic First:** 1.3" OLED focus with smooth 25FPS rendering.
* **Sentient Engine:** A state machine that reacts to touch and phone notifications.
* **Social & Smart:** BLE integration for time sync and alerts.

---

## 2. Hardware Stack
* **Microcontroller:** ESP32 DevKit.
* **Display:** 1.3" SH1106 I2C OLED (U8g2 Driver).
* **Interaction:** Capacitive Touch (GPIO 4).
* **Connectivity:** BLE via Chronos App.

---

## 3. Core Functionality (Sentient Engine v1.4 - Reverted)
Successfully reverted to the Bitmap-based engine to resolve I2C bottleneck/jitter issues:

1.  **INTRO State:** Plays a smooth boot animation (`daichi_intro.h`).
2.  **IDLE State:** Randomly cycles between two mood sequences:
    * `moodHappy` (772 frames - Original Mochi).
    * `moodMisc` (225 frames - Extra expressions from `250frames.h`).
    * Displays real-time Clock overlay.
3.  **MUSIC State:** Shows Track Title & Artist with random dancing bars.
4.  **NOTIFICATION State:** Interrupts face to show messages for 8 seconds.
5.  **PETTED State:** Triggered by GPIO 4 touch. Displays a "LOVE" heart reaction.

---

## 4. Key Resources & Progress
* **Library Stack:** `U8g2`, `ChronosESP32`, `NimBLE-Arduino`.
* **Backup:** GIF-based engine (v2.0) backed up in `backups/v2.0_GIF_Backup/` for future transition to SPI hardware.
* **Asset Pipeline:** Pre-calculated 128x64 bitmaps for maximum I2C efficiency.

---

## 5. Current Stage: "Stable Base Re-established"
The project is back in its most performant state for current hardware.

### Current Progress:
1.  **Performance Fixed:** Eliminated jitter and choppiness by returning to raw bitmap rendering.
2.  **State Machine Preserved:** Kept all smart features (Notifications, Music, Touch) while swapping the rendering engine.
3.  **Clean Setup:** Removed all redundant files; `hinhdongesp32/` is now minimal and optimized.

### Next Technical Hurdles:
* **MPU6050:** Integrating motion-based reactions (Dizzy/Sleepy).
* **Hardware Upgrade:** Transitioning to SPI TFT (ST7789) to re-enable the high-quality GIF engine at 60FPS.

---
**Status:** *Stable - v1.4 Bitmap Engine Active*
**Lead:** *Manaswin Kath (RAS Vice Chair)*

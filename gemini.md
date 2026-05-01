# Project "Mochi EDC": Technical Roadmap & Master Context

## 1. Project Philosophy & Motto
**Motto:** *"Soul in the Shell, Intelligence in the Pocket."*
The Mochi EDC is not just a keychain; it is a **Sentient Robotics Companion**. The goal is to replicate and exceed the high-polish aesthetic of the Dasai Mochi Gen 3 using accessible ESP32 hardware. We prioritize emotional expression, premium animation pacing, and seamless smartphone integration to create an EDC item that feels "alive."

---

## 2. Current State: Sentient Engine v2.1.4
The project has evolved through multiple rendering architectures. We successfully resolved the I2C bottleneck of the 1.3" OLED by transitioning from a real-time GIF decoding engine (v2.0) back to a **Pre-Converted Bitmap Engine (v2.1)**. This ensures a rock-solid 60FPS feel without the jitter or "math lag" associated with real-time decompression.

### Key Milestones Achieved:
*   **Visual Fidelity:** Implemented **Bayer (Ordered) Dithering** to simulate grayscale on a monochrome screen, achieving the famous "Huykhong Look."
*   **Smart Integration:** Full **Chronos BLE** support for real-time Clock sync, Phone Battery monitoring, Music Metadata (Track/Artist), and Notification/Caller ID interrupts.
*   **Physical Interaction:** Capacitive Touch sensing on **GPIO 4** allows the user to "pet" Mochi, triggering a dedicated "LOVE" state.
*   **Stability:** Optimized I2C bus clock to 400kHz and implemented buffered rendering to eliminate screen flashing and outlines.

---

## 3. The Technical Toolchain
We have developed a custom asset pipeline to convert high-resolution video recordings of Mochi into high-performance C++ headers:

*   **`hinhdongesp32.ino`**: The heart of Mochi. A multi-state machine managing INTRO, IDLE, MUSIC, NOTIFICATION, and PETTED states. It handles sequential diagnostic playback and randomized mood selection.
*   **`mochi_tuner_gui.py`**: A real-time OpenCV tuning dashboard. It allows the user to adjust Black-Level clipping (to kill glare), Contrast, and Gamma while watching a live dithered preview of the 1080p source video.
*   **`mochi_mass_exporter.py`**: The precision slicer. It takes timestamps or frame ranges from the tuned video and exports them as optimized 128x64 horizontal-packed bitmap arrays stored in PROGMEM.
*   **`animation_bitmaps.h`**: The central index that maps dozens of independent animation files into a unified `MoodData` structure for the ESP32.
*   **`DasaiBitmaps/`**: The primary asset vault containing 40+ high-quality expressions converted from original Dasai sources.

---

## 4. Current File Structure
*   `hinhdongesp32/`: Active Arduino Sketch folder.
    *   `DasaiBitmaps/`: Header files for all expressions.
    *   `frames/`: Legacy raw bitmap frames (for fallback stability).
    *   `animation_bitmaps.h`: The mood registry.
*   `0419(1)/`: The "Gold Mine" folder containing the pre-cropped 1080p master video (`0419(1).mp4`) and GIF.
*   `backups/v2.0_GIF_Backup/`: Safety copy of the real-time GIF engine for future SPI hardware tests.

---

## 5. Future Roadmap: The "Ultra-Sentient" Upgrade
1.  **Asset Slicing:** Manually curate the 80+ expressions from `0419(1).mp4` using the Tuner GUI to reach the full vocabulary limit.
2.  **MPU6050 Physics:** Integrate an IMU to trigger "Dizzy" animations during movement and "Sleepy" animations during stillness.
3.  **LDR Integration:** Add a light sensor so Mochi's eyes "squint" or react to bright sunlight vs. dark pockets.
4.  **Hardware Evolution (v3.0):** Transition to a **1.3" Square ST7789 SPI TFT (240x240)**. This will enable 16-bit color, 80MHz data rates, and anti-aliased rendering, moving beyond the limits of I2C.

---
**Lead Developer:** *Manaswin Kath (RAS Vice Chair)*
**Status:** *Stable - v2.1.4 Engine Active*

# Mochi EDC - Sentient Companion

A high-polish, open-source clone of the Dasai Mochi, designed to run smoothly on ESP32 microcontrollers.

This repository features the **Sentient Engine (v2.1.4)** which renders 40+ dynamic, high-quality dithered animations at a rock-solid 25FPS on 1.3" I2C OLED displays using raw bitmaps.

## Key Features

*   **Huykhong-Style Dithering:** We use Bayer (Ordered) dithering to simulate a CRT-like grayscale effect on monochrome displays.
*   **Sentient State Machine:** Mochi has a mind of its own. It randomly swaps moods every few seconds and transitions seamlessly.
*   **Chronos BLE Integration:** Syncs with your smartphone to provide real-time updates for:
    *   Music playback (Track & Artist visualization)
    *   App Notifications
    *   Incoming Calls
    *   Phone Battery and Clock
*   **Touch Interaction:** Capable of detecting capacitive touch (via GPIO 4) to trigger a dedicated "Petting / LOVE" reaction.

## The Asset Pipeline

This repo includes a full Python toolchain to extract, tune, and convert raw MP4 video into optimized C++ header arrays:

1.  **`mochi_tuner_gui.py`:** An interactive, frame-by-frame GUI (using OpenCV) to dial in the perfect `BlackLevel`, `Contrast`, and `Gamma` to remove camera noise and produce the best dithered image.
2.  **`mochi_mass_exporter.py`:** A batch converter that takes specific frame ranges and custom overrides, automating the process of generating optimized 128x64 horizontal-packed bitmap arrays (`.h` files).

## Hardware Setup

*   **Controller:** ESP32 DevKit (or ESP32-S3)
*   **Display:** 1.3" SH1106 I2C OLED
*   **Touch:** A wire attached to GPIO 4 for petting

*Make sure to use the "Huge APP (3MB No OTA)" Partition Scheme in the Arduino IDE when flashing.*

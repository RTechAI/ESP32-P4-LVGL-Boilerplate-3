# ESP32-P4-WIFI6-Touch-LCD-7B LVGL Boilerplate (3-Page UI Edition)

[![ESP-IDF Version](https://shields.io)](https://github.com)
[![LVGL Version](https://shields.io)](https://github.com)
[![License](https://shields.io)](LICENSE)

A production-ready, zero-bloat, open-source-friendly HMI starter template and architecture explicitly built for the **Waveshare ESP32-P4-WIFI6-Touch-LCD-7B** development board (featuring the high-performance dual-core RISC-V ESP32-P4 MCU and a 7-inch MIPI DSI display). 

This version delivers a pristine, multi-file **3-Page Embedded UI Framework** (Dashboard, System, and Admin views) optimized for real-world products. Completely free of subscription fees, platform lock-ins, or bloated factory demo spaghetti.

**Powered by the ForgeUI Architectural Framework.**  
**Created by:** Scott Forster  
**Contact:** forgeui.esp32@gmail.com  
**Current Release:** `FORGEUI_P4_591__RC1_PUBLIC_RELEASE_PREP__README_STARTED__2026-05-10`

---

## 🚀 Overview & Philosophy
ForgeUI is a product-oriented embedded UI framework focused on stable hardware bring-up, clean architecture, reusable LVGL patterns, and deep hardware-level integration with the ESP32-P4 pipeline. 

Unlike manufacturer demos that pack tightly coupled smart-home widgets into an unmodifiable file structure, ForgeUI delivers an **"Empty Can" Architecture**. It takes care of 100% of the painful peripheral bring-up, leaving you with a clean, stable, and teachable canvas ready for immediate application development.

---

## ✨ Proven RC1 Core Features
- **3-Page Core Workflow:** Clean architectural boundaries separating the `Dashboard` (Home screen/Launcher workspace), `System Page`, and `Admin Panel`.
- **Integrated Control Modals:** Drop-down persistent Status Drawer layout, custom System Overlay Keyboard, and an Access-Restricted Admin Gate passcode pad.
- **Wireless Coexistence Staging:** Vetted routines for scanning, connecting, and disconnecting networks smoothly without memory leakage.
- **Timekeeping Persistence:** Real-time sync pipeline linking the high-precision external DS3231 hardware clock to the internal system ticker.
- **Config-Driven Architecture:** Toggle core physical hardware modules or visual screens instantly via compile-time feature switches.

---

## 🛠️ Proven Hardware Integration
This boilerplate is fully verified, calibrated, and rock-stable on the following hardware layout:
- **Primary Development Kit:** Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
- **Core Microcontroller:** ESP32-P4 (High-performance dual-core RISC-V MCU)
- **Wireless Coprocessor:** Onboard ESP32-C6 (Providing Hosted Wi-Fi 6 via SDIO transport)
- **Display Controller:** EK79007 (7-inch MIPI DSI, 800x480 resolution, 180° rotation applied)
- **Touch Digitizer:** GT911 (Capacitive multi-touch)
- **Hardware Real-Time Clock:** DS3231 (Power-off time retention over shared I2C bus)
- **Audio Path Codec:** ES8311 (Speaker output, runtime volume control sliders, and async beep testing operational)

---

## 📦 Software Stack & Component Matrix
Avoid version incompatibility compilation errors. This baseline relies on a fully vetted, known-good component layout tree:
- **Framework:** ESP-IDF v5.5.4
- **Graphics Engine:** LVGL v9.2.2
- **Wireless Infrastructure:** `esp_hosted` v2.9.7 & `esp_wifi_remote` v1.3.0
- **Display Core Pipeline:** `esp_lvgl_port` v2.7.2 & `esp_lcd_ek79007` v1.0.4
- **Touch Digitizer Core:** `esp_lcd_touch_gt911` v1.2.0~2
- **Audio Architecture:** `esp_codec_dev` v1.2.0
- **Board Support Package:** Waveshare BSP v1.0.2

---

## 🚀 Quick Start & Build Environment

### 1. Recommended Workspace
- **OS:** Windows / Linux / macOS
- **IDE:** VS Code with the official ESP-IDF Extension installed.
- **Toolchain:** ESP-IDF v5.5.4

### 2. Compilation Commands
Open your terminal inside the project root directory and execute the following sequence:

```bash
# Set the compilation target to the P4 chip
idf.py set-target esp32p4

# Clear out old build cache and generate clean CMake files
idf.py fullclean

# Compile the project
idf.py build

# Flash the binary and open the serial monitor
idf.py flash monitor
```

---

## 📐 Core Architectural Design Rules
To keep your product stable under heavy async workloads, the framework strictly enforces decoupling between the visual layout and system truth:

1. **`main.c` is the Orchestrator:** It solely owns board startup, peripheral bring-up, internal LVGL initialization, backend task staging, and the core runtime loop.
2. **UI Does NOT Own System Truth:** Backends own truth. The UI layer only renders state changes and bubbles clean user-intent notifications back down to system processes.
3. **Safe LVGL Threading Context:** Visual changes must execute strictly from an LVGL-safe context. Backend/event tasks must *never* directly mutate a UI object from an unsafe async thread.
4. **Compile-Time Configuration:** All structural feature ownership flags reside cleanly inside `00_ForgeUI_Config.h`.

---

## 📁 Project Structure (3-Page UI Layout)

```text
├── main.c                  # Core board bring-up, HW orchestration, and runtime loops
├── 00_ForgeUI_Config.h     # Global feature config, compile-time switches, and pin maps
│
├── main/
│   ├── 01_FG_HMI.c / .h        # Main multi-page LVGL runtime shell allocation
│   │
│   ├── 10_UI_Dashboard.c       # Screen 1: Primary control dashboard & launcher canvas
│   ├── 11_UI_PreOp.c           # Logo splash, boot loading steps, and pre-op sequences
│   ├── 12_UI_System.c          # Screen 2: System parameters & setting tiles hub
│   ├── 13_UI_Admin.c           # Screen 3: Privileged setup options panel
│   ├── 14_UI_Header.c          # Persistent status bar & ticker rendering
│   ├── 15_UI_Keyboard.c        # Global system input overlay keyboard template
│   ├── 16_UI_StatusDrawer.c    # Drop-down quick status adjustments panel
│   ├── 17_UI_AdminGate.c       # Security PIN keypad lock barrier layout
│   │
│   ├── 20_RTC.c                # System time synchronization handlers
│   ├── 21_RTC_DS3231.c         # Hardware I2C DS3231 driver layer
│   │
│   ├── 30_Audio.c              # ES8311 volume management and notification audio
│   ├── 30_WIFI.c               # esp_hosted wireless scanner loops and event handlers
│   │
│   ├── 40_SD.c                 # SD card detection, storage mounting, and directory building
│   │
│   ├── 50_SDMMC_BUS.c / .h     # Custom SDMMC bus abstractions (Placeholder)
│   ├── CMakeLists.txt          # Main component build directives
│   └── idf_component.yml       # Component registry dependency bindings
```

---

## ⚙️ Feature Configuration System
Scale firmware size up or down instantly via compile-time flags inside `00_ForgeUI_Config.h`. Toggle features between `0` (disabled) and `1` (enabled) to optimize builds or isolate hardware bugs:

```c
#define FORGEUI_ENABLE_WIFI   1
#define FORGEUI_ENABLE_SD     1
#define FORGEUI_ENABLE_AUDIO  1
#define FORGEUI_ENABLE_RTC    1
```

---

## 📡 Subsystem Implementation Profiles

### Display & Video Pipeline
- **Drivers:** Stable hardware-accelerated LVGL rendering via `esp_lcd_ek79007`.
- **Orientation:** Display is mapped to a 180-degree rotation using the BSP layer:
  ```c
  bsp_display_rotate(disp, LV_DISPLAY_ROTATION_180);
  ```

### Real-Time Clock (RTC) Architecture
- The external **DS3231** provides non-volatile power-off time retention (Address: `0x68`, Port: `I2C_NUM_0`, SDA: `GPIO 7`, SCL: `GPIO 8`).
- At boot, time is read from the DS3231 to seed the internal ESP system clock. During runtime, the system queries the high-speed internal system clock. User adjustments made via the UI are written back to the DS3231 instantly.

### Hosted Wi-Fi 6 Coexistence Configuration
The ESP32-P4 does not have a native on-chip RF radio. Wireless capability is achieved using the onboard **ESP32-C6 coprocessor** communicating over an SDIO transport layer via `esp_hosted`.
- **Bus Setup:** SDIO Slot 1, 4-bit bus width, running at a 40 MHz clock.
- **Coexistence Safeguards:** PSRAM XIP and TWT are explicitly disabled to guarantee absolute bus stability.
- **CRITICAL INITIALIZATION ORDER:** In `main.c`, you **MUST** follow this sequence exactly:
  1. Display Startup & LVGL Environment bring-up
  2. UI Object Tree creation
  3. `fg_wifi_init()` (Hosted Wi-Fi initialization)
  4. `fg_sd_init()` & `fg_sd_test()` (SD card filesystem mounting)
  *Do not mount the SD card before the Wi-Fi stack initializes, as esp_hosted must claim the SDMMC slot handshake first.*

### SD Card Storage Media
Supports card presence detection, directory structure instantiation, and live read/write speed testing. *Note: Live full-formatting is bypassed in this release to maintain shared-bus ownership rules with the active Hosted Wi-Fi stack.*

---

## 📂 Visual Assets & Documentation Setup
The project workspace includes step-by-step menuconfig reference screenshots under `📂 /docs/setup`. 

### Included Reference Guides:
- **ESP-Hosted & WiFi Remote** component profiles
- **4-bit SDIO transport** bus parameters
- **PSRAM XIP tracking** toggles & TWT disable registers
- **SD IO / RTC clock lines** pin routing maps
- **Power Management** registers layout

### Recommended Verification Screenshots for Release:
- `Dashboard UI Frame` | `System Page Matrix` | `Wi-Fi Toggle Card` | `SD Status Tile`
- `Audio Volume Slider` | `Status Drawer Overlay` | `Admin Passcode Gate` | `Theme Profiles`

---

## 🗺️ Roadmap
- [ ] Implement multiple runtime global color themes (Nebula Blue, Carbon, Atlas Light).
- [ ] Add polished async page navigation slide/fade transitions.
- [ ] Incorporate secure cryptographic Admin authentication algorithms.
- [ ] Integrate a SPIFFS mini-database engine for storing system preferences.
- [ ] Expand the modular icon pipeline into a fully reusable custom widget library.
- [ ] Complete cross-platform port baseline targeting the ESP32-S3.

---

## ⚖️ License & Open Source Compliance
The core ForgeUI codebase, application layout files, configuration headers, and architectural frameworks are governed strictly under the **ForgeUI Source Available License v1.0** (See the `LICENSE` file in the root directory). 

- **Permitted Use:** Free for personal, educational, evaluation, internal testing, and commercial binary product distribution.
- **Restricted Use:** Public redistribution of raw ForgeUI source code files or repackaging into competing frameworks/starter-kits is forbidden without explicit written permission.
- **Attribution:** Any firmware build using this baseline must retain visible project attribution (e.g., *"Powered by ForgeUI"* or *"Built using ForgeUI"*) inside About panels or system informational readouts.

*Upstream code modules, component dependencies, and drivers (LVGL, Espressif ESP-IDF, Waveshare BSP) are owned and licensed independently by their respective copyright holders under standard MIT and Apache 2.0 terms.* Review `THIRD_PARTY_LICENSES.md` for exact structural boundaries.

---

## ⚠️ Disclaimer
This framework is provided "AS IS" without warranty of any kind. Developers must thoroughly validate electrical safety, thermal layout profiles, and overall hardware loop stability before deploying this boilerplate into a commercial environment.

## Keywords

ESP32-P4 UI Framework  
ESP32-P4 LVGL Example  
ESP32-P4 Touchscreen UI  
ESP32-P4 GUI Framework  
LVGL ESP-IDF Starter Project  
Waveshare ESP32-P4 Example  
ESP32-P4 Boilerplate  
ESP32-P4 Dashboard UI  
LVGL Touchscreen Framework  
ESP32-P4 Embedded GUI

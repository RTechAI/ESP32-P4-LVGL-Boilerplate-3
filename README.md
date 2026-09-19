# ESP32-P4 LVGL Boilerplate — ForgeUI

This repository is a public ESP-IDF and LVGL 9 boilerplate/reference for the **Waveshare ESP32-P4-WIFI6-Touch-LCD-7B**, including an ESP32-P4, MIPI DSI display, and GT911 touch baseline.

The public `v1.0.0` release documents a ForgeUI-based three-page embedded UI baseline and its associated hardware integration.

ForgeUI is developed by [RTechAI](https://github.com/RTechAI). This repository remains a public technical reference while current ForgeUI development has expanded separately into ForgeUI Studio.

**Created by:** Scott Forster  
**Contact:** forgeui.esp32@gmail.com  

---

## 🚀 Overview & Philosophy
ForgeUI is a product-oriented embedded UI framework focused on hardware bring-up, clean architecture, reusable LVGL patterns, and deep hardware-level integration with the ESP32-P4 pipeline.

Unlike manufacturer demos that pack tightly coupled smart-home widgets into an unmodifiable file structure, ForgeUI delivers an **"Empty Can" Architecture**. It takes care of 100% of the painful peripheral bring-up, leaving you with a clean, stable, and teachable canvas ready for immediate application development.

---

## ✨ Documented v1.0.0 Baseline Features
- **3-Page Core Workflow:** Clean architectural boundaries separating the `Dashboard` (Home screen/Launcher workspace), `System Page`, and `Admin Panel`.
- **Integrated Control Modals:** Drop-down persistent Status Drawer layout, custom System Overlay Keyboard, and an Access-Restricted Admin Gate passcode pad.
- **Wireless Coexistence Staging:** Vetted routines for scanning, connecting, and disconnecting networks smoothly without memory leakage.
- **Timekeeping Persistence:** Real-time sync pipeline linking the high-precision external DS3231 hardware clock to the internal system ticker.
- **Config-Driven Architecture:** Toggle core physical hardware modules or visual screens instantly via compile-time feature switches.

---

## 🛠️ Tested Hardware Integration
The documented public baseline was tested with the following hardware layout:
- **Primary Development Kit:** Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
- **Core Microcontroller:** ESP32-P4 (High-performance dual-core RISC-V MCU)
- **Wireless Coprocessor:** Onboard ESP32-C6 (Providing Hosted Wi-Fi 6 via SDIO transport)
- **Display Controller:** EK79007 (7-inch MIPI DSI, 800x480 resolution, 180° rotation applied)
- **Touch Digitizer:** GT911 (Capacitive multi-touch)
- **Hardware Real-Time Clock:** DS3231 (Power-off time retention over shared I2C bus)
- **Audio Path Codec:** ES8311 (Speaker output, runtime volume control sliders, and async beep testing operational)

---

## 📦 Software Stack & Component Matrix
Avoid version incompatibility compilation errors. The documented public baseline uses the following component layout tree:
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
- **Environment:** Install and initialize the ESP-IDF v5.5.4 environment before running `idf.py`.

### 2. Compilation Commands
Open your terminal inside the project root directory and execute the following sequence:

```bash
# Set the compilation target to the P4 chip
idf.py set-target esp32p4

# Compile the project
idf.py build

# Flash the binary and open the serial monitor
idf.py flash monitor
```

If automatic port detection does not work, supply your serial port with `idf.py -p <port> flash monitor`. Run `idf.py fullclean` before `idf.py build` only when a clean reconfiguration is required.

---

## 📐 Core Architectural Design Rules
To keep your product stable under heavy async workloads, the framework strictly enforces decoupling between the visual layout and system truth:

1. **`main/main.c` is the Orchestrator:** It solely owns board startup, peripheral bring-up, internal LVGL initialization, backend task staging, and the core runtime loop.
2. **UI Does NOT Own System Truth:** Backends own truth. The UI layer only renders state changes and bubbles clean user-intent notifications back down to system processes.
3. **Safe LVGL Threading Context:** Visual changes must execute strictly from an LVGL-safe context. Backend/event tasks must *never* directly mutate a UI object from an unsafe async thread.
4. **Compile-Time Configuration:** All structural feature ownership flags reside in `main/00_ForgeUI_Config.h`.

---

## 📁 Project Structure (3-Page UI Layout)

```text
├── CMakeLists.txt                 # ESP-IDF project entry point
├── dependencies.lock              # Locked managed-component versions
├── main/
│   ├── main.c                     # Board bring-up, orchestration, and runtime loop
│   ├── 00_ForgeUI_Config.h        # Feature switches and configuration
│   ├── 02_FG_HMI.c / .h           # Multi-page LVGL runtime shell
│   ├── 05_FG_Icons.c / .h         # Icon registration
│   ├── 10_UI_Dashboard.c / .h     # Dashboard view
│   ├── 11_UI_PreOp.c / .h         # Pre-operation view
│   ├── 12_UI_System.c / .h        # System view
│   ├── 13_UI_Admin.c / .h         # Admin view
│   ├── 14_UI_Header.c / .h        # Persistent status header
│   ├── 15_UI_Keyboard.c / .h      # Keyboard overlay
│   ├── 16_UI_AdminGate.c / .h     # Admin gate and UI style
│   ├── 17_UI_StatusDrawer.c / .h  # Status drawer and reactor modal
│   ├── 20_RTC.c / .h              # RTC integration
│   ├── 30_Audio.c / .h            # Audio integration
│   ├── 30_WIFI.c / .h             # ESP-Hosted Wi-Fi integration
│   ├── 40_SD.c / .h               # SD-card integration
│   ├── assets/icons/              # Compiled LVGL icon assets
│   ├── CMakeLists.txt             # Main component registration
│   └── idf_component.yml          # Component manifest
├── components/bsp_extra/          # Auxiliary board-support component
├── docs/setup/                    # Setup screenshots and architecture notes
├── LICENSE                         # ForgeUI Source Available License
└── THIRD_PARTY_LICENSES.md         # Third-party notices
```

---

## ⚙️ Feature Configuration System
Scale firmware size up or down via compile-time flags inside `main/00_ForgeUI_Config.h`. Toggle features between `0` (disabled) and `1` (enabled) to optimize builds or isolate hardware bugs:

```c
#define FORGEUI_ENABLE_WIFI   1
#define FORGEUI_ENABLE_SD     1
#define FORGEUI_ENABLE_AUDIO  1
#define FORGEUI_ENABLE_RTC    1
```

---

## 📡 Subsystem Implementation Profiles

### Display & Video Pipeline
- **Drivers:** The documented baseline uses the Waveshare BSP and `esp_lcd_ek79007` for LVGL rendering.
- **Orientation:** The current source does not apply an explicit `bsp_display_rotate(...)` override; use the BSP-configured panel orientation for this board baseline.

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

## Roadmap

Current ForgeUI development continues through the broader [ForgeUI](https://forgeui.co.nz) ecosystem and [ForgeUI Studio](https://studio.forgeui.co.nz).

This repository remains focused on its public ESP32-P4/LVGL boilerplate and hardware-reference role.

## Current ForgeUI Studio

ForgeUI development has expanded into [ForgeUI](https://forgeui.co.nz) and [ForgeUI Hosted Studio](https://studio.forgeui.co.nz).

ForgeUI Hosted Studio is available for public registration.

This repository remains the public ESP32-P4/LVGL boilerplate and technical reference.

## About ForgeUI

[ForgeUI](https://forgeui.co.nz) is developed by [RTechAI](https://github.com/RTechAI), the GitHub home for ForgeUI public repositories and reference work.

ForgeUI Studio is the current visual embedded UI/HMI development environment, and ForgeUI Hosted Studio is its hosted application.

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

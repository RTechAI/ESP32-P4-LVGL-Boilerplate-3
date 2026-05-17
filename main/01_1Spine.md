# ESP32-P4-WIFI6-Touch-LCD-7B LVGL Boilerplate Spine (Reactor UI Edition)

============================================================
WHAT IS THE SPINE?
============================================================

The Spine is the master architecture, layout mapping, and ownership 
document for the ESP32-P4-WIFI6-Touch-LCD-7B LVGL Boilerplate.

The Spine is intended to evolve alongside the ForgeUI codebase.
It must be updated whenever:
- Core framework architecture shifts occur
- Directory file or module ownership changes
- Internal runtime behavior changes
- Peripheral hardware truths change
- Product and appliance design layout targets adapt
- Known-good verification baselines are updated

If something becomes important enough to repeatedly explain, it belongs in the Spine.
The Spine is the single source of truth for the current known-good baseline. If code, 
comments, behavior, or documentation drift away from the Spine, the Spine must be 
updated or the implementation corrected. Everything eventually traces back here.

Architecture Philosophy:
Keep it simple. Keep it stable. Keep it teachable. Keep it modular. 
Everything hardware-abstracted works flawlessly first → then build product on top.

============================================================
SAVE POINT
============================================================

FORGEUI_P4_598__THEME_SELECTOR_AND_UI_MODULE_TOGGLES_OK__2026-05-15

============================================================
PROJECT IDENTITY & SPECIFICATION
============================================================

Project Name:
ESP32-P4-WIFI6-Touch-LCD-7B LVGL Boilerplate

Target Hardware Board:
Waveshare ESP32-P4-WIFI6-Touch-LCD-7B (7-inch MIPI DSI, 800x480 Layout)

Active Production Branch:
ForgeUI-Reactor (Modular Appliance Launcher & Modal UI)

Created by:
Scott Forster

Contact:
forgeui.esp32@gmail.com

Powered by the ForgeUI Architectural Framework. (ForkGuard retired for this baseline).

============================================================
CURRENT MISSION & PRODUCT DIRECTION
============================================================

Build a fully alive, hardware-proven, high-performance ESP32-P4 baseline, then evolve the Reactor branch into a decoupled, modular, appliance-style launcher UI suitable for commercial product deployment.

The UI must be consistent, clean, teachable, sellable, easy to modify, stable on hardware, and not over-engineered. This is no longer a one-off demo screen.

Current Next Milestone Mission:
RTC dashboard tile cleanup and improved manual Date/Time configuration setting entry workflows.

============================================================
HARD ARCHITECTURAL RULES
============================================================

1. NO DEMOS in runtime path. No hidden factory dependencies or bloatware.
2. NO GUESSING. Isolate and test raw hardware bounds directly.
3. NO HIDDEN DUPLICATE DRIVERS. Eliminate file system and component pollution.
4. NO RANDOM PER-WIDGET STYLING. Enforce strict centralized stylesheets.
5. NO SUBSYSTEM OWNS ANOTHER SUBSYSTEM. Maintain strict decoupling boundaries.
6. MAIN.C OWNS BOOT ORDER ONLY. Orchestrates startup sequences and driver loops.
7. BACKENDS OWN SYSTEM TRUTH. UI only renders cached state and bubbles up user intent.

============================================================
BRANCH STATUS
============================================================

main branch:
Stable, traditional multi-page static ForgeUI baseline project.

ForgeUI-Reactor branch:
Active touch-first launcher/appliance UI production branch. Implements hidden tabview 
routing context, launcher-card layouts, shared modal layers, modular icon pipelines, 
and an overlay/page split architecture.

============================================================
PROVEN WORKING HARDWARE & REACTOR FEATURES
============================================================

- Display Pipeline (EK79007 MIPI DSI): OK (Native BSP panel orientation verified)
- Touch Interface (GT911 I2C): OK (Multi-touch operational, Tab swipe tuning ADDED)
- Graphics Engine (LVGL v9.2.2): OK (PSRAM optimization verified)
- Navigation Model: Hidden Tabview Routing Engine (Top selection bar hidden)
- Screen Transitions: Instant workspace switching via `LV_ANIM_OFF`
- Network Stack (ESP32-C6 Hosted WiFi): OK (Scan / Connect / Password / Disconnect / Forget)
- Storage Filesystem (SD Card): OK (256GB SDHC, 4-bit, 40MHz R/W pass)
- Audio Engine (ES8311 Codec): OK (Speaker out, beep testing, volume sliders operational)
- Persistent Header: Live clock tracking, header toggle, and header clock path alive
- Security Gate: Admin PIN keypad barrier system layout and dev switches active
- System Launcher Hub: Modular matrix icon hub alive and proven
- Reactor Modal Framework: Stable async modal generation and deletion engine
- Integrated Modals: Brightness, Sound, Wi-Fi, and Storage Modals verified
- Modular Configuration: Theme selector engine, Dashboard tile toggles, and tile gating OK

============================================================
DISPLAY ROTATION TRUTH
============================================================

The Waveshare ESP32-P4-WIFI6-Touch-LCD-7B native display orientation is already correct out of the box.

CRITICAL REMOVAL RULE: Do NOT call `bsp_display_rotate(disp, LV_DISPLAY_ROTATION_180);`.
Forcing this rotation layer causes an ugly visible startup screen flip, white frame transition lags, and unnecessary redraw processing overhead. ForgeUI runs strictly in native BSP panel alignment.

============================================================
PLATFORM TRUTH: WIRELESS COEXISTENCE & SD RULES
============================================================

The ESP32-P4 processor contains NO native on-chip RF Wi-Fi radio. Wireless functionality is achieved using the onboard ESP32-C6 chip acting as a coprocessor over a high-speed 4-bit, 40MHz SDIO transport layer handled by `esp_hosted`.

CRITICAL COEXISTENCE INIT ORDER:
To avoid SDIO bus timeouts, resource conflicts, and driver panics, initialization must follow this sequence exactly:
1. `fg_wifi_init()`
2. `fg_sd_init()`
3. `fg_sd_test()`

Reason: `esp_hosted` must claim the SDMMC slot handshake first. The SD card filesystem safely attaches to the shared bus afterward. Breaking this order causes hard storage initialization timeouts and wireless instability.

WIFI ASYNC RULE: Never run heavy blocking logic inside wireless event handlers. The event must trip a simple flag, and the main system task handles processing inside `fg_wifi_pump()`.

============================================================
SDMMC BUS STRATEGY
============================================================

Files `50_SDMMC_BUS.h` and `50_SDMMC_BUS.c` exist within the component directory structure.
Status: UNUSED / PLACEHOLDER LAYER. Do NOT build or declare a custom bus layer yet. The working system relies strictly on the WiFi-first, SD-second initialization block workaround.

============================================================
ESP-IDF BASELINE
============================================================

Version: ESP-IDF v5.5.4 (Strict version matching enforced)
Required Core Defs:
- ESP-Hosted enabled
- WiFi Remote enabled
- PSRAM XIP FROM PSRAM = OFF (Must remain disabled to prevent linker timing crashes)

============================================================
CURRENT ACTIVE PROJECT STRUCTURE
============================================================

The main/ component directory must match this structure cleanly:

├── main.c                  # Board boot sequence and driver lifecycle management
├── 00_ForgeUI_Config.h     # Global feature compile flags, pin maps, and style IDs
├── 01_1Spine.md            # This master architecture specification document
├── 01_FG_HMI.h             # HMI abstraction definitions layer
├── 02_FG_HMI.c             # Base canvas allocation and display task port mapping
├── 05_FG_Icons.h / .c      # Custom modular icon structures and image pipelines
├── 10_UI_Dashboard.h / .c  # Screen 1: Primary appliance launcher card hub
├── 11_UI_PreOp.h / .c      # Logo splash, boot loading steps, and pre-op sequences
├── 12_UI_System.h / .c     # Screen 2: System preference arrays & icon matrix launcher
├── 13_UI_Admin.h / .c      # Screen 3: Privileged setup options canvas
├── 14_UI_Header.h / .c     # Top persistent tracking bar and ticker rendering
├── 15_UI_Keyboard.h / .c    # Global system input overlay keyboard template
├── 16_UI_AdminGate.h / .c  # Security PIN keypad lock barrier layout
├── 16_UI_Style.h / .c      # Centralized theme control definitions (Nebula Blue active)
├── 17_UI_StatusDrawer.h / .c # Drop-down quick status adjustments panel
├── 17_UI_ReactorModal.h / .c # Decoupled shared overlay and async modal builder engine
├── 20_RTC.h / .c           # Hardware clock sync tasks and time conversions
├── 30_Audio.h / .c         # Codec volume settings and feedback tone generation
├── 30_WIFI.h / .c          # esp_hosted wireless scanner loop and event handlers
├── 40_SD.h / .c            # Storage media mounts and file structure mapping
├── 50_SDMMC_BUS.h / .c     # Unused placeholder layer (Do not invoke)
├── CMakeLists.txt          # Main component build directives
└── idf_component.yml       # Espressif registry dependency bindings

============================================================
REACTOR UI NAVIGATION & MODAL ARCHITECTURE
============================================================

The Reactor branch changes the navigation model from traditional scrolling tabs into a touch-first, icon-driven appliance interface.

1. Hidden Tabview Routing Engine: Do NOT delete the internal LVGL tabview. It is utilized as the background routing engine. The top tab bar is completely hidden.
2. Interaction Flows: Users tap large high-visibility launcher cards on the Dashboard to route to specific screens, or interact with a modular icon matrix on the System screen.
3. Page Nav Helpers: Explicit route changes utilize `fg_hmi_go_dashboard()`, `fg_hmi_go_system()`, or `fg_hmi_go_admin()` with `LV_ANIM_OFF` to bypass sluggish side-scrolling visual redraws on the 7-inch panel.
4. Appliance Style Hubs: Features like Brightness, Sound, Wi-Fi, and Storage are no longer crammed onto a single messy settings page. They launch instantly inside clean, modern, isolated asynchronous popups managed by the `ReactorModal` framework.
5. Primary vs Secondary Navigation: Touch-driven icon/card selection acts as primary navigation. Horizontal swiping across the hidden tabview remains active as a secondary navigation model. Do not rely on swipe tracking as the primary path as it can introduce physical drag resistance on the panel canvas.

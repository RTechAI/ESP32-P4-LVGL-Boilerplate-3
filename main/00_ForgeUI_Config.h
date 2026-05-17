
// ============================================================
// ForgeUI Configuration System
// ============================================================
//
// ForgeUI
// ESP32-P4 Embedded UI Framework
//
// Created by:
// Scott Forster
//
// Contact:
// forgeui.esp32@gmail.com
//
// Official Project:
// https://github.com/RTechAI/ForgeUI-P4
//
// Framework Purpose:
// - reusable ESP32-P4 application baseline
// - LVGL v9 UI framework
// - ESP-IDF hardware integration layer
// - clean modular embedded UI architecture
//
// Target Hardware:
// - Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
//
// Core Stack:
// - ESP-IDF
// - LVGL
// - ESP-Hosted
// - ESP32-C6 WiFi Remote
//
// ============================================================

// ============================================================
// ForgeUI Global Configuration.
// ============================================================

#pragma once

#include "driver/i2c_master.h"


// ============================================================
// ForgeUI Master Feature Switches
// ============================================================

#define FORGEUI_ENABLE_AUDIO               1
#define FORGEUI_ENABLE_WIFI                1
#define FORGEUI_ENABLE_SD                  1

// Optional application/workflow modules
#define FORGEUI_ENABLE_PREOP               1


// ============================================================
// ForgeUI Header Options
// ============================================================
//
// FORGEUI_ENABLE_HEADER
//
// 1 = Header enabled
// 0 = Header disabled
//
// Controls:
// - top tab/header bar
// - header overlay layer
//
// ============================================================

#define FORGEUI_ENABLE_HEADER              1

// ============================================================
// ForgeUI Time Options
// ============================================================
//
// FORGEUI_ENABLE_RTC (TIME ON SCREEN)
//
// 1 = Time system ON
// 0 = Time system OFF
//
// Controls:
// - RTC backend
// - Time tile/modal
// - runtime timekeeping
//
//
// FORGEUI_SHOW_HEADER_CLOCK
//
// 1 = Header clock visible
// 0 = Header clock hidden
//
// Requires:
// FORGEUI_ENABLE_HEADER = 1
//
#define FORGEUI_ENABLE_RTC                 1
#define FORGEUI_SHOW_HEADER_CLOCK          1
// ============================================================

// ============================================================
// ForgeUI Admin Gate Options
// ============================================================

#define FORGEUI_ENABLE_ADMIN_GATE          1
#define FORGEUI_ADMIN_DEFAULT_PIN          "1234"

// 0 = disabled / no timeout yet
#define FORGEUI_ADMIN_SESSION_TIMEOUT_MIN  0

// ============================================================
// ForgeUI Visual Active Styles
// ============================================================

//#define FORGEUI_STYLE_ACTIVE                FORGEUI_STYLE_ATLAS_LIGHT 
//#define FORGEUI_STYLE_ACTIVE                  FORGEUI_STYLE_NEBULA_BLUE 
#define FORGEUI_STYLE_ACTIVE               FORGEUI_STYLE_REACTOR

#define FORGEUI_ENABLE_DASHBOARD_TILES   1

// ============================================================
// Theme IDs LEAVE ETHESE ALONE
// ============================================================

#define FORGEUI_STYLE_ATLAS_LIGHT          0
#define FORGEUI_STYLE_NEBULA_BLUE          1
#define FORGEUI_STYLE_REACTOR              2


// ============================================================
// ForgeUI Icon Sets
// ============================================================
//
// Current Reactor proof uses 48px icons.
//
// Rules:
// - icon declarations live in 05_FG_Icons.c/h
// - CMake must compile generated icon .c files
// - use underscores only
// - never use hyphens in C symbol names
//
// ============================================================

#define FORGEUI_ICON_SET_LIGHT_32          0
#define FORGEUI_ICON_SET_DARK_32           1
#define FORGEUI_ICON_SET_LIGHT_48          2
#define FORGEUI_ICON_SET_DARK_48           3
#define FORGEUI_ICON_SET_DARK_64           4

// Active icon pack

// ============================================================
// ForgeUI Home Button
// ============================================================
//
// Small persistent Reactor home navigation button.
//
// Layout:
//
// - 28x28 home icon
// - 36x36 touch/button tile
//
// Purpose:
//
// - quick return to Dashboard
// - global utility navigation
// - visible across all tabs/pages
//
// Reactor Direction:
//
// Main 48px tiles remain the primary launcher UI.
//
// Home button acts as:
//
// - utility navigation
// - fast escape/home control
//
// ============================================================

#define FORGEUI_ICON_SET_ACTIVE            FORGEUI_ICON_SET_DARK_48

#define FORGEUI_ENABLE_HOME_BUTTON         1

// 36x36 outer touch tile
#define FORGEUI_HOME_BUTTON_SIZE           36

// overlay position
#define FORGEUI_HOME_BUTTON_MARGIN_X       25
#define FORGEUI_HOME_BUTTON_MARGIN_Y       25


// ============================================================
// Wi-Fi Backend Options
// ============================================================
//
// ESP32-P4 has NO native Wi-Fi radio.
//
// Wi-Fi path:
//
// ESP32-P4
// -> ESP-Hosted
// -> SDIO
// -> ESP32-C6
// -> Wi-Fi Remote
//
// ============================================================

#define FORGEUI_WIFI_BACKEND_NONE          0
#define FORGEUI_WIFI_BACKEND_HOSTED        1

#if FORGEUI_ENABLE_WIFI
#define FORGEUI_WIFI_BACKEND               FORGEUI_WIFI_BACKEND_HOSTED
#else
#define FORGEUI_WIFI_BACKEND               FORGEUI_WIFI_BACKEND_NONE
#endif

// ============================================================
// RTC Backend Options
// ============================================================

#define FORGEUI_RTC_BACKEND_NONE           0
#define FORGEUI_RTC_BACKEND_ESP_NVS        1
#define FORGEUI_RTC_BACKEND_DS3231         2


// ============================================================
// Active RTC Backend
// ============================================================

#if FORGEUI_ENABLE_RTC
#define FORGEUI_RTC_BACKEND                FORGEUI_RTC_BACKEND_DS3231
#else
#define FORGEUI_RTC_BACKEND                FORGEUI_RTC_BACKEND_NONE
#endif


// ============================================================
// DS3231 Settings
// ============================================================

#define FORGEUI_DS3231_ADDR                0x68
#define FORGEUI_DS3231_I2C_PORT            I2C_NUM_0

#define FORGEUI_DS3231_SDA_IO              7
#define FORGEUI_DS3231_SCL_IO              8


// ============================================================
// ForgeUI Status Drawer Options
// ============================================================

#define FORGEUI_STATUS_DRAWER_LEFT         0
#define FORGEUI_STATUS_DRAWER_RIGHT        1
#define FORGEUI_STATUS_DRAWER_SIDE         FORGEUI_STATUS_DRAWER_RIGHT

#define FORGEUI_STATUS_DRAWER_WIDTH        240

#define FORGEUI_STATUS_DRAWER_HANDLE_W     28
#define FORGEUI_STATUS_DRAWER_HANDLE_H     120

#define FORGEUI_STATUS_DRAWER_Y            80
#define FORGEUI_STATUS_DRAWER_BOTTOM_GAP   20

// ============================================================
// ForgeUI Reactor Modal Options
// ============================================================

#define FORGEUI_REACTOR_MODAL_STYLE_FLAT       0
#define FORGEUI_REACTOR_MODAL_STYLE_GLOW       1
#define FORGEUI_REACTOR_MODAL_STYLE_GLASS      2

#define FORGEUI_REACTOR_MODAL_STYLE_ACTIVE     FORGEUI_REACTOR_MODAL_STYLE_FLAT

#define FORGEUI_REACTOR_MODAL_ANIM_NONE        0
#define FORGEUI_REACTOR_MODAL_ANIM_FADE        1
#define FORGEUI_REACTOR_MODAL_ANIM_ZOOM        2

#define FORGEUI_REACTOR_MODAL_ANIM_ACTIVE      FORGEUI_REACTOR_MODAL_ANIM_NONE

#define FORGEUI_REACTOR_MODAL_DIM_BACKGROUND   0

#define FORGEUI_REACTOR_MODAL_WIDTH            500
#define FORGEUI_REACTOR_MODAL_HEIGHT           520

// ============================================================
// Reactor Modal Runtime State
// ============================================================
//
// Current Stable Reactor Baseline:
//
// - Reactor hub tile launcher active
// - Shared Reactor modal system active
// - Shared keyboard overlay active
// - Brightness modal stable
// - Sound modal stable
// - WiFi modal stable
// - Storage modal stable
// - 48px icon pack active
// - async modal delete active
// - stale LVGL object guards active
// - modal close reset bug fixed
//
// Rules:
//
// - Reactor modals must NEVER assume LVGL
//   objects still exist during timer refresh
//
// - ALL timer-driven UI refresh paths must
//   validate object pointers before use
//
// - Shared overlays/keyboards/modals must
//   use async deletion where possible
//
// - Backend modules own runtime truth
//   UI only renders state
//
// ============================================================
// Powered by ForgeUI 
// use it emnjoy it make it your next UI and let me know how you go, happy to help
// Your succesfull build is important, get going with easy UI alive ForgeUI
// ============================================================

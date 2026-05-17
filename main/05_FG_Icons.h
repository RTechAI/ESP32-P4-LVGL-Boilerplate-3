#pragma once

// ============================================================
// ForgeUI Icon Interface
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
// Purpose:
// Shared icon accessor interface for ForgeUI UI modules.
//
// UI layers should access icons through this
// interface rather than directly referencing
// LVGL image asset symbols.
//
// This architecture allows:
//
// - centralized icon ownership
// - future icon pack switching
// - theme-aware icon systems
// - cleaner module separation
// - reusable UI asset pipelines
//
// Architecture Rules:
//
// - no UI logic
// - no styling ownership
// - no backend ownership
// - no runtime state ownership
//
// Current Reactor Baseline:
//
// - centralized icon access layer
// - shared 48px icon pipeline
// - touch-first launcher support
//
// Future Direction:
//
// - dark/light icon packs
// - runtime icon scaling
// - appliance-specific icon sets
// - customer branding packs
//
// Target Hardware:
//
// - Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
//
// Core Stack:
//
// - ESP-IDF
// - LVGL v9
//
// ============================================================

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif


// ============================================================
// Shared Icon Accessors
// ============================================================

const void *fg_icon_system(void);
const void *fg_icon_admin(void);
const void *fg_icon_sdcard(void);
const void *fg_icon_sound(void);
const void *fg_icon_wifi(void);
const void *fg_icon_brightness(void);
const void *fg_icon_time(void);
const void *fg_icon_home(void);

#ifdef __cplusplus
}
#endif
#pragma once

// ============================================================
// ForgeUI Keyboard Overlay
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
// Shared on-screen LVGL keyboard helper.
//
// Features:
//
// - reusable singleton keyboard
// - textarea attach/detach
// - password entry support
// - READY/CANCEL handling
// - shared overlay behaviour
//
// Used By:
//
// - WiFi password entry
// - future PIN entry
// - text input fields
// - future setup wizards
//
// Architecture Rules:
//
// - UI layer only
// - no backend ownership
// - no hardware ownership
// - no runtime truth storage
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
// Keyboard API
// ============================================================

// Attach keyboard to textarea and show
void fg_keyboard_attach(lv_obj_t *ta);

// Hide keyboard overlay
void fg_keyboard_hide(void);


#ifdef __cplusplus
}
#endif
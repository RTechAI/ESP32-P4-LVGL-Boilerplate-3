#pragma once

// ============================================================
// ForgeUI System UI
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
// Main runtime system and settings UI builder.
//
// Responsibilities:
//
// - build System launcher hub
// - expose system controls
// - launch Reactor modals
// - render backend status
// - send user intent to backend modules
//
// Controlled through:
//
//   00_ForgeUI_Config.h
//
// Architecture Rules:
//
// - UI layer only
// - no backend ownership
// - no hardware ownership
// - no runtime truth storage
//
// Backend modules own system truth.
// This layer renders state and sends intent only.
//
// Current Reactor Direction:
//
// System acts as:
//
// - settings hub
// - hardware proof hub
// - modular launcher
// - shared modal entry point
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
// System Builder
// ============================================================

void ui_system_build(lv_obj_t *parent);


#ifdef __cplusplus
}
#endif
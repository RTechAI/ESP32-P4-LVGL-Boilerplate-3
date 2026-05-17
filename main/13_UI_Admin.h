#pragma once

// ============================================================
// ForgeUI Admin UI
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
// Optional protected administration UI.
//
// Responsibilities:
//
// - build Admin page content
// - host protected UI and workflows
// - provide service and maintenance area
// - support future protected tooling
//
// Controlled through:
//
//   00_ForgeUI_Config.h
//
// Optional Features:
//
// - Admin gate
// - PIN access
// - RFID access
// - maintenance tools
// - diagnostics
// - firmware/service tools
//
// Architecture Rules:
//
// - UI layer only
// - no backend ownership
// - no hardware ownership
// - no runtime truth storage
//
// Current Reactor Direction:
//
// Admin may be launched through:
//
// - Dashboard launcher cards
// - System launcher hub
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
// Admin Builder
// ============================================================

void ui_admin_build(lv_obj_t *parent);


#ifdef __cplusplus
}
#endif
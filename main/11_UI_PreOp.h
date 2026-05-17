#pragma once

// ============================================================
// ForgeUI Pre Operation UI
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
// Optional workflow and application module.
//
// Current Direction:
//
// - workflow placeholder
// - industrial workflow area
// - ForkGuard-compatible expansion point
//
// Architecture Rules:
//
// - UI layer only
// - no backend ownership
// - no hardware ownership
// - no runtime truth storage
//
// Controlled By:
//
//   FORGEUI_ENABLE_PREOP
//
// Defined in:
//
//   00_ForgeUI_Config.h
//
// Future Direction:
//
// - digital forms
// - checklist systems
// - inspection workflows
// - operator workflows
// - telemetry-linked actions
//
// Module Design Goals:
//
// This module is intended to remain:
//
// - optional
// - removable
// - product configurable
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
// Pre Operation Builder
// ============================================================

void ui_preop_build(lv_obj_t *parent);


#ifdef __cplusplus
}
#endif
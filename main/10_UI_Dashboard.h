#pragma once

// ============================================================
// ForgeUI Dashboard UI
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
// Dashboard and home launcher UI builder.
//
// Responsibilities:
//
// - build Dashboard page content
// - create launcher cards
// - provide top-level UI entry points
// - route user navigation intent
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
// Dashboard acts as:
//
// - home screen
// - launcher hub
// - appliance-style entry point
//
// Primary Navigation:
//
// - launcher cards
// - icon taps
//
// Secondary Navigation:
//
// - hidden LVGL tabview swipe routing
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
// Dashboard Builder
// ============================================================

void ui_dashboard_build(lv_obj_t *parent);


#ifdef __cplusplus
}
#endif
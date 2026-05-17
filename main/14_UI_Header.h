#pragma once

// ============================================================
// ForgeUI Header Overlay
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
// Global overlay and header UI layer.
//
// Responsibilities:
//
// - persistent overlay rendering
// - top-right RTC clock display
// - lightweight runtime status display
// - shared overlay positioning
//
// Current Features:
//
// - top-right RTC clock
// - persistent foreground overlay
//
// Architecture Rules:
//
// - UI layer only
// - no backend ownership
// - no hardware ownership
// - no runtime truth storage
//
// Backend modules own runtime truth.
// This layer renders display state only.
//
// Future Direction:
//
// - status icons
// - notifications
// - admin/session indicators
// - telemetry indicators
// - product badges
// - live connection indicators
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


// ============================================================
// Header Overlay
// ============================================================

// Create persistent overlay/header objects
void fg_header_create(lv_obj_t *parent);

// Refresh runtime header values
void fg_header_refresh(void);
#pragma once

// ============================================================
// ForgeUI Admin Gate
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
// Optional PIN-based Admin access layer.
//
// Features:
//
// - Admin UI protection
// - unlock state handling
// - PIN entry UI
// - relock support
// - optional compile-time bypass
//
// Controlled Through:
//
//   FORGEUI_ENABLE_ADMIN_GATE
//
// Architecture Rules:
//
// - UI layer only
// - no backend ownership
// - no hardware ownership
// - no persistent runtime storage
//
// Current Reactor Direction:
//
// Admin gate acts as:
//
// - protected UI boundary
// - session entry point
// - appliance-style protected access layer
//
// Future Direction:
//
// - RFID unlock
// - role and session system
// - timeout support
// - persistent PIN storage
// - service and user roles
// - customer access levels
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

#include <stdbool.h>

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif


// ============================================================
// Admin Gate API
// ============================================================

// Build locked/admin gate UI
void fg_admin_gate_build(lv_obj_t *parent);

// Returns current unlock state
bool fg_admin_is_unlocked(void);

// Force admin relock
void fg_admin_lock(void);


#ifdef __cplusplus
}
#endif
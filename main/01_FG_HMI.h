#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// ForgeUI HMI Runtime Interface
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
// Lightweight HMI-level runtime interface for UI startup,
// page routing, and development-only HMI behaviour flags.
//
// System-wide ownership belongs in:
//
//   00_ForgeUI_Config.h
//
// This file should only contain:
//
// - HMI entry points
// - HMI-only development flags
// - UI navigation helpers
// - lightweight UI routing declarations
//
// This file must NOT own:
//
// - backend state
// - hardware configuration
// - subsystem lifecycle
// - duplicated global feature config
//
// ============================================================


// ============================================================
// Admin Access Control
// ============================================================
//
// FG_ADMIN_DEV_UNLOCK
//
// Development helper only.
//
// 1 = Admin UI unlocked
// 0 = Admin protection enabled
//
// Current purpose:
//
// - development access
// - UI testing
// - routing validation
//
// Future direction:
//
// - PIN gate
// - RFID login
// - role-based access
// - session timeout handling
//
// ============================================================

#define FG_ADMIN_DEV_UNLOCK  1


// ============================================================
// HMI Navigation Entry Points
// ============================================================
//
// These helpers route the internal LVGL
// tabview/navigation engine.
//
// Reactor UI uses:
//
// Primary navigation:
// - icon/card tap
//
// Secondary navigation:
// - swipe gestures
//
// ============================================================

void fg_hmi_init(void);

void fg_hmi_go_dashboard(void);
void fg_hmi_go_preop(void);
void fg_hmi_go_system(void);
void fg_hmi_go_admin(void);

#ifdef __cplusplus
}
#endif
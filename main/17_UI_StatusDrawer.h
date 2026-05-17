#pragma once

// ============================================================
// ForgeUI Status Drawer
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
// Global slide-out runtime status drawer.
//
// Features:
//
// - edge toggle handle
// - overlay drawer panel
// - runtime status display
// - config-controlled left/right side
//
// Architecture Rules:
//
// - UI layer only
// - no backend ownership
// - no hardware ownership
// - no runtime truth storage
//
// Backend modules own runtime truth.
// Drawer only renders display state.
//
// Future Direction:
//
// - live backend state
// - notifications
// - alerts
// - telemetry indicators
// - quick settings
// - product status badges
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


// ============================================================
// Status Drawer API
// ============================================================

// Create global status drawer
void fg_status_drawer_create(void);

// Toggle drawer open/closed
void fg_status_drawer_toggle(void);
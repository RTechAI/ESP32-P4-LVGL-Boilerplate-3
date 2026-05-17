#pragma once

// ============================================================
// ForgeUI Reactor Modal
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
// Shared Reactor modal and overlay system.
//
// Responsibilities:
//
// - create shared modal overlays
// - provide reusable modal containers
// - support shared close handling
// - manage modal lifecycle ownership
//
// Current Features:
//
// - singleton modal system
// - reusable overlay layer
// - shared close callback support
// - Reactor-compatible modal styling
//
// Architecture Rules:
//
// - UI layer only
// - no backend ownership
// - no hardware ownership
// - no runtime truth storage
//
// Current Ownership:
//
// Feature modules own:
//
// - modal content
// - feature logic
// - runtime state
//
// Reactor modal owns:
//
// - overlay lifecycle
// - modal container
// - close callback handling
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
// Close Callback
// ============================================================

typedef void (*fg_reactor_modal_close_cb_t)(void);


// ============================================================
// Modal API
// ============================================================

// Open Reactor modal and return content container
lv_obj_t *fg_reactor_modal_open(const char *title,
                                const void *icon);

// Register optional modal close callback
void fg_reactor_modal_set_close_cb(
    fg_reactor_modal_close_cb_t cb);

// Close active Reactor modal
void fg_reactor_modal_close(lv_obj_t *modal);


#ifdef __cplusplus
}
#endif
#pragma once

// ============================================================
// ForgeUI SD Storage System
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
// Shared SD card and filesystem backend.
//
// Features:
//
// - SD mount/init
// - SD read/write test
// - ForgeUI folder structure
// - storage reset/rebuild
// - folder listing
// - runtime status helpers
//
// Runtime Model:
//
// - backend owns SD state
// - UI sends intent only
// - backend owns filesystem lifecycle
//
// Important:
//
// Hosted WiFi and SDMMC share hardware paths.
//
// Current Stable Boot Order:
//
//   WiFi first
//   -> SD second
//
// Architecture Rules:
//
// - backend layer only
// - no LVGL ownership
// - no UI styling
// - no direct UI dependencies
//
// Future Direction:
//
// - databases
// - telemetry storage
// - export systems
// - cloud sync
// - backup/restore systems
//
// Target Hardware:
//
// - Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
//
// Core Stack:
//
// - ESP-IDF
// - ESP-Hosted
// - SDMMC
// - LVGL v9
//
// ============================================================

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


// ============================================================
// Core
// ============================================================

// Initialise SD backend
bool fg_sd_init(void);

// Simple SD read/write validation test
bool fg_sd_test(void);


// ============================================================
// Status Helpers
// ============================================================

bool fg_sd_is_ready(void);

const char *fg_sd_status_text(void);

const char *fg_sd_last_action_text(void);

const char *fg_sd_size_text_get(void);


// ============================================================
// Storage Actions
// ============================================================

// Create ForgeUI folder structure
bool fg_sd_create_folders(void);

// Write simple boot marker file
bool fg_sd_write_boot_marker(void);

// Async UI-safe reset task
bool fg_sd_reset_async(void);


// ============================================================
// Fast Storage Reset
// ============================================================
//
// Fast app-level reset.
//
// Deletes:
//
//   /sdcard/ForgeUI
//
// Rebuilds:
//
//   clean ForgeUI folder structure
//
// Does NOT:
//
// - full-format the SD card
// - remount storage
//
// ============================================================

bool fg_sd_reset_storage_blocking(void);


// ============================================================
// Folder / File View
// ============================================================

// List ForgeUI root folders/files
bool fg_sd_list_forgeui(char *out,
                        int out_len);


#ifdef __cplusplus
}
#endif
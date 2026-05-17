#pragma once

// ============================================================
// ForgeUI Audio System
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
// Shared ForgeUI audio backend.
//
// Responsibilities:
//
// - initialize speaker and audio path
// - manage output volume
// - provide simple audio test playback
//
// Current Features:
//
// - BSP audio initialization
// - speaker codec support
// - runtime volume control
// - sine-wave speaker test beep
//
// Architecture Rules:
//
// - backend layer only
// - no LVGL ownership
// - no UI styling
// - UI sends intent only
//
// Controlled Through:
//
//   FORGEUI_ENABLE_AUDIO
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

#include "esp_err.h"


// ============================================================
// Audio API
// ============================================================

// Init audio system (called automatically on first use)
esp_err_t fg_audio_init(void);

// Set speaker volume (0–100)
esp_err_t fg_audio_set_volume(int volume);

// Play test beep
esp_err_t fg_audio_test_beep(void);
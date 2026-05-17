#pragma once

// ============================================================
// ForgeUI RTC System
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
// Shared runtime RTC and timekeeping layer.
//
// Responsibilities:
//
// - runtime clock ownership
// - RTC backend abstraction
// - DS3231 integration
// - NVS fallback persistence
// - date/time formatting helpers
//
// Controlled Through:
//
//   FORGEUI_RTC_BACKEND
//
// Current Runtime Model:
//
// - ESP system time = active runtime clock
// - DS3231 = persistent hardware source
// - NVS = fallback persistence
//
// Architecture Rules:
//
// - UI never owns runtime time truth
// - runtime reads use ESP system time
// - DS3231 is used at boot restore and set/apply
// - BSP owns I2C bus initialization
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


// Initialise runtime RTC system
void fg_rtc_init(void);


// Set runtime/system time
void fg_rtc_set(int year,
                int month,
                int day,
                int hour,
                int min);


// Get runtime/system time
void fg_rtc_get(int *year,
                int *month,
                int *day,
                int *hour,
                int *min,
                int *sec);


// Format helpers
void fg_rtc_format_day(char *out, int max);

void fg_rtc_format_time(char *out, int max);

void fg_rtc_format_header(char *out, int max);
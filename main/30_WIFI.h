#pragma once

// ============================================================
// ForgeUI Hosted WiFi System
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
// ESP32-P4 hosted WiFi backend.
//
// Features:
//
// - hosted WiFi initialization
// - scan
// - connect/disconnect
// - IP status tracking
// - network forget/reset
// - scan result caching
//
// Runtime Model:
//
// - backend owns WiFi state
// - UI sends intent only
// - event handler sets flags
// - fg_wifi_pump() processes deferred work
//
// Hardware:
//
// ESP32-P4 host
// + onboard ESP32-C6 WiFi
// + ESP-Hosted / WiFi Remote
//
// Future Direction:
//
// - saved networks
// - reconnect manager
// - RSSI reporting
// - cloud sync
// - NTP time sync
//
// Target Hardware:
//
// - Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
//
// Core Stack:
//
// - ESP-IDF
// - ESP-Hosted
// - ESP WiFi Remote
// - LVGL v9
//
// ============================================================

#include <stdbool.h>


// Initialise hosted WiFi backend
void fg_wifi_init(void);

// Runtime WiFi pump/task helper
void fg_wifi_pump(void);


// Status helpers
bool fg_wifi_is_ready(void);

bool fg_wifi_is_connected(void);

const char *fg_wifi_status_text(void);

const char *fg_wifi_ip_text(void);


// WiFi scan helpers
void fg_wifi_scan_start(void);

int fg_wifi_get_scan_results(char ssids[][33],
                             int max);


// Connection helpers
void fg_wifi_connect(const char *ssid,
                     const char *pass);

void fg_wifi_disconnect(void);

void fg_wifi_forget(void);
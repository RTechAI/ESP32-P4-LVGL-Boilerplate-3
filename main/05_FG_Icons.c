// ============================================================
// ForgeUI Icon Pipeline
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
// Centralized icon asset ownership and access layer.
//
// Responsibilities:
//
// - expose shared UI icon accessors
// - isolate LVGL image asset symbols
// - centralize icon ownership
// - support future icon pack switching
//
// Architecture Rules:
//
// - no direct UI logic
// - no styling ownership
// - no runtime state ownership
// - no hardware ownership
//
// Current Reactor baseline uses:
//
// - 48px icon assets
//
// Future Direction:
//
// - multiple icon packs
// - theme-aware icon sets
// - runtime icon scaling
// - dark/light variants
// - product-specific icon packs
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

#include "05_FG_Icons.h"
#include "00_ForgeUI_Config.h"

// ============================================================
// External LVGL Image Assets
// ============================================================

LV_IMAGE_DECLARE(fg_icon_admin_48px);
LV_IMAGE_DECLARE(fg_icon_brightness_48px);
LV_IMAGE_DECLARE(fg_icon_sound_48px);
LV_IMAGE_DECLARE(fg_icon_system_48px);
LV_IMAGE_DECLARE(fg_icon_time_48px);
LV_IMAGE_DECLARE(fg_icon_wifi_48px);
LV_IMAGE_DECLARE(fg_icon_sd_card_48px);
LV_IMAGE_DECLARE(fg_icon_home_28px);

// ============================================================
// Icons
// ============================================================

const void *fg_icon_system(void)
{
    return &fg_icon_system_48px;
}

const void *fg_icon_admin(void)
{
    return &fg_icon_admin_48px;
}

const void *fg_icon_brightness(void)
{
    return &fg_icon_brightness_48px;
}

const void *fg_icon_sound(void)
{
    return &fg_icon_sound_48px;
}

const void *fg_icon_wifi(void)
{
    return &fg_icon_wifi_48px;
}

const void *fg_icon_sdcard(void)
{
    return &fg_icon_sd_card_48px;
}

const void *fg_icon_time(void)
{
    return &fg_icon_time_48px;
}

const void *fg_icon_home(void)
{
    return &fg_icon_home_28px;
}

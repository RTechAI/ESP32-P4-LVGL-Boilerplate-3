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
// Global header and status overlay layer.
//
// Responsibilities:
//
// - top-right clock display
// - persistent overlay elements
// - lightweight runtime status display
// - shared overlay positioning
//
// Current Features:
//
// - RTC date/time display
// - persistent foreground overlay
// - Reactor-compatible header layer
//
// Architecture Rules:
//
// - overlay layer only
// - no backend ownership
// - no hardware ownership
// - no low-level display ownership
// - lightweight refresh path only
//
// Backend modules own runtime truth.
// Header only renders display state.
//
// Current Ownership:
//
// - RTC formatting comes from 20_RTC.c
// - visual styling comes from 16_UI_Style.c
// - feature gating comes from 00_ForgeUI_Config.h
//
// Future Direction:
//
// - WiFi status icon
// - SD status icon
// - admin/session state
// - notifications
// - telemetry indicators
// - product/system badges
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
// Includes
// ============================================================

#include "14_UI_Header.h"

#include "20_RTC.h"

#include "16_UI_Style.h"

#include "00_ForgeUI_Config.h"


// ============================================================
// Local State
// ============================================================

static lv_obj_t *g_blk    = NULL;
static lv_obj_t *g_lbl_dt = NULL;


// ============================================================
// Header Create
// ============================================================

void fg_header_create(lv_obj_t *parent)
{
    g_blk = lv_obj_create(parent);

    lv_obj_remove_style_all(g_blk);

    lv_obj_set_size(g_blk,
                    LV_SIZE_CONTENT,
                    LV_SIZE_CONTENT);

    lv_obj_set_style_pad_all(g_blk, 0, 0);

    lv_obj_set_style_border_width(g_blk, 0, 0);

    lv_obj_set_style_bg_opa(g_blk,
                            LV_OPA_TRANSP,
                            0);


    // ========================================================
    // Position
    // ========================================================

    lv_obj_align(g_blk,
                 LV_ALIGN_TOP_RIGHT,
                 -12,
                 10);


    // ========================================================
    // Date/Time Label
    // ========================================================

#if FORGEUI_SHOW_HEADER_CLOCK

    g_lbl_dt = lv_label_create(g_blk);

    lv_obj_set_style_text_align(g_lbl_dt,
                                LV_TEXT_ALIGN_RIGHT,
                                0);

    lv_obj_set_style_text_font(g_lbl_dt,
                               &lv_font_montserrat_14,
                               0);

    fg_style_apply_label_dim(g_lbl_dt);

    lv_label_set_text(g_lbl_dt,
                      "Fri 17\n14:32");

#else

    g_lbl_dt = NULL;

#endif


    // ========================================================
    // Overlay Priority
    // ========================================================

    lv_obj_move_foreground(g_blk);
}

// ============================================================
// Header Refresh
// ============================================================

void fg_header_refresh(void)
{
#if FORGEUI_SHOW_HEADER_CLOCK

    char buf[32];

    fg_rtc_format_header(buf, sizeof(buf));

    if (g_lbl_dt) {
        lv_label_set_text(g_lbl_dt, buf);
    }

#endif

    if (g_blk) {
        lv_obj_move_foreground(g_blk);
    }
}
// ============================================================
// ForgeUI Pre Operation UI
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
// Optional workflow and application module.
//
// Current Purpose:
//
// - placeholder workflow page
// - future industrial workflow area
// - ForkGuard-compatible expansion point
//
// Architecture Rules:
//
// - UI layer only
// - no backend ownership
// - no hardware ownership
// - no persistent runtime truth
//
// Future Direction:
//
// - digital forms
// - inspection workflows
// - checklist systems
// - operator workflows
// - telemetry-linked actions
// - compliance capture
// - guided workflow systems
//
// Module Design Goals:
//
// This module is intended to remain:
//
// - optional
// - removable
// - product configurable
//
// Future ownership should remain controlled
// through ForgeUI feature and configuration layers.
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

#include "11_UI_PreOp.h"
#include "16_UI_Style.h"


// ============================================================
// Pre Operation Builder
// ============================================================

void ui_preop_build(lv_obj_t *parent)
{
    fg_style_apply_screen(parent);

    lv_obj_set_style_pad_all(parent, 16, 0);


    // ========================================================
    // Main Panel
    // ========================================================

    lv_obj_t *panel = lv_obj_create(parent);

    fg_style_apply_tile(panel);

    lv_obj_set_size(panel, lv_pct(100), lv_pct(100));

    lv_obj_center(panel);

    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);

    lv_obj_set_flex_flow(panel,
                         LV_FLEX_FLOW_COLUMN);

    lv_obj_set_flex_align(panel,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);


    // ========================================================
    // Title
    // ========================================================

    lv_obj_t *title = lv_label_create(panel);

    lv_label_set_text(title, "Pre Operation");

    fg_style_apply_label(title);

    lv_obj_set_style_text_font(title,
                               &lv_font_montserrat_24,
                               0);


    // ========================================================
    // Subtitle
    // ========================================================

    lv_obj_t *sub = lv_label_create(panel);

    lv_label_set_text(sub,
                      "ForgeUI PreOp module ready");

    fg_style_apply_label_dim(sub);
}
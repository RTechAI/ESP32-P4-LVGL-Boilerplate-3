// ============================================================
// ForgeUI Keyboard Overlay
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
// Shared LVGL on-screen keyboard helper.
//
// Responsibilities:
//
// - create reusable keyboard overlay
// - attach keyboard to text areas
// - manage keyboard lifecycle
// - provide shared text-entry path
//
// Current Features:
//
// - shared singleton keyboard
// - password and text entry support
// - READY/CANCEL handling
// - Reactor-compatible overlay behaviour
// - theme-aware keyboard styling
//
// Architecture Rules:
//
// - UI layer only
// - no backend ownership
// - no hardware ownership
// - no runtime truth storage
//
// Keyboard Ownership:
//
// - single shared keyboard instance
// - reused across modules and modals
// - foreground overlay only
//
// Future Direction:
//
// - theme variants
// - numeric keypad mode
// - PIN entry mode
// - animation and transitions
// - auto-layout shifting
// - appliance/product keyboard styles
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

#include "15_UI_Keyboard.h"
#include "16_UI_Style.h"
#include "lvgl.h"
#include "00_ForgeUI_Config.h"


// ============================================================
// Local State
// ============================================================

static lv_obj_t *g_kb = NULL;


// ============================================================
// Keyboard Event Handler
// ============================================================
// Handles:
// - READY (✔)
// - CANCEL (✖)

static void kb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_READY ||
        code == LV_EVENT_CANCEL)
    {
        fg_keyboard_hide();
    }
}


// ============================================================
// Attach Keyboard
// ============================================================

void fg_keyboard_attach(lv_obj_t *ta)
{
    if (!ta) {
        return;
    }

    lv_obj_t *scr = lv_screen_active();


    // ========================================================
    // Create Shared Keyboard
    // ========================================================

    if (!g_kb)
    {
        g_kb = lv_keyboard_create(scr);

        fg_style_apply_panel(g_kb);

/* Nebula keyboard polish */
#if FORGEUI_STYLE_ACTIVE == FORGEUI_STYLE_NEBULA_BLUE
    lv_obj_set_style_bg_color(g_kb, lv_color_hex(0x0B1220), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_kb, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(g_kb, lv_color_hex(0x1E3A5F), LV_PART_MAIN);
    lv_obj_set_style_border_width(g_kb, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(g_kb, 14, LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_kb, 8, LV_PART_MAIN);

    lv_obj_set_style_bg_color(g_kb, lv_color_hex(0x14213A), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(g_kb, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_text_color(g_kb, lv_color_hex(0xEAF2FF), LV_PART_ITEMS);
    lv_obj_set_style_border_color(g_kb, lv_color_hex(0x2F80ED), LV_PART_ITEMS);
    lv_obj_set_style_border_width(g_kb, 1, LV_PART_ITEMS);
    lv_obj_set_style_radius(g_kb, 8, LV_PART_ITEMS);

    lv_obj_set_style_bg_color(g_kb, lv_color_hex(0x2563EB), LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(g_kb, lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_PRESSED);

    /* Catch remaining special/function keys */
    lv_obj_set_style_bg_color(g_kb, lv_color_hex(0x14213A), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(g_kb, lv_color_hex(0xEAF2FF), LV_PART_ITEMS | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(g_kb, lv_color_hex(0x1D4ED8), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(g_kb, lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_obj_set_style_bg_color(g_kb, lv_color_hex(0x1E293B), LV_PART_ITEMS | LV_STATE_FOCUSED);
    lv_obj_set_style_text_color(g_kb, lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_FOCUSED);
#endif

        lv_obj_set_size(g_kb,
                lv_pct(100),
                      220);

        lv_obj_align(g_kb,
                     LV_ALIGN_BOTTOM_MID,
                     0,
                     0);

        // Handle ✔ and ✖ events
        lv_obj_add_event_cb(g_kb,
                            kb_event_cb,
                            LV_EVENT_ALL,
                            NULL);
    }


    // ========================================================
    // Attach To Text Area
    // ========================================================

    lv_keyboard_set_textarea(g_kb, ta);

    lv_obj_move_foreground(g_kb);

    lv_obj_clear_flag(g_kb,
                      LV_OBJ_FLAG_HIDDEN);
}


// ============================================================
// Hide Keyboard
// ============================================================

void fg_keyboard_hide(void)
{
    if (g_kb)
    {
        lv_keyboard_set_textarea(g_kb, NULL);

        lv_obj_add_flag(g_kb,
                        LV_OBJ_FLAG_HIDDEN);
    }
}
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
// - create shared modal overlay
// - create reusable modal container
// - provide consistent modal header layout
// - handle shared close behaviour
// - support modal cleanup callbacks
//
// Current Features:
//
// - singleton modal overlay
// - optional background dim
// - optional glow style
// - optional fade animation
// - async delete close path
// - close callback support
//
// Architecture Rules:
//
// - UI layer only
// - no backend ownership
// - no hardware ownership
// - no runtime truth storage
// - no feature-specific logic
//
// Ownership:
//
// Feature modules build their content inside
// the returned modal body object.
//
// Reactor modal owns only:
//
// - overlay lifecycle
// - modal frame
// - close button
// - cleanup callback
//
// Controlled Through:
//
//   00_ForgeUI_Config.h
//
// Future Direction:
//
// - zoom animation
// - glass style
// - modal size presets
// - drawer and popup variants
// - focus trapping
// - keyboard-aware layout
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
// ForgeUI Reactor Modal
// ============================================================

#include "17_UI_ReactorModal.h"

#include "00_ForgeUI_Config.h"
#include "16_UI_Style.h"

#include "lvgl.h"

static fg_reactor_modal_close_cb_t g_close_cb = NULL;
static lv_obj_t *g_modal_overlay = NULL;

void fg_reactor_modal_set_close_cb(fg_reactor_modal_close_cb_t cb)
{
    g_close_cb = cb;
}

static void evt_modal_close(lv_event_t *e)
{
    LV_UNUSED(e);

    if (g_close_cb) {
        g_close_cb();
        g_close_cb = NULL;
    }

    if (g_modal_overlay) {
        lv_obj_t *old = g_modal_overlay;
        g_modal_overlay = NULL;

        lv_obj_add_flag(old, LV_OBJ_FLAG_HIDDEN);
        lv_obj_delete_async(old);
    }
}

lv_obj_t *fg_reactor_modal_open(const char *title, const void *icon)
{
    if (g_modal_overlay) {
        lv_obj_delete_async(g_modal_overlay);
        g_modal_overlay = NULL;
    }

    lv_obj_t *scr = lv_screen_active();

#if FORGEUI_REACTOR_MODAL_DIM_BACKGROUND
    g_modal_overlay = lv_obj_create(scr);
    lv_obj_remove_style_all(g_modal_overlay);
    lv_obj_set_size(g_modal_overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(g_modal_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(g_modal_overlay, LV_OPA_50, 0);
#else
    g_modal_overlay = lv_obj_create(scr);
    lv_obj_remove_style_all(g_modal_overlay);
    lv_obj_set_size(g_modal_overlay, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(g_modal_overlay, LV_OPA_TRANSP, 0);
#endif

    lv_obj_t *box = lv_obj_create(g_modal_overlay);
    fg_style_apply_tile(box);

    lv_obj_set_size(box,
                    FORGEUI_REACTOR_MODAL_WIDTH,
                    FORGEUI_REACTOR_MODAL_HEIGHT);

    lv_obj_center(box);

    lv_obj_set_layout(box, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(box, LV_FLEX_FLOW_COLUMN);

    lv_obj_set_flex_align(box,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_pad_all(box, 22, 0);
    lv_obj_set_style_pad_row(box, 14, 0);

#if FORGEUI_REACTOR_MODAL_STYLE_ACTIVE == FORGEUI_REACTOR_MODAL_STYLE_GLOW
    lv_obj_set_style_shadow_width(box, 28, 0);
    lv_obj_set_style_shadow_opa(box, LV_OPA_40, 0);
    lv_obj_set_style_shadow_color(box, fg_style_accent(), 0);
#endif

    lv_obj_t *header = lv_obj_create(box);
    lv_obj_remove_style_all(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 58);

    lv_obj_set_layout(header, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header,
                          LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_t *left = lv_obj_create(header);
    lv_obj_remove_style_all(left);
    lv_obj_set_size(left, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_set_layout(left, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(left,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(left, 12, 0);

    if (icon) {
        lv_obj_t *img = lv_image_create(left);
        lv_image_set_src(img, icon);
    }

    lv_obj_t *lbl = lv_label_create(left);
    lv_label_set_text(lbl, title ? title : "Panel");
    fg_style_apply_label(lbl);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_24, 0);

    lv_obj_t *btn_close = lv_button_create(header);
    fg_style_apply_button(btn_close);
    lv_obj_set_size(btn_close, 44, 44);
    lv_obj_add_event_cb(btn_close, evt_modal_close, LV_EVENT_CLICKED, NULL);

    lv_obj_t *x = lv_label_create(btn_close);
    lv_label_set_text(x, "X");
    lv_obj_center(x);

#if FORGEUI_REACTOR_MODAL_ANIM_ACTIVE == FORGEUI_REACTOR_MODAL_ANIM_FADE
    lv_obj_set_style_opa(g_modal_overlay, LV_OPA_TRANSP, 0);
    lv_obj_fade_in(g_modal_overlay, 160, 0);
#endif

    return box;
}

void fg_reactor_modal_close(lv_obj_t *modal)
{
    LV_UNUSED(modal);

    if (g_close_cb) {
        g_close_cb();
        g_close_cb = NULL;
    }

    if (g_modal_overlay) {
        lv_obj_t *old = g_modal_overlay;
        g_modal_overlay = NULL;

        lv_obj_add_flag(old, LV_OBJ_FLAG_HIDDEN);
        lv_obj_delete_async(old);
    }
}
// ============================================================
// ForgeUI Status Drawer
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
// Global slide-out runtime status drawer.
//
// Responsibilities:
//
// - provide quick system visibility
// - expose lightweight runtime state
// - remain available above normal UI pages
// - provide appliance-style quick status access
//
// Current Features:
//
// - edge handle launcher
// - animated slide drawer
// - static status labels
// - config-controlled left/right side
// - top-layer overlay ownership
//
// Architecture Rules:
//
// - UI-only overlay
// - no backend ownership
// - no hardware ownership
// - lightweight render layer only
//
// Backend modules own runtime truth and state.
// Drawer only renders display state.
//
// Controlled Through:
//
//   00_ForgeUI_Config.h
//
// Current Reactor Direction:
//
// Status Drawer acts as:
//
// - persistent runtime overlay
// - quick-access status surface
// - appliance-style side panel
//
// Future Direction:
//
// - live backend bindings
// - notifications
// - alerts
// - telemetry indicators
// - mini quick-settings
// - network indicators
// - customer branding/status modules
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

#include "17_UI_StatusDrawer.h"

#include "00_ForgeUI_Config.h"
#include "16_UI_Style.h"

#include "lvgl.h"

static lv_obj_t *g_drawer = NULL;
static lv_obj_t *g_handle = NULL;
static lv_obj_t *g_handle_lbl = NULL;

static bool g_drawer_open = false;

// ===============================
// Toggle
// ===============================

void fg_status_drawer_toggle(void)
{
    if (!g_drawer) {
        return;
    }

    g_drawer_open = !g_drawer_open;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, g_drawer);
    lv_anim_set_time(&a, 180);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);

#if FORGEUI_STATUS_DRAWER_SIDE == FORGEUI_STATUS_DRAWER_RIGHT

    if (g_drawer_open)
    {
        lv_anim_set_values(&a,
                           FORGEUI_STATUS_DRAWER_WIDTH,
                           0);

        if (g_handle_lbl) {
            lv_label_set_text(g_handle_lbl, ">");
        }
    }
    else
    {
        lv_anim_set_values(&a,
                           lv_obj_get_x(g_drawer),
                           FORGEUI_STATUS_DRAWER_WIDTH);

        if (g_handle_lbl) {
            lv_label_set_text(g_handle_lbl, "<");
        }
    }

#else

    if (g_drawer_open)
    {
        lv_anim_set_values(&a,
                           -FORGEUI_STATUS_DRAWER_WIDTH,
                           0);

        if (g_handle_lbl) {
            lv_label_set_text(g_handle_lbl, "<");
        }
    }
    else
    {
        lv_anim_set_values(&a,
                           lv_obj_get_x(g_drawer),
                           -FORGEUI_STATUS_DRAWER_WIDTH);

        if (g_handle_lbl) {
            lv_label_set_text(g_handle_lbl, ">");
        }
    }

#endif

    lv_anim_start(&a);
}

// ===============================
// Handle Event
// ===============================

static void evt_drawer_toggle(lv_event_t *e)
{
    LV_UNUSED(e);
    fg_status_drawer_toggle();
}

// ===============================
// Create
// ===============================

void fg_status_drawer_create(void)
{
    lv_obj_t *parent = lv_layer_top();

    g_drawer = lv_obj_create(parent);

    lv_obj_set_size(
        g_drawer,
        FORGEUI_STATUS_DRAWER_WIDTH,
        lv_disp_get_ver_res(NULL)
            - FORGEUI_STATUS_DRAWER_Y
            - FORGEUI_STATUS_DRAWER_BOTTOM_GAP);

#if FORGEUI_STATUS_DRAWER_SIDE == FORGEUI_STATUS_DRAWER_RIGHT

    lv_obj_align(g_drawer,
                 LV_ALIGN_TOP_RIGHT,
                 0,
                 FORGEUI_STATUS_DRAWER_Y);

    lv_obj_set_x(g_drawer,
                 FORGEUI_STATUS_DRAWER_WIDTH);

#else

    lv_obj_align(g_drawer,
                 LV_ALIGN_TOP_LEFT,
                 0,
                 FORGEUI_STATUS_DRAWER_Y);

    lv_obj_set_x(g_drawer,
                 -FORGEUI_STATUS_DRAWER_WIDTH);

#endif

    fg_style_apply_panel(g_drawer);

    lv_obj_set_style_pad_all(g_drawer, 16, 0);
    lv_obj_set_style_pad_row(g_drawer, 12, 0);

    lv_obj_set_layout(g_drawer, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(g_drawer, LV_FLEX_FLOW_COLUMN);

    // ===========================
    // Labels
    // ===========================

    lv_obj_t *lbl_wifi = lv_label_create(g_drawer);
    lv_label_set_text(lbl_wifi, "WiFi: READY");
    fg_style_apply_label(lbl_wifi);

    lv_obj_t *lbl_sd = lv_label_create(g_drawer);
    lv_label_set_text(lbl_sd, "SD: READY");
    fg_style_apply_label(lbl_sd);

    lv_obj_t *lbl_rtc = lv_label_create(g_drawer);
    lv_label_set_text(lbl_rtc, "RTC: OK");
    fg_style_apply_label(lbl_rtc);

    lv_obj_t *lbl_audio = lv_label_create(g_drawer);
    lv_label_set_text(lbl_audio, "Audio: ON");
    fg_style_apply_label(lbl_audio);

    lv_obj_t *lbl_admin = lv_label_create(g_drawer);
    lv_label_set_text(lbl_admin, "Admin: DEV");
    fg_style_apply_label(lbl_admin);

    // ===========================
    // Edge Handle
    // ===========================

    g_handle = lv_button_create(parent);

    lv_obj_set_size(g_handle,
                    FORGEUI_STATUS_DRAWER_HANDLE_W,
                    FORGEUI_STATUS_DRAWER_HANDLE_H);

#if FORGEUI_STATUS_DRAWER_SIDE == FORGEUI_STATUS_DRAWER_RIGHT

    lv_obj_align(g_handle,
                 LV_ALIGN_RIGHT_MID,
                 0,
                 0);

#else

    lv_obj_align(g_handle,
                 LV_ALIGN_LEFT_MID,
                 0,
                 0);

#endif

    lv_obj_set_y(g_handle,
                 FORGEUI_STATUS_DRAWER_Y / 2);

    fg_style_apply_button(g_handle);
    fg_style_apply_button(g_handle);

lv_obj_set_style_bg_opa(g_handle, LV_OPA_70, 0);

lv_obj_set_style_bg_color(
    g_handle,
    lv_color_hex(0x1F2937),
    0);

lv_obj_set_style_border_width(g_handle, 1, 0);

lv_obj_set_style_border_color(
    g_handle,
    lv_color_hex(0x6FA8DC),
    0);

lv_obj_set_style_shadow_width(g_handle, 12, 0);

lv_obj_set_style_shadow_opa(g_handle, LV_OPA_20, 0);

lv_obj_set_style_shadow_color(
    g_handle,
    lv_color_hex(0x6FA8DC),
    0);

    lv_obj_add_event_cb(g_handle,
                        evt_drawer_toggle,
                        LV_EVENT_CLICKED,
                        NULL);

    g_handle_lbl = lv_label_create(g_handle);

#if FORGEUI_STATUS_DRAWER_SIDE == FORGEUI_STATUS_DRAWER_RIGHT
    lv_label_set_text(g_handle_lbl, "<");
#else
    lv_label_set_text(g_handle_lbl, ">");
#endif

    lv_obj_center(g_handle_lbl);
}
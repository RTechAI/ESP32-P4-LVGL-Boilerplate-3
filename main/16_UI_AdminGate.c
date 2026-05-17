// ============================================================
// ForgeUI Admin Gate
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
// Optional PIN-based Admin access layer.
//
// Responsibilities:
//
// - protect Admin UI access
// - handle PIN entry UI
// - manage temporary unlock state
// - provide future authentication entry point
//
// Current V1 Features:
//
// - compile-time enable/disable
// - masked PIN entry
// - unlock/relock support
// - keypad-style PIN UI
// - simple runtime session state
//
// Architecture Rules:
//
// - UI-only access layer
// - no backend ownership
// - no hardware ownership
// - no persistent storage yet
// - no hardware authentication yet
//
// Controlled Through:
//
//   FORGEUI_ENABLE_ADMIN_GATE
//
// Default PIN Source:
//
//   FORGEUI_ADMIN_DEFAULT_PIN
//
// Current Reactor Direction:
//
// Admin gate acts as:
//
// - protected UI boundary
// - future session entry point
// - appliance-style protected access layer
//
// Future Direction:
//
// - NVS-stored PIN
// - RFID unlock
// - role and session system
// - timeout handling
// - user database integration
// - customer/service roles
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

#include "16_UI_AdminGate.h"

#include <string.h>
#include <stdio.h>

#include "16_UI_Style.h"
#include "00_ForgeUI_Config.h"
#include "13_UI_Admin.h"

#if FORGEUI_ENABLE_ADMIN_GATE

#define FG_ADMIN_PIN_MAX_LEN 8

static bool g_admin_unlocked = false;

static lv_obj_t *g_admin_parent = NULL;
static lv_obj_t *g_pin_lbl = NULL;
static lv_obj_t *g_status_lbl = NULL;

static char g_pin[FG_ADMIN_PIN_MAX_LEN + 1] = {0};
static uint8_t g_pin_len = 0;

bool fg_admin_is_unlocked(void)
{
    return g_admin_unlocked;
}

void fg_admin_lock(void)
{
    g_admin_unlocked = false;
    g_pin_len = 0;
    g_pin[0] = '\0';
}

static void fg_admin_refresh_pin_label(void)
{
    if (!g_pin_lbl) return;

    char masked[FG_ADMIN_PIN_MAX_LEN + 1];

    for (uint8_t i = 0; i < g_pin_len; i++) {
        masked[i] = '*';
    }

    masked[g_pin_len] = '\0';

    if (g_pin_len == 0) {
        lv_label_set_text(g_pin_lbl, "----");
    } else {
        lv_label_set_text(g_pin_lbl, masked);
    }
}

static void fg_admin_clear_pin(void)
{
    g_pin_len = 0;
    g_pin[0] = '\0';
    fg_admin_refresh_pin_label();
}

static void fg_admin_unlock_success(void)
{
    g_admin_unlocked = true;

    if (g_status_lbl) {
        lv_label_set_text(g_status_lbl, "Unlocked");
    }

    if (g_admin_parent) {
        lv_obj_clean(g_admin_parent);
        ui_admin_build(g_admin_parent);
    }
}

static void fg_admin_submit_pin(void)
{
    if (strcmp(g_pin, FORGEUI_ADMIN_DEFAULT_PIN) == 0) {
        fg_admin_clear_pin();
        fg_admin_unlock_success();
        return;
    }

    if (g_status_lbl) {
        lv_label_set_text(g_status_lbl, "Wrong PIN");
    }

    fg_admin_clear_pin();
}

static void evt_digit(lv_event_t *e)
{
    const char *digit = (const char *)lv_event_get_user_data(e);

    if (!digit) return;

    if (g_pin_len >= FG_ADMIN_PIN_MAX_LEN) {
        if (g_status_lbl) {
            lv_label_set_text(g_status_lbl, "PIN too long");
        }
        return;
    }

    g_pin[g_pin_len++] = digit[0];
    g_pin[g_pin_len] = '\0';

    if (g_status_lbl) {
        lv_label_set_text(g_status_lbl, "Enter PIN");
    }

    fg_admin_refresh_pin_label();
}

static void evt_clear(lv_event_t *e)
{
    LV_UNUSED(e);

    if (g_status_lbl) {
        lv_label_set_text(g_status_lbl, "Cleared");
    }

    fg_admin_clear_pin();
}

static void evt_back(lv_event_t *e)
{
    LV_UNUSED(e);

    if (g_pin_len > 0) {
        g_pin_len--;
        g_pin[g_pin_len] = '\0';
    }

    if (g_status_lbl) {
        lv_label_set_text(g_status_lbl, "Enter PIN");
    }

    fg_admin_refresh_pin_label();
}

static void evt_enter(lv_event_t *e)
{
    LV_UNUSED(e);
    fg_admin_submit_pin();
}

static lv_obj_t *fg_admin_make_button(lv_obj_t *parent,
                                      const char *text,
                                      lv_event_cb_t cb,
                                      void *user_data)
{
    lv_obj_t *btn = lv_button_create(parent);
    fg_style_apply_button(btn);
    lv_obj_set_size(btn, 92, 54);

    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_center(lbl);

    return btn;
}

void fg_admin_gate_build(lv_obj_t *parent)
{
    g_admin_parent = parent;
    fg_admin_clear_pin();

    lv_obj_t *panel = lv_obj_create(parent);
    fg_style_apply_tile(panel);

    lv_obj_set_size(panel, 420, 520);
    lv_obj_center(panel);

    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(panel, 18, 0);
    lv_obj_set_style_pad_row(panel, 12, 0);

    lv_obj_set_flex_align(panel,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, "Admin Locked");
    fg_style_apply_label(title);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);

    lv_obj_t *sub = lv_label_create(panel);
    lv_label_set_text(sub, "Enter PIN");
    fg_style_apply_label_dim(sub);

    g_pin_lbl = lv_label_create(panel);
    lv_label_set_text(g_pin_lbl, "----");
    fg_style_apply_label(g_pin_lbl);
    lv_obj_set_style_text_font(g_pin_lbl, &lv_font_montserrat_24, 0);

    g_status_lbl = lv_label_create(panel);
    lv_label_set_text(g_status_lbl, "Enter PIN");
    fg_style_apply_label_dim(g_status_lbl);

    lv_obj_t *grid = lv_obj_create(panel);
    fg_style_apply_panel(grid);
    lv_obj_set_size(grid, 330, 300);

    lv_obj_set_layout(grid, LV_LAYOUT_GRID);

    static lv_coord_t col_dsc[] = {
        92, 92, 92, LV_GRID_TEMPLATE_LAST
    };

    static lv_coord_t row_dsc[] = {
        54, 54, 54, 54, LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(grid, 8, 0);
    lv_obj_set_style_pad_row(grid, 8, 0);
    lv_obj_set_style_pad_column(grid, 8, 0);

    lv_obj_t *b1 = fg_admin_make_button(grid, "1", evt_digit, "1");
    lv_obj_set_grid_cell(b1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *b2 = fg_admin_make_button(grid, "2", evt_digit, "2");
    lv_obj_set_grid_cell(b2, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *b3 = fg_admin_make_button(grid, "3", evt_digit, "3");
    lv_obj_set_grid_cell(b3, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *b4 = fg_admin_make_button(grid, "4", evt_digit, "4");
    lv_obj_set_grid_cell(b4, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *b5 = fg_admin_make_button(grid, "5", evt_digit, "5");
    lv_obj_set_grid_cell(b5, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *b6 = fg_admin_make_button(grid, "6", evt_digit, "6");
    lv_obj_set_grid_cell(b6, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *b7 = fg_admin_make_button(grid, "7", evt_digit, "7");
    lv_obj_set_grid_cell(b7, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);

    lv_obj_t *b8 = fg_admin_make_button(grid, "8", evt_digit, "8");
    lv_obj_set_grid_cell(b8, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 2, 1);

    lv_obj_t *b9 = fg_admin_make_button(grid, "9", evt_digit, "9");
    lv_obj_set_grid_cell(b9, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 2, 1);

    lv_obj_t *bc = fg_admin_make_button(grid, "Clear", evt_clear, NULL);
    lv_obj_set_grid_cell(bc, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 3, 1);

    lv_obj_t *b0 = fg_admin_make_button(grid, "0", evt_digit, "0");
    lv_obj_set_grid_cell(b0, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 3, 1);

    lv_obj_t *bb = fg_admin_make_button(grid, "Back", evt_back, NULL);
    lv_obj_set_grid_cell(bb, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 3, 1);

    lv_obj_t *enter = lv_button_create(panel);
    fg_style_apply_button(enter);
    lv_obj_set_size(enter, 330, 58);
    lv_obj_add_event_cb(enter, evt_enter, LV_EVENT_CLICKED, NULL);

    lv_obj_t *enter_lbl = lv_label_create(enter);
    lv_label_set_text(enter_lbl, "Enter");
    lv_obj_center(enter_lbl);
}

#else

bool fg_admin_is_unlocked(void)
{
    return true;
}

void fg_admin_lock(void)
{
}

void fg_admin_gate_build(lv_obj_t *parent)
{
    LV_UNUSED(parent);
}

#endif
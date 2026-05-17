// ============================================================
// ForgeUI HMI Core
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
// Main LVGL HMI construction and navigation layer.
//
// Responsibilities:
//
// - create root tabview
// - apply global visual styles
// - create shared header overlay
// - create top-level UI pages
// - dispatch page builders
// - provide centralized navigation helpers
//
// Architecture Rules:
//
// - no backend ownership
// - no hardware ownership
// - no persistent runtime truth
// - no subsystem state storage
//
// Backend modules own runtime truth.
// UI layers only render state and send intent.
//
// Current UI Pages:
//
// - Dashboard
// - Pre Op
// - System
// - Admin
//
// Reactor UI Direction:
//
// Reactor uses hidden internal LVGL tab routing.
//
// Primary navigation:
// - launcher cards
// - icon taps
//
// Secondary navigation:
// - swipe gestures
//
// Target Hardware:
// - Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
//
// Core Stack:
// - ESP-IDF
// - LVGL v9
// - ESP-Hosted
// - ESP32-C6 WiFi Remote
//
// ============================================================

// ============================================================
// Includes
// ============================================================

#include "lvgl.h"

#include "01_FG_HMI.h"

#include "10_UI_Dashboard.h"
#include "11_UI_PreOp.h"
#include "12_UI_System.h"
#include "13_UI_Admin.h"

#include "14_UI_Header.h"
#include "16_UI_Style.h"
#include "05_FG_Icons.h"
#include "00_ForgeUI_Config.h"


// ============================================================
// Local State
// ============================================================

static lv_obj_t *g_tabview = NULL;
static lv_obj_t *g_home_btn = NULL;

#define FG_TAB_DASHBOARD  0
#define FG_TAB_PREOP      1
#define FG_TAB_SYSTEM     2
#define FG_TAB_ADMIN      3


// ============================================================
// Swipe Tuning
// ============================================================

static void fg_hmi_swipe_event_cb(lv_event_t *e);

static void fg_hmi_tune_tab_swipe(lv_obj_t *tabview)
{
    if (!tabview) return;

    lv_obj_t *content = lv_tabview_get_content(tabview);
    if (!content) return;

    lv_obj_set_scroll_dir(content, LV_DIR_HOR);
    lv_obj_set_scroll_snap_x(content, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(content, LV_SCROLLBAR_MODE_OFF);

    lv_obj_remove_flag(content, LV_OBJ_FLAG_SCROLL_ONE);
    lv_obj_add_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(content,
                        fg_hmi_swipe_event_cb,
                        LV_EVENT_ALL,
                        NULL);
}

static lv_point_t g_swipe_start;
static bool g_swipe_tracking = false;

#define FG_SWIPE_MIN_X  45
#define FG_SWIPE_MAX_Y  80

static void fg_hmi_swipe_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (!g_tabview) return;

    if (code == LV_EVENT_PRESSED) {
        lv_indev_t *indev = lv_indev_get_act();
        if (!indev) return;

        lv_indev_get_point(indev, &g_swipe_start);
        g_swipe_tracking = true;
        return;
    }

    if (code == LV_EVENT_RELEASED && g_swipe_tracking) {
        lv_point_t end;
        lv_indev_t *indev = lv_indev_get_act();

        g_swipe_tracking = false;

        if (!indev) return;

        lv_indev_get_point(indev, &end);

        int dx = end.x - g_swipe_start.x;
        int dy = end.y - g_swipe_start.y;

        if (LV_ABS(dx) < FG_SWIPE_MIN_X) return;
        if (LV_ABS(dy) > FG_SWIPE_MAX_Y) return;

        uint32_t active = lv_tabview_get_tab_active(g_tabview);

        if (dx < 0 && active < FG_TAB_ADMIN) {
            lv_tabview_set_active(g_tabview, active + 1, LV_ANIM_ON);
        }
        else if (dx > 0 && active > FG_TAB_DASHBOARD) {
            lv_tabview_set_active(g_tabview, active - 1, LV_ANIM_ON);
        }
    }

    LV_UNUSED(obj);
}

static void fg_hmi_home_btn_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    fg_hmi_go_dashboard();
}

static void fg_hmi_create_home_button(lv_obj_t *scr)
{
#if FORGEUI_ENABLE_HOME_BUTTON

    if (!scr) return;

    g_home_btn = lv_button_create(scr);

    fg_style_apply_tile(g_home_btn);

    lv_obj_set_size(g_home_btn,
                    FORGEUI_HOME_BUTTON_SIZE,
                    FORGEUI_HOME_BUTTON_SIZE);

    lv_obj_align(g_home_btn,
                 LV_ALIGN_BOTTOM_LEFT,
                 FORGEUI_HOME_BUTTON_MARGIN_X,
                 -FORGEUI_HOME_BUTTON_MARGIN_Y);

    lv_obj_add_event_cb(g_home_btn,
                        fg_hmi_home_btn_event_cb,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_t *img = lv_image_create(g_home_btn);

    lv_image_set_src(img, fg_icon_home());

    lv_obj_center(img);

    lv_obj_move_foreground(g_home_btn);

#endif
}

// ============================================================
// ForgeUI HMI Init
// ============================================================

void fg_hmi_init(void)
{
    // ========================================================
    // Root Screen
    // ========================================================

    lv_obj_t *scr = lv_screen_active();

    fg_style_apply_screen(scr);


    // ========================================================
    // Main TabView
    // ========================================================

    g_tabview = lv_tabview_create(scr);
    lv_obj_t *tabview = g_tabview;

    fg_style_apply_screen(tabview);

    lv_tabview_set_tab_bar_position(tabview, LV_DIR_TOP);

    fg_hmi_tune_tab_swipe(tabview);


    // ========================================================
// Top Header Bar
// ========================================================

lv_obj_t *tab_bar = lv_tabview_get_tab_bar(tabview);

#if !FORGEUI_ENABLE_HEADER

    lv_obj_add_flag(tab_bar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_height(tab_bar, 0);

#elif FORGEUI_STYLE_ACTIVE == FORGEUI_STYLE_REACTOR

    lv_obj_add_flag(tab_bar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_height(tab_bar, 0);

#else

    fg_style_apply_panel(tab_bar);

    lv_obj_set_height(tab_bar, 60);

    lv_obj_set_style_pad_top(tab_bar,    8, 0);
    lv_obj_set_style_pad_bottom(tab_bar, 8, 0);

    lv_obj_set_style_pad_left(tab_bar,  10, 0);
    lv_obj_set_style_pad_right(tab_bar, 10, 0);

    lv_obj_set_style_text_font(tab_bar,
                               &lv_font_montserrat_22,
                               0);

    lv_obj_set_style_text_color(tab_bar,
                                fg_style_text(),
                                0);

#endif


    // ========================================================
    // Header Overlay
    // ========================================================

#if FORGEUI_ENABLE_HEADER
    fg_header_create(scr);
#endif


    // ========================================================
    // Tabs
    // ========================================================

    lv_obj_t *tab_dashboard =
        lv_tabview_add_tab(tabview, "Dashboard");

    lv_obj_t *tab_preop =
        lv_tabview_add_tab(tabview, "Pre Op");

    lv_obj_t *tab_system =
        lv_tabview_add_tab(tabview, "System");

    lv_obj_t *tab_admin =
        lv_tabview_add_tab(tabview, "Admin");


    // ========================================================
    // Tab Builders
    // ========================================================

    ui_dashboard_build(tab_dashboard);

    ui_preop_build(tab_preop);

    ui_system_build(tab_system);

    ui_admin_build(tab_admin);

    fg_hmi_create_home_button(scr);
}


// ============================================================
// HMI Navigation Helpers
// ============================================================

void fg_hmi_go_dashboard(void)
{
    if (g_tabview) {
        lv_tabview_set_active(g_tabview,
                              FG_TAB_DASHBOARD,
                              LV_ANIM_OFF);
    }
}

void fg_hmi_go_preop(void)
{
    if (g_tabview) {
        lv_tabview_set_active(g_tabview,
                              FG_TAB_PREOP,
                              LV_ANIM_OFF);
    }
}

void fg_hmi_go_system(void)
{
    if (g_tabview) {
        lv_tabview_set_active(g_tabview,
                              FG_TAB_SYSTEM,
                              LV_ANIM_OFF);
    }
}

void fg_hmi_go_admin(void)
{
    if (g_tabview) {
        lv_tabview_set_active(g_tabview,
                              FG_TAB_ADMIN,
                              LV_ANIM_OFF);
    }
}

// ============================================================
// ForgeUI System UI
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
// Main runtime system and settings UI layer.
//
// Responsibilities:
//
// - display runtime system status
// - expose user and system controls
// - host shared Reactor launcher cards
// - launch shared Reactor modals
// - provide hardware/system validation controls
//
// Current Features:
//
// - display brightness
// - audio test and volume control
// - hosted WiFi manager
// - SD card management
// - RTC date/time control
// - Reactor modal launch system
//
// Architecture Rules:
//
// - UI layer only
// - no low-level hardware initialization
// - no backend ownership
// - no persistent runtime truth
//
// Backend modules own runtime truth and state.
// This layer renders state and sends intent only.
//
// Feature Ownership:
//
// Controlled through:
//
//   00_ForgeUI_Config.h
//
// Current Optional Modules:
//
// - FORGEUI_ENABLE_RTC
// - FORGEUI_ENABLE_AUDIO
// - FORGEUI_ENABLE_WIFI
// - FORGEUI_ENABLE_SD
//
// Reactor Direction:
//
// System page is evolving into:
//
// - modular launcher hub
// - appliance-style settings UI
// - shared modal launch system
//
// Future Direction:
//
// - runtime themes
// - firmware update tiles
// - diagnostics
// - telemetry panels
// - admin settings
// - user/session management
// - customer/product modules
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

#include "12_UI_System.h"
#include "00_ForgeUI_Config.h"
#include "16_UI_Style.h"
#include "17_UI_ReactorModal.h"

#include "lvgl.h"
#include "esp_log.h"
#include "esp_err.h"
#include "bsp/display.h"
#include "05_FG_Icons.h"
#include "01_FG_HMI.h"

#include <stdio.h>
#include <string.h>

#if FORGEUI_ENABLE_RTC
#include "20_RTC.h"
#endif

#if FORGEUI_ENABLE_AUDIO
#include "30_Audio.h"
#endif

#if FORGEUI_ENABLE_WIFI
#include "30_WIFI.h"
#include "15_UI_Keyboard.h"
#endif

#if FORGEUI_ENABLE_SD
#include "40_SD.h"
#include "nvs.h"
#include "esp_system.h"
#endif

static const char *TAG = "FG_SYSTEM";

// ===== State =====
static lv_obj_t *g_sys_brightness_value = NULL;

#if FORGEUI_ENABLE_AUDIO
static lv_obj_t *g_sys_sound_volume_value = NULL;
static int g_sound_volume = 70;
#endif

#if FORGEUI_ENABLE_RTC
static lv_obj_t *g_sys_rtc_status = NULL;

static lv_obj_t *g_lbl_year  = NULL;
static lv_obj_t *g_lbl_month = NULL;
static lv_obj_t *g_lbl_day   = NULL;
static lv_obj_t *g_lbl_hour  = NULL;
static lv_obj_t *g_lbl_min   = NULL;
static lv_obj_t *g_lbl_sec   = NULL;

static int g_year  = 2026;
static int g_month = 4;
static int g_day   = 17;
static int g_hour  = 12;
static int g_min   = 0;
static int g_sec   = 0;
#endif

#if FORGEUI_ENABLE_WIFI
static lv_obj_t *g_wifi_status_lbl = NULL;
static lv_obj_t *g_wifi_ip_lbl     = NULL;
static lv_obj_t *g_wifi_dropdown   = NULL;
static lv_obj_t *g_wifi_pass_ta    = NULL;
static lv_timer_t *g_wifi_timer    = NULL;
#endif

#if FORGEUI_ENABLE_SD
static lv_obj_t *g_sd_status_lbl    = NULL;
static lv_obj_t *g_sd_action_lbl    = NULL;
static lv_timer_t *g_sd_timer       = NULL;
static lv_obj_t *g_sd_size_lbl      = NULL;
static bool g_sd_safe_armed         = false;
static lv_obj_t *g_sd_safe_btn_lbl  = NULL;
static lv_obj_t *g_sd_list_lbl      = NULL;
static lv_obj_t *g_sd_reset_btn_lbl = NULL;
static bool g_sd_reset_armed        = false;
#endif


// ===== Helpers =====
static lv_obj_t *fg_make_tile(lv_obj_t *parent, const char *title, int32_t w_pct, int32_t h)
{
    lv_obj_t *tile = lv_obj_create(parent);
    lv_obj_set_size(tile, lv_pct(w_pct), h);

    fg_style_apply_tile(tile);

    lv_obj_set_layout(tile, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *lbl = lv_label_create(tile);
    lv_label_set_text(lbl, title);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lbl, fg_style_text(), 0);

    return tile;
}

static void fg_set_value_label(lv_obj_t *lbl, int value, bool pad2)
{
    char buf[16];
    if (pad2) snprintf(buf, sizeof(buf), "%02d", value);
    else      snprintf(buf, sizeof(buf), "%d", value);
    lv_label_set_text(lbl, buf);
}

#if FORGEUI_ENABLE_RTC
static void fg_refresh_rtc_labels(void)
{
    if (g_lbl_year)  fg_set_value_label(g_lbl_year,  g_year,  false);
    if (g_lbl_month) fg_set_value_label(g_lbl_month, g_month, true);
    if (g_lbl_day)   fg_set_value_label(g_lbl_day,   g_day,   true);
    if (g_lbl_hour)  fg_set_value_label(g_lbl_hour,  g_hour,  true);
    if (g_lbl_min)   fg_set_value_label(g_lbl_min,   g_min,   true);
    if (g_lbl_sec)   fg_set_value_label(g_lbl_sec,   g_sec,   true);
}
#endif



static lv_obj_t *fg_make_step_row(lv_obj_t *parent, const char *name, lv_obj_t **out_value_lbl)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, 24);
    lv_obj_set_style_pad_all(row, 2, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);

    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row,
        LV_FLEX_ALIGN_SPACE_BETWEEN,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    lv_obj_t *name_lbl = lv_label_create(row);
lv_label_set_text(name_lbl, name);
lv_obj_set_width(name_lbl, 90);
lv_obj_set_style_text_font(name_lbl, &lv_font_montserrat_16, 0);
lv_obj_set_style_text_color(name_lbl, fg_style_text(), 0);

lv_obj_t *btn_minus = lv_button_create(row);
fg_style_apply_button(btn_minus);
lv_obj_set_size(btn_minus, 34, 28);

lv_obj_t *lbl_minus = lv_label_create(btn_minus);
lv_label_set_text(lbl_minus, "-");
lv_obj_center(lbl_minus);

lv_obj_t *value_lbl = lv_label_create(row);
lv_label_set_text(value_lbl, "--");
lv_obj_set_width(value_lbl, 80);
lv_obj_set_style_text_align(value_lbl, LV_TEXT_ALIGN_CENTER, 0);
lv_obj_set_style_text_font(value_lbl, &lv_font_montserrat_16, 0);
lv_obj_set_style_text_color(value_lbl, fg_style_text(), 0);

lv_obj_t *btn_plus = lv_button_create(row);
fg_style_apply_button(btn_plus);
lv_obj_set_size(btn_plus, 34, 28);

lv_obj_t *lbl_plus = lv_label_create(btn_plus);
lv_label_set_text(lbl_plus, "+");
lv_obj_center(lbl_plus);

    *out_value_lbl = value_lbl;
    return row;
}

static lv_obj_t *fg_find_minus_btn(lv_obj_t *row)
{
    return lv_obj_get_child(row, 1);
}

static lv_obj_t *fg_find_plus_btn(lv_obj_t *row)
{
    return lv_obj_get_child(row, 3);
}

// ===== WiFi UI =====
#if FORGEUI_ENABLE_WIFI
static void fg_wifi_ui_refresh(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if (!g_wifi_status_lbl ||
        !g_wifi_ip_lbl)
    {
        return;
    }

    if (!lv_obj_is_valid(g_wifi_status_lbl) ||
        !lv_obj_is_valid(g_wifi_ip_lbl))
    {
        return;
    }

    lv_label_set_text_fmt(g_wifi_status_lbl,
                          "Status: %s",
                          fg_wifi_status_text());

    lv_label_set_text_fmt(g_wifi_ip_lbl,
                          "IP: %s",
                          fg_wifi_ip_text());

    if (g_wifi_dropdown &&
        lv_obj_is_valid(g_wifi_dropdown) &&
        strcmp(fg_wifi_status_text(), "SCAN_DONE") == 0)
    {
        char ssids[12][33];
        char opts[512] = {0};

        int n = fg_wifi_get_scan_results(ssids, 12);

        for (int i = 0; i < n; i++) {
            if (ssids[i][0] == 0) continue;

            if (opts[0] != 0) {
                strcat(opts, "\n");
            }

            strcat(opts, ssids[i]);
        }

        if (opts[0]) {
            lv_dropdown_set_options(g_wifi_dropdown, opts);
        }
    }
}

static void evt_wifi_scan(lv_event_t *e)
{
    LV_UNUSED(e);
    ESP_LOGI(TAG, "WiFi UI scan pressed");
    fg_wifi_scan_start();
}

static void evt_wifi_connect(lv_event_t *e)
{
    LV_UNUSED(e);

    if (!g_wifi_dropdown) {
        ESP_LOGW(TAG, "WiFi connect pressed but dropdown missing");
        return;
    }

    char ssid[64] = {0};
    lv_dropdown_get_selected_str(g_wifi_dropdown, ssid, sizeof(ssid));

    const char *pass = "";
    if (g_wifi_pass_ta) {
        pass = lv_textarea_get_text(g_wifi_pass_ta);
    }

    ESP_LOGI(TAG, "WiFi UI connect pressed: %s", ssid);
    fg_wifi_connect(ssid, pass);
}

static void evt_wifi_disconnect(lv_event_t *e)
{
    LV_UNUSED(e);
    ESP_LOGI(TAG, "WiFi UI disconnect pressed");
    fg_wifi_disconnect();
}

static void evt_wifi_pass_focus(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    fg_keyboard_attach(ta);
}

static void evt_wifi_pass_ready(lv_event_t *e)
{
    LV_UNUSED(e);
    fg_keyboard_hide();
}

static void evt_wifi_forget(lv_event_t *e)
{
    LV_UNUSED(e);

    ESP_LOGI(TAG, "WiFi forget pressed");

    fg_wifi_forget();

    if (g_wifi_pass_ta) {
        lv_textarea_set_text(g_wifi_pass_ta, "");
    }

    if (g_wifi_status_lbl) {
        lv_label_set_text(g_wifi_status_lbl, "Status: FORGOTTEN");
    }

    if (g_wifi_ip_lbl) {
        lv_label_set_text(g_wifi_ip_lbl, "IP: -");
    }
}
#endif

// ===== SD UI =====
#if FORGEUI_ENABLE_SD
static void fg_sd_ui_refresh(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if (!g_sd_status_lbl ||
        !g_sd_action_lbl ||
        !g_sd_size_lbl)
    {
        return;
    }

    if (!lv_obj_is_valid(g_sd_status_lbl) ||
        !lv_obj_is_valid(g_sd_action_lbl) ||
        !lv_obj_is_valid(g_sd_size_lbl))
    {
        return;
    }

    lv_label_set_text_fmt(g_sd_status_lbl,
                          "Status: %s",
                          fg_sd_status_text());

    lv_label_set_text(g_sd_size_lbl,
                      fg_sd_size_text_get());

    if (!g_sd_safe_armed &&
        !g_sd_reset_armed)
    {
        lv_label_set_text_fmt(g_sd_action_lbl,
                              "Last: %s",
                              fg_sd_last_action_text());
    }
}

static void evt_sd_test(lv_event_t *e)
{
    LV_UNUSED(e);

    ESP_LOGI(TAG, "SD test pressed");

    bool ok = fg_sd_test();

    if (g_sd_action_lbl) {
        lv_label_set_text(g_sd_action_lbl,
            ok ? "SD TEST PASS\nWrite/read check OK"
               : "SD TEST FAILED\nCheck SD card");
    }
}

static void evt_sd_list(lv_event_t *e)
{
    LV_UNUSED(e);

    char buf[512] = {0};
    bool ok = fg_sd_list_forgeui(buf, sizeof(buf));

    if (g_sd_action_lbl) {
        lv_label_set_text(g_sd_action_lbl,
                          ok ? "Last: Folder list loaded"
                             : "Last: Folder list failed/missing");
    }

    if (g_sd_list_lbl) {
        lv_label_set_text(g_sd_list_lbl, buf);
    }
}

static void evt_sd_prepare_card(lv_event_t *e)
{
    LV_UNUSED(e);

    if (!g_sd_safe_armed)
    {
        g_sd_safe_armed = true;

        if (g_sd_safe_btn_lbl) {
            lv_label_set_text(g_sd_safe_btn_lbl, "CONFIRM SAFE RESET");
        }

        if (g_sd_action_lbl) {
            lv_label_set_text(g_sd_action_lbl,
                "SAFE RESET FORGEUI\n"
                "Deletes all ForgeUI files\n"
                "Rebuilds clean folders\n"
                "Restarts in safe mode\n"
                "Press again to confirm");
        }

        return;
    }

    g_sd_safe_armed = false;

    if (g_sd_safe_btn_lbl) {
        lv_label_set_text(g_sd_safe_btn_lbl, "Safe Reset (Restart) ⚠");
    }

    if (g_sd_action_lbl) {
        lv_label_set_text(g_sd_action_lbl,
            "Restarting...\n"
            "Safe reset will run before UI/WiFi");
    }

    nvs_handle_t nvs;
    if (nvs_open("forgeui", NVS_READWRITE, &nvs) == ESP_OK)
    {
        nvs_set_u8(nvs, "sd_prep", 1);
        nvs_commit(nvs);
        nvs_close(nvs);
    }

    vTaskDelay(pdMS_TO_TICKS(300));
    esp_restart();
}

static void evt_sd_reset_storage(lv_event_t *e)
{
    LV_UNUSED(e);

    if (!g_sd_reset_armed)
    {
        g_sd_reset_armed = true;

        if (g_sd_reset_btn_lbl) {
            lv_label_set_text(g_sd_reset_btn_lbl, "CONFIRM LIVE REBUILD");
        }

        if (g_sd_action_lbl) {
            lv_label_set_text(g_sd_action_lbl,
                "LIVE REBUILD FORGEUI\n"
                "Deletes all ForgeUI files\n"
                "Rebuilds clean folders now\n"
                "No restart\n"
                "Press again to confirm");
        }

        ESP_LOGW(TAG, "Live ForgeUI rebuild armed");
        return;
    }

    g_sd_reset_armed = false;

    if (g_sd_reset_btn_lbl) {
        lv_label_set_text(g_sd_reset_btn_lbl, "Live Rebuild ForgeUI");
    }

    if (g_sd_action_lbl) {
        lv_label_set_text(g_sd_action_lbl,
            "Rebuilding ForgeUI now...\n"
            "Do NOT power off.");
    }

    ESP_LOGW(TAG, "Live ForgeUI rebuild confirmed");

    fg_sd_reset_async();
}
#endif

// ===== Events =====
static void evt_brightness_changed(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int value = (int)lv_slider_get_value(slider);

    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", value);
    if (g_sys_brightness_value) lv_label_set_text(g_sys_brightness_value, buf);

    bsp_display_brightness_set(value);
    ESP_LOGI(TAG, "Brightness set: %d%%", value);
}

#if FORGEUI_ENABLE_AUDIO
static void evt_sound_volume_changed(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    g_sound_volume = (int)lv_slider_get_value(slider);

    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", g_sound_volume);

    if (g_sys_sound_volume_value) {
        lv_label_set_text(g_sys_sound_volume_value, buf);
    }

    fg_audio_set_volume(g_sound_volume);
    ESP_LOGI(TAG, "Sound volume set: %d%%", g_sound_volume);
}

static void evt_sound_test(lv_event_t *e)
{
    LV_UNUSED(e);
    ESP_LOGI(TAG, "Sound test pressed");

    fg_audio_set_volume(g_sound_volume);

    esp_err_t err = fg_audio_test_beep();

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Sound test OK");
    } else {
        ESP_LOGE(TAG, "Sound test FAILED: %s", esp_err_to_name(err));
    }
}
#endif

#if FORGEUI_ENABLE_RTC
static void evt_year_minus(lv_event_t *e)  { LV_UNUSED(e); if (g_year  > 2024) g_year--;  fg_refresh_rtc_labels(); }
static void evt_year_plus(lv_event_t *e)   { LV_UNUSED(e); if (g_year  < 2099) g_year++;  fg_refresh_rtc_labels(); }

static void evt_month_minus(lv_event_t *e) { LV_UNUSED(e); if (g_month > 1)    g_month--; else g_month = 12; fg_refresh_rtc_labels(); }
static void evt_month_plus(lv_event_t *e)  { LV_UNUSED(e); if (g_month < 12)   g_month++; else g_month = 1;  fg_refresh_rtc_labels(); }

static void evt_day_minus(lv_event_t *e)   { LV_UNUSED(e); if (g_day   > 1)    g_day--;   else g_day = 31;   fg_refresh_rtc_labels(); }
static void evt_day_plus(lv_event_t *e)    { LV_UNUSED(e); if (g_day   < 31)   g_day++;   else g_day = 1;    fg_refresh_rtc_labels(); }

static void evt_hour_minus(lv_event_t *e)  { LV_UNUSED(e); if (g_hour  > 0)    g_hour--;  else g_hour = 23;  fg_refresh_rtc_labels(); }
static void evt_hour_plus(lv_event_t *e)   { LV_UNUSED(e); if (g_hour  < 23)   g_hour++;  else g_hour = 0;   fg_refresh_rtc_labels(); }

static void evt_min_minus(lv_event_t *e)   { LV_UNUSED(e); if (g_min   > 0)    g_min--;   else g_min = 59;   fg_refresh_rtc_labels(); }
static void evt_min_plus(lv_event_t *e)    { LV_UNUSED(e); if (g_min   < 59)   g_min++;   else g_min = 0;    fg_refresh_rtc_labels(); }

static void evt_sec_minus(lv_event_t *e)   { LV_UNUSED(e); if (g_sec   > 0)    g_sec--;   else g_sec = 59;   fg_refresh_rtc_labels(); }
static void evt_sec_plus(lv_event_t *e)    { LV_UNUSED(e); if (g_sec   < 59)   g_sec++;   else g_sec = 0;    fg_refresh_rtc_labels(); }

static void evt_apply_rtc(lv_event_t *e)
{
    LV_UNUSED(e);

    fg_rtc_set(g_year, g_month, g_day, g_hour, g_min);

    char buf[128];
    snprintf(buf, sizeof(buf),
             "RTC set:\n%04d-%02d-%02d %02d:%02d",
             g_year, g_month, g_day, g_hour, g_min);

    if (g_sys_rtc_status) lv_label_set_text(g_sys_rtc_status, buf);

    ESP_LOGI(TAG, "RTC SET APPLIED: %04d-%02d-%02d %02d:%02d",
             g_year, g_month, g_day, g_hour, g_min);
}
#endif

// ===== Build =====
static void ui_system_legacy_build(lv_obj_t *parent)
{
    fg_style_apply_screen(parent);

    lv_obj_set_style_pad_all(parent, 16, 0);
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(parent, 16, 0);
    lv_obj_set_style_pad_column(parent, 16, 0);

    // ===== Tile: Brightness =====
    lv_obj_t *tile_brightness = fg_make_tile(parent, "Display Brightness", 48, 180);

    g_sys_brightness_value = lv_label_create(tile_brightness);
    lv_label_set_text(g_sys_brightness_value, "50%");
    fg_style_apply_label(g_sys_brightness_value);

    lv_obj_t *brightness_slider = lv_slider_create(tile_brightness);
    lv_obj_set_width(brightness_slider, lv_pct(100));
    lv_slider_set_range(brightness_slider, 10, 100);
    lv_slider_set_value(brightness_slider, 50, LV_ANIM_OFF);
    bsp_display_brightness_set(50);
    lv_obj_add_event_cb(brightness_slider, evt_brightness_changed, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *brightness_hint = lv_label_create(tile_brightness);
    lv_label_set_text(brightness_hint, "Live screen brightness control");
    fg_style_apply_label_dim(brightness_hint);

    #if FORGEUI_ENABLE_AUDIO
    // ===== Tile: Sound =====
    lv_obj_t *tile_sound = fg_make_tile(parent, "Sound", 48, 180);

    lv_obj_t *sound_info = lv_label_create(tile_sound);
    lv_label_set_text(sound_info, "Audio OK");
    fg_style_apply_label(sound_info);

    g_sys_sound_volume_value = lv_label_create(tile_sound);
    lv_label_set_text(g_sys_sound_volume_value, "70%");
    fg_style_apply_label(g_sys_sound_volume_value);

    lv_obj_t *sound_slider = lv_slider_create(tile_sound);
    lv_obj_set_width(sound_slider, lv_pct(100));
    lv_slider_set_range(sound_slider, 0, 100);
    lv_slider_set_value(sound_slider, g_sound_volume, LV_ANIM_OFF);
    lv_obj_add_event_cb(sound_slider, evt_sound_volume_changed, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *btn_test = lv_button_create(tile_sound);
    fg_style_apply_button(btn_test);
    lv_obj_add_event_cb(btn_test, evt_sound_test, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_test_lbl = lv_label_create(btn_test);
    lv_label_set_text(btn_test_lbl, "Speaker Test");
    lv_obj_center(btn_test_lbl);
#endif

    // ===== Tile: WiFi =====
    #if FORGEUI_ENABLE_WIFI
    lv_obj_t *tile_wifi = fg_make_tile(parent, "WiFi", 48, 380);

    g_wifi_status_lbl = lv_label_create(tile_wifi);
    lv_label_set_text_fmt(g_wifi_status_lbl, "Status: %s", fg_wifi_status_text());
    fg_style_apply_label(g_wifi_status_lbl);

    g_wifi_ip_lbl = lv_label_create(tile_wifi);
    lv_label_set_text_fmt(g_wifi_ip_lbl, "IP: %s", fg_wifi_ip_text());
    fg_style_apply_label(g_wifi_ip_lbl);

    lv_obj_t *btn_scan = lv_button_create(tile_wifi);
    fg_style_apply_button(btn_scan);
    lv_obj_add_event_cb(btn_scan, evt_wifi_scan, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_scan_lbl = lv_label_create(btn_scan);
    lv_label_set_text(btn_scan_lbl, "Scan WiFi");
    lv_obj_center(btn_scan_lbl);

    g_wifi_dropdown = lv_dropdown_create(tile_wifi);
    fg_style_apply_dropdown(g_wifi_dropdown);
    lv_obj_set_width(g_wifi_dropdown, lv_pct(100));
    lv_dropdown_set_options(g_wifi_dropdown, "No scan yet");

    g_wifi_pass_ta = lv_textarea_create(tile_wifi);
    fg_style_apply_textarea(g_wifi_pass_ta);
    lv_obj_set_width(g_wifi_pass_ta, lv_pct(100));
    lv_textarea_set_one_line(g_wifi_pass_ta, true);
    lv_textarea_set_password_mode(g_wifi_pass_ta, true);
    lv_textarea_set_placeholder_text(g_wifi_pass_ta, "Password");

    lv_obj_add_event_cb(g_wifi_pass_ta, evt_wifi_pass_focus, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(g_wifi_pass_ta, evt_wifi_pass_ready, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(g_wifi_pass_ta, evt_wifi_pass_ready, LV_EVENT_CANCEL, NULL);

    lv_obj_t *btn_connect = lv_button_create(tile_wifi);
    fg_style_apply_button(btn_connect);
    lv_obj_add_event_cb(btn_connect, evt_wifi_connect, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_connect_lbl = lv_label_create(btn_connect);
    lv_label_set_text(btn_connect_lbl, "Connect");
    lv_obj_center(btn_connect_lbl);

    lv_obj_t *wifi_btn_row = lv_obj_create(tile_wifi);
lv_obj_remove_style_all(wifi_btn_row);

lv_obj_set_width(wifi_btn_row, lv_pct(100));
lv_obj_set_height(wifi_btn_row, LV_SIZE_CONTENT);

lv_obj_set_layout(wifi_btn_row, LV_LAYOUT_FLEX);
lv_obj_set_flex_flow(wifi_btn_row, LV_FLEX_FLOW_ROW);

lv_obj_set_flex_align(
    wifi_btn_row,
    LV_FLEX_ALIGN_SPACE_BETWEEN,
    LV_FLEX_ALIGN_CENTER,
    LV_FLEX_ALIGN_CENTER
);

    lv_obj_t *btn_disconnect = lv_button_create(wifi_btn_row);
    fg_style_apply_button(btn_disconnect);
    lv_obj_set_width(btn_disconnect, lv_pct(48));
    lv_obj_add_event_cb(btn_disconnect, evt_wifi_disconnect, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_disconnect_lbl = lv_label_create(btn_disconnect);
    lv_label_set_text(btn_disconnect_lbl, "Disconnect");
    lv_obj_center(btn_disconnect_lbl);

    lv_obj_t *btn_forget = lv_button_create(wifi_btn_row);
    fg_style_apply_button(btn_forget);
    lv_obj_set_width(btn_forget, lv_pct(48));
    lv_obj_add_event_cb(btn_forget, evt_wifi_forget, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_forget_lbl = lv_label_create(btn_forget);
    lv_label_set_text(btn_forget_lbl, "Forget WiFi");
    lv_obj_center(btn_forget_lbl);

    if (!g_wifi_timer) {
        g_wifi_timer = lv_timer_create(fg_wifi_ui_refresh, 1000, NULL);
    }
#endif

#if FORGEUI_ENABLE_SD
        // ===== Tile: SD Card =====
    lv_obj_t *tile_sd = fg_make_tile(parent, "SD Card", 48, 380);

    g_sd_status_lbl = lv_label_create(tile_sd);
    lv_label_set_text_fmt(g_sd_status_lbl, "Status: %s", fg_sd_status_text());
    fg_style_apply_label(g_sd_status_lbl);

    g_sd_size_lbl = lv_label_create(tile_sd);
    lv_label_set_text(g_sd_size_lbl, fg_sd_size_text_get());
    fg_style_apply_label(g_sd_size_lbl);

    g_sd_action_lbl = lv_label_create(tile_sd);
    lv_label_set_text_fmt(g_sd_action_lbl, "Last: %s", fg_sd_last_action_text());
    fg_style_apply_label_dim(g_sd_action_lbl);
    lv_obj_set_width(g_sd_action_lbl, lv_pct(100));
    lv_label_set_long_mode(g_sd_action_lbl, LV_LABEL_LONG_WRAP);

    g_sd_list_lbl = lv_label_create(tile_sd);
    lv_label_set_text(g_sd_list_lbl, "Press List ForgeUI");
    fg_style_apply_label_dim(g_sd_list_lbl);
    lv_obj_set_width(g_sd_list_lbl, lv_pct(100));
    lv_label_set_long_mode(g_sd_list_lbl, LV_LABEL_LONG_WRAP);

    lv_obj_t *btn_sd_test = lv_button_create(tile_sd);
    fg_style_apply_button(btn_sd_test);
    lv_obj_add_event_cb(btn_sd_test, evt_sd_test, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_sd_test_lbl = lv_label_create(btn_sd_test);
    lv_label_set_text(btn_sd_test_lbl, "Test SD");
    lv_obj_center(btn_sd_test_lbl);

    lv_obj_t *btn_sd_list = lv_button_create(tile_sd);
    fg_style_apply_button(btn_sd_list);
    lv_obj_add_event_cb(btn_sd_list, evt_sd_list, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_sd_list_lbl = lv_label_create(btn_sd_list);
    lv_label_set_text(btn_sd_list_lbl, "List ForgeUI");
    lv_obj_center(btn_sd_list_lbl);

    lv_obj_t *btn_sd_prepare = lv_button_create(tile_sd);
    fg_style_apply_button(btn_sd_prepare);
    lv_obj_add_event_cb(btn_sd_prepare, evt_sd_prepare_card, LV_EVENT_CLICKED, NULL);

    g_sd_safe_btn_lbl = lv_label_create(btn_sd_prepare);
    lv_label_set_text(g_sd_safe_btn_lbl, "Safe Reset SD Card (Restart) ⚠");
    lv_obj_center(g_sd_safe_btn_lbl);

    lv_obj_t *btn_sd_reset = lv_button_create(tile_sd);
    fg_style_apply_button(btn_sd_reset);
    lv_obj_add_event_cb(btn_sd_reset, evt_sd_reset_storage, LV_EVENT_CLICKED, NULL);

    g_sd_reset_btn_lbl = lv_label_create(btn_sd_reset);
    lv_label_set_text(g_sd_reset_btn_lbl, "Live Rebuild ForgeUI (No Restart) ⚠");
    lv_obj_center(g_sd_reset_btn_lbl);

    if (!g_sd_timer) {
        g_sd_timer = lv_timer_create(fg_sd_ui_refresh, 1000, NULL);
    }
#endif


        // ===== Tile: RTC =====
#if FORGEUI_ENABLE_RTC
    lv_obj_t *tile_rtc = fg_make_tile(parent, "RTC Time Set", 98, 225);

    lv_obj_set_style_pad_all(tile_rtc, 10, 0);
    lv_obj_set_style_pad_row(tile_rtc, 6, 0);

    lv_obj_t *rtc_grid = lv_obj_create(tile_rtc);
    lv_obj_set_width(rtc_grid, lv_pct(100));
    lv_obj_set_height(rtc_grid, 112);
    lv_obj_set_style_pad_all(rtc_grid, 0, 0);
    lv_obj_set_style_pad_row(rtc_grid, 6, 0);
    lv_obj_set_style_pad_column(rtc_grid, 26, 0);
    lv_obj_set_style_border_width(rtc_grid, 0, 0);
    lv_obj_set_style_bg_opa(rtc_grid, LV_OPA_TRANSP, 0);

    lv_obj_set_layout(rtc_grid, LV_LAYOUT_GRID);

    static lv_coord_t rtc_col_dsc[] = {
    100, 42, 95, 42,
    170,
    100, 42, 95, 42,
    LV_GRID_TEMPLATE_LAST
};

    static lv_coord_t rtc_row_dsc[] = {
        30, 30, 30,
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_grid_dsc_array(rtc_grid, rtc_col_dsc, rtc_row_dsc);

    lv_obj_t *row_year  = fg_make_step_row(rtc_grid, "Year",  &g_lbl_year);
    lv_obj_t *row_hour  = fg_make_step_row(rtc_grid, "Hour",  &g_lbl_hour);
    lv_obj_t *row_month = fg_make_step_row(rtc_grid, "Month", &g_lbl_month);
    lv_obj_t *row_min   = fg_make_step_row(rtc_grid, "Min",   &g_lbl_min);
    lv_obj_t *row_day   = fg_make_step_row(rtc_grid, "Day",   &g_lbl_day);
    lv_obj_t *row_sec   = fg_make_step_row(rtc_grid, "Sec",   &g_lbl_sec);

    lv_obj_set_grid_cell(row_year,  LV_GRID_ALIGN_STRETCH, 0, 4, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(row_hour,  LV_GRID_ALIGN_STRETCH, 5, 4, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_set_grid_cell(row_month, LV_GRID_ALIGN_STRETCH, 0, 4, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_set_grid_cell(row_min,   LV_GRID_ALIGN_STRETCH, 5, 4, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_set_grid_cell(row_day,   LV_GRID_ALIGN_STRETCH, 0, 4, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(row_sec,   LV_GRID_ALIGN_STRETCH, 5, 4, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_add_event_cb(fg_find_minus_btn(row_year),  evt_year_minus,  LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(fg_find_plus_btn(row_year),   evt_year_plus,   LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(fg_find_minus_btn(row_month), evt_month_minus, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(fg_find_plus_btn(row_month),  evt_month_plus,  LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(fg_find_minus_btn(row_day),   evt_day_minus,   LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(fg_find_plus_btn(row_day),    evt_day_plus,    LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(fg_find_minus_btn(row_hour),  evt_hour_minus,  LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(fg_find_plus_btn(row_hour),   evt_hour_plus,   LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(fg_find_minus_btn(row_min),   evt_min_minus,   LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(fg_find_plus_btn(row_min),    evt_min_plus,    LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(fg_find_minus_btn(row_sec),   evt_sec_minus,   LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(fg_find_plus_btn(row_sec),    evt_sec_plus,    LV_EVENT_CLICKED, NULL);

    fg_rtc_get(&g_year, &g_month, &g_day, &g_hour, &g_min, &g_sec);

    if (g_year < 2020) {
        g_year  = 2026;
        g_month = 1;
        g_day   = 1;
        g_hour  = 12;
        g_min   = 0;
        g_sec   = 0;
    }

    fg_refresh_rtc_labels();

    lv_obj_t *rtc_bottom = lv_obj_create(tile_rtc);
    lv_obj_set_width(rtc_bottom, lv_pct(100));
    lv_obj_set_height(rtc_bottom, 44);
    lv_obj_set_style_pad_all(rtc_bottom, 0, 0);
    lv_obj_set_style_border_width(rtc_bottom, 0, 0);
    lv_obj_set_style_bg_opa(rtc_bottom, LV_OPA_TRANSP, 0);

    lv_obj_set_layout(rtc_bottom, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(rtc_bottom, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(rtc_bottom,
        LV_FLEX_ALIGN_SPACE_BETWEEN,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    g_sys_rtc_status = lv_label_create(rtc_bottom);
    lv_label_set_text(g_sys_rtc_status, "DS3231 + ESP runtime RTC active");
    lv_obj_set_style_text_font(g_sys_rtc_status, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(g_sys_rtc_status, fg_style_text_dim(), 0);

    lv_obj_t *btn_apply = lv_button_create(rtc_bottom);
    fg_style_apply_button(btn_apply);
    lv_obj_set_size(btn_apply, 150, 40);
    lv_obj_add_event_cb(btn_apply, evt_apply_rtc, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_apply_lbl = lv_label_create(btn_apply);
    lv_label_set_text(btn_apply_lbl, "Apply Time");
    lv_obj_set_style_text_font(btn_apply_lbl, &lv_font_montserrat_16, 0);
    lv_obj_center(btn_apply_lbl);

#endif
}

static void evt_system_hub_admin(lv_event_t *e)
{
    LV_UNUSED(e);
    fg_hmi_go_admin();
}

static void evt_system_hub_placeholder(lv_event_t *e)
{
    LV_UNUSED(e);
    ESP_LOGI(TAG, "System hub card pressed");
}

static lv_obj_t *fg_make_hub_card(lv_obj_t *parent,
                                  const char *label,
                                  const void *icon,
                                  lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_button_create(parent);
    fg_style_apply_tile(btn);

    lv_obj_set_size(btn, 210, 170);

    lv_obj_set_layout(btn, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *img = lv_image_create(btn);
    lv_image_set_src(img, icon);

    lv_obj_t *txt = lv_label_create(btn);
    lv_label_set_text(txt, label);
    fg_style_apply_label(txt);
    lv_obj_set_style_text_font(txt, &lv_font_montserrat_18, 0);

    return btn;
}

static void fg_system_modal_cleanup(void)
{
#if FORGEUI_ENABLE_WIFI
    if (g_wifi_timer) {
        lv_timer_delete(g_wifi_timer);
        g_wifi_timer = NULL;
    }

    g_wifi_status_lbl = NULL;
    g_wifi_ip_lbl     = NULL;
    g_wifi_dropdown   = NULL;
    g_wifi_pass_ta    = NULL;
#endif

#if FORGEUI_ENABLE_SD
    if (g_sd_timer) {
        lv_timer_delete(g_sd_timer);
        g_sd_timer = NULL;
    }

    g_sd_status_lbl    = NULL;
    g_sd_action_lbl    = NULL;
    g_sd_size_lbl      = NULL;
    g_sd_safe_btn_lbl  = NULL;
    g_sd_list_lbl      = NULL;
    g_sd_reset_btn_lbl = NULL;

    g_sd_safe_armed  = false;
    g_sd_reset_armed = false;
#endif
}

static void evt_system_hub_brightness(lv_event_t *e)
{
    LV_UNUSED(e);

     fg_system_modal_cleanup();

    lv_obj_t *box =
        fg_reactor_modal_open("Brightness",
                              fg_icon_brightness());

    g_sys_brightness_value = lv_label_create(box);
    lv_label_set_text(g_sys_brightness_value, "50%");
    fg_style_apply_label(g_sys_brightness_value);

    lv_obj_t *slider = lv_slider_create(box);

    lv_obj_set_width(slider, lv_pct(92));

    lv_slider_set_range(slider, 10, 100);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);

    lv_obj_add_event_cb(slider,
                        evt_brightness_changed,
                        LV_EVENT_VALUE_CHANGED,
                        NULL);

    lv_obj_t *hint = lv_label_create(box);
    lv_label_set_text(hint,
                      "Live display brightness control");

    fg_style_apply_label_dim(hint);
}

static void evt_system_hub_sound(lv_event_t *e)
{
    LV_UNUSED(e);

     fg_system_modal_cleanup();

    lv_obj_t *box =
        fg_reactor_modal_open("Sound",
                              fg_icon_sound());

    lv_obj_t *status = lv_label_create(box);
    lv_label_set_text(status, "Audio Ready");
    fg_style_apply_label(status);

    g_sys_sound_volume_value = lv_label_create(box);
    lv_label_set_text(g_sys_sound_volume_value, "70%");
    fg_style_apply_label(g_sys_sound_volume_value);

    lv_obj_t *slider = lv_slider_create(box);

    lv_obj_set_width(slider, lv_pct(92));

    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, g_sound_volume, LV_ANIM_OFF);

    lv_obj_add_event_cb(slider,
                        evt_sound_volume_changed,
                        LV_EVENT_VALUE_CHANGED,
                        NULL);

    lv_obj_t *btn_test = lv_button_create(box);
    fg_style_apply_button(btn_test);

    lv_obj_set_size(btn_test, 180, 44);

    lv_obj_add_event_cb(btn_test,
                        evt_sound_test,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_t *lbl = lv_label_create(btn_test);
    lv_label_set_text(lbl, "Speaker Test");
    lv_obj_center(lbl);

    lv_obj_t *hint = lv_label_create(box);

    lv_label_set_text(hint,
                      "Live audio volume control");

    fg_style_apply_label_dim(hint);
}

#if FORGEUI_ENABLE_WIFI
static void evt_system_hub_wifi(lv_event_t *e)
{
    LV_UNUSED(e);

     fg_system_modal_cleanup();

    lv_obj_t *box =
        fg_reactor_modal_open("WiFi",
                              fg_icon_wifi());

    g_wifi_status_lbl = lv_label_create(box);
    lv_label_set_text_fmt(g_wifi_status_lbl,
                          "Status: %s",
                          fg_wifi_status_text());
    fg_style_apply_label(g_wifi_status_lbl);

    g_wifi_ip_lbl = lv_label_create(box);
    lv_label_set_text_fmt(g_wifi_ip_lbl,
                          "IP: %s",
                          fg_wifi_ip_text());
    fg_style_apply_label_dim(g_wifi_ip_lbl);

    lv_obj_t *btn_scan = lv_button_create(box);
    fg_style_apply_button(btn_scan);
    lv_obj_set_size(btn_scan, 180, 42);
    lv_obj_add_event_cb(btn_scan,
                        evt_wifi_scan,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_t *btn_scan_lbl = lv_label_create(btn_scan);
    lv_label_set_text(btn_scan_lbl, "Scan WiFi");
    lv_obj_center(btn_scan_lbl);

    g_wifi_dropdown = lv_dropdown_create(box);
    fg_style_apply_dropdown(g_wifi_dropdown);
    lv_obj_set_width(g_wifi_dropdown, lv_pct(92));
    lv_dropdown_set_options(g_wifi_dropdown, "No scan yet");

    g_wifi_pass_ta = lv_textarea_create(box);
    fg_style_apply_textarea(g_wifi_pass_ta);
    lv_obj_set_width(g_wifi_pass_ta, lv_pct(92));
    lv_textarea_set_one_line(g_wifi_pass_ta, true);
    lv_textarea_set_password_mode(g_wifi_pass_ta, true);
    lv_textarea_set_placeholder_text(g_wifi_pass_ta, "Password");

    lv_obj_add_event_cb(g_wifi_pass_ta,
                        evt_wifi_pass_focus,
                        LV_EVENT_FOCUSED,
                        NULL);

    lv_obj_add_event_cb(g_wifi_pass_ta,
                        evt_wifi_pass_ready,
                        LV_EVENT_READY,
                        NULL);

    lv_obj_add_event_cb(g_wifi_pass_ta,
                        evt_wifi_pass_ready,
                        LV_EVENT_CANCEL,
                        NULL);

    lv_obj_t *btn_connect = lv_button_create(box);
    fg_style_apply_button(btn_connect);
    lv_obj_set_size(btn_connect, 180, 42);
    lv_obj_add_event_cb(btn_connect,
                        evt_wifi_connect,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_t *btn_connect_lbl = lv_label_create(btn_connect);
    lv_label_set_text(btn_connect_lbl, "Connect");
    lv_obj_center(btn_connect_lbl);

    lv_obj_t *row = lv_obj_create(box);
    lv_obj_remove_style_all(row);
    lv_obj_set_width(row, lv_pct(92));
    lv_obj_set_height(row, 48);

    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row,
                          LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_t *btn_disconnect = lv_button_create(row);
    fg_style_apply_button(btn_disconnect);
    lv_obj_set_size(btn_disconnect, 150, 42);
    lv_obj_add_event_cb(btn_disconnect,
                        evt_wifi_disconnect,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_t *btn_disconnect_lbl = lv_label_create(btn_disconnect);
    lv_label_set_text(btn_disconnect_lbl, "Disconnect");
    lv_obj_center(btn_disconnect_lbl);

    lv_obj_t *btn_forget = lv_button_create(row);
    fg_style_apply_button(btn_forget);
    lv_obj_set_size(btn_forget, 150, 42);
    lv_obj_add_event_cb(btn_forget,
                        evt_wifi_forget,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_t *btn_forget_lbl = lv_label_create(btn_forget);
    lv_label_set_text(btn_forget_lbl, "Forget");
    lv_obj_center(btn_forget_lbl);

    if (!g_wifi_timer) {
        g_wifi_timer = lv_timer_create(fg_wifi_ui_refresh,
                                       1000,
                                       NULL);
    }
}
#endif

#if FORGEUI_ENABLE_SD
static void evt_system_hub_storage(lv_event_t *e)
{
    LV_UNUSED(e);

     fg_system_modal_cleanup();

    lv_obj_t *box =
        fg_reactor_modal_open("Storage",
                              fg_icon_sdcard());

    g_sd_status_lbl = lv_label_create(box);
    lv_label_set_text_fmt(g_sd_status_lbl,
                          "Status: %s",
                          fg_sd_status_text());
    fg_style_apply_label(g_sd_status_lbl);

    g_sd_size_lbl = lv_label_create(box);
    lv_label_set_text(g_sd_size_lbl,
                      fg_sd_size_text_get());
    fg_style_apply_label_dim(g_sd_size_lbl);

    g_sd_action_lbl = lv_label_create(box);
    lv_label_set_text_fmt(g_sd_action_lbl,
                          "Last: %s",
                          fg_sd_last_action_text());
    fg_style_apply_label_dim(g_sd_action_lbl);
    lv_obj_set_width(g_sd_action_lbl, lv_pct(92));
    lv_label_set_long_mode(g_sd_action_lbl,
                           LV_LABEL_LONG_WRAP);

    lv_obj_t *btn_test = lv_button_create(box);
    fg_style_apply_button(btn_test);
    lv_obj_set_size(btn_test, 190, 42);
    lv_obj_add_event_cb(btn_test,
                        evt_sd_test,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_t *lbl_test = lv_label_create(btn_test);
    lv_label_set_text(lbl_test, "Test SD");
    lv_obj_center(lbl_test);

    lv_obj_t *btn_list = lv_button_create(box);
    fg_style_apply_button(btn_list);
    lv_obj_set_size(btn_list, 190, 42);
    lv_obj_add_event_cb(btn_list,
                        evt_sd_list,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_t *lbl_list = lv_label_create(btn_list);
    lv_label_set_text(lbl_list, "List ForgeUI");
    lv_obj_center(lbl_list);

    g_sd_list_lbl = lv_label_create(box);
    lv_label_set_text(g_sd_list_lbl,
                      "Press List ForgeUI");
    fg_style_apply_label_dim(g_sd_list_lbl);
    lv_obj_set_width(g_sd_list_lbl, lv_pct(92));
    lv_label_set_long_mode(g_sd_list_lbl,
                           LV_LABEL_LONG_WRAP);

    lv_obj_t *btn_reset = lv_button_create(box);
    fg_style_apply_button(btn_reset);
    lv_obj_set_size(btn_reset, 250, 42);
    lv_obj_add_event_cb(btn_reset,
                        evt_sd_reset_storage,
                        LV_EVENT_CLICKED,
                        NULL);

    g_sd_reset_btn_lbl = lv_label_create(btn_reset);
    lv_label_set_text(g_sd_reset_btn_lbl,
                      "Live Rebuild ForgeUI");
    lv_obj_center(g_sd_reset_btn_lbl);

    if (!g_sd_timer) {
        g_sd_timer = lv_timer_create(fg_sd_ui_refresh,
                                     1000,
                                     NULL);
    }
}
#endif

#if FORGEUI_ENABLE_RTC
static void evt_system_hub_time(lv_event_t *e)
{
    LV_UNUSED(e);

    fg_system_modal_cleanup();

    fg_rtc_get(&g_year,
               &g_month,
               &g_day,
               &g_hour,
               &g_min,
               &g_sec);

    lv_obj_t *box =
        fg_reactor_modal_open("Time",
                              fg_icon_time());

    g_sys_rtc_status = lv_label_create(box);
    lv_label_set_text(g_sys_rtc_status,
                      "Set date and time");
    fg_style_apply_label(g_sys_rtc_status);

    lv_obj_t *row_year  = fg_make_step_row(box, "Year",  &g_lbl_year);
    lv_obj_t *row_month = fg_make_step_row(box, "Month", &g_lbl_month);
    lv_obj_t *row_day   = fg_make_step_row(box, "Day",   &g_lbl_day);
    lv_obj_t *row_hour  = fg_make_step_row(box, "Hour",  &g_lbl_hour);
    lv_obj_t *row_min   = fg_make_step_row(box, "Min",   &g_lbl_min);

    lv_obj_add_event_cb(fg_find_minus_btn(row_year),
                        evt_year_minus,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_add_event_cb(fg_find_plus_btn(row_year),
                        evt_year_plus,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_add_event_cb(fg_find_minus_btn(row_month),
                        evt_month_minus,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_add_event_cb(fg_find_plus_btn(row_month),
                        evt_month_plus,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_add_event_cb(fg_find_minus_btn(row_day),
                        evt_day_minus,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_add_event_cb(fg_find_plus_btn(row_day),
                        evt_day_plus,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_add_event_cb(fg_find_minus_btn(row_hour),
                        evt_hour_minus,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_add_event_cb(fg_find_plus_btn(row_hour),
                        evt_hour_plus,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_add_event_cb(fg_find_minus_btn(row_min),
                        evt_min_minus,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_add_event_cb(fg_find_plus_btn(row_min),
                        evt_min_plus,
                        LV_EVENT_CLICKED,
                        NULL);

    fg_refresh_rtc_labels();

    lv_obj_t *btn_apply = lv_button_create(box);
    fg_style_apply_button(btn_apply);

    lv_obj_set_size(btn_apply, 180, 44);

    lv_obj_add_event_cb(btn_apply,
                        evt_apply_rtc,
                        LV_EVENT_CLICKED,
                        NULL);

    lv_obj_t *lbl = lv_label_create(btn_apply);
    lv_label_set_text(lbl, "Apply Time");
    lv_obj_center(lbl);
}
#endif

void ui_system_build(lv_obj_t *parent)
{
    fg_style_apply_screen(parent);

    lv_obj_set_style_pad_all(parent, 24, 0);

    lv_obj_t *panel = lv_obj_create(parent);
    fg_style_apply_tile(panel);

    lv_obj_set_size(panel, lv_pct(100), lv_pct(100));
    lv_obj_center(panel);

    lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_set_flex_align(panel,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_pad_row(panel, 24, 0);
    lv_obj_set_style_pad_column(panel, 24, 0);

    fg_make_hub_card(panel, "Brightness", fg_icon_brightness(), evt_system_hub_brightness);
    fg_make_hub_card(panel, "Sound",      fg_icon_sound(),      evt_system_hub_sound);
    fg_make_hub_card(panel, "WiFi",       fg_icon_wifi(),       evt_system_hub_wifi);
    fg_make_hub_card(panel, "Storage",    fg_icon_sdcard(),     evt_system_hub_storage);
    #if FORGEUI_ENABLE_RTC
    fg_make_hub_card(panel,
                     "Time",
                     fg_icon_time(),
                     evt_system_hub_time);
#endif
    fg_make_hub_card(panel, "Admin",      fg_icon_admin(),      evt_system_hub_admin);
}
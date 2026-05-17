// ============================================================
// ForgeUI RTC System
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
// Shared runtime RTC and timekeeping layer.
//
// Responsibilities:
//
// - runtime system time ownership
// - DS3231 integration
// - NVS fallback persistence
// - time formatting helpers
// - boot-time clock restore
// - shared runtime clock services
//
// Current V1 Architecture:
//
// DS3231
//   -> persistent hardware RTC
//
// ESP system time
//   -> active runtime clock
//
// NVS
//   -> fallback persistence layer
//
// Architecture Rules:
//
// - BSP owns I2C bus initialization
// - RTC layer attaches only
// - runtime reads use ESP system time only
// - DS3231 used at boot and apply/set
// - UI never owns runtime time truth
//
// Controlled Through:
//
//   FORGEUI_RTC_BACKEND
//
// Supported Backends:
//
// - NONE
// - ESP_NVS
// - DS3231
//
// Current Runtime Ownership:
//
// Runtime clock truth always comes from:
//
//   ESP system time
//
// DS3231 acts as:
//
// - persistent restore source
// - hardware retention layer
//
// Future Direction:
//
// - NTP sync
// - timezone support
// - DST support
// - RTC diagnostics
// - cloud time sync
// - RTC health monitoring
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

#include "20_RTC.h"
#include "00_ForgeUI_Config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"

#if FORGEUI_RTC_BACKEND == FORGEUI_RTC_BACKEND_DS3231
#include "driver/i2c_master.h"
#include "bsp/esp-bsp.h"
#endif

static const char *TAG = "FORGEUI_RTC";

static const char *g_day_names[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

#if FORGEUI_RTC_BACKEND == FORGEUI_RTC_BACKEND_DS3231
static i2c_master_bus_handle_t s_ds_bus = NULL;
static i2c_master_dev_handle_t s_ds_dev = NULL;

static uint8_t rtc_bcd_to_dec(uint8_t val)
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

static uint8_t rtc_dec_to_bcd(uint8_t val)
{
    return ((val / 10) << 4) | (val % 10);
}

static void rtc_ds3231_init(void)
{
    if (s_ds_dev != NULL) {
        return;
    }

    // Waveshare BSP owns I2C init.
    // DS3231 only attaches as another device on the BSP bus.
    s_ds_bus = bsp_i2c_get_handle();

    if (s_ds_bus == NULL) {
        ESP_LOGE(TAG, "DS3231: BSP I2C bus not available");
        return;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = FORGEUI_DS3231_ADDR,
        .scl_speed_hz = 100000,
    };

    esp_err_t err = i2c_master_bus_add_device(s_ds_bus, &dev_cfg, &s_ds_dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "DS3231 add device failed: %s", esp_err_to_name(err));
        s_ds_dev = NULL;
        return;
    }

    ESP_LOGI(TAG, "DS3231 attached to BSP I2C bus at 0x%02X", FORGEUI_DS3231_ADDR);
}

static bool rtc_ds3231_get(int *y, int *m, int *d, int *h, int *min, int *s)
{
    if (s_ds_dev == NULL) {
        rtc_ds3231_init();
    }

    if (s_ds_dev == NULL) {
        ESP_LOGW(TAG, "DS3231 not ready");
        return false;
    }

    uint8_t reg = 0x00;
    uint8_t data[7] = {0};

    esp_err_t err = i2c_master_transmit_receive(
        s_ds_dev,
        &reg,
        1,
        data,
        7,
        100
    );

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "DS3231 read failed: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG,
             "DS3231 RAW %02X %02X %02X %02X %02X %02X %02X",
             data[0], data[1], data[2], data[3],
             data[4], data[5], data[6]);

    if (s)   *s   = rtc_bcd_to_dec(data[0] & 0x7F);
    if (min) *min = rtc_bcd_to_dec(data[1] & 0x7F);
    if (h)   *h   = rtc_bcd_to_dec(data[2] & 0x3F);
    if (d)   *d   = rtc_bcd_to_dec(data[4] & 0x3F);
    if (m)   *m   = rtc_bcd_to_dec(data[5] & 0x1F);
    if (y)   *y   = 2000 + rtc_bcd_to_dec(data[6]);

    return true;
}

static bool rtc_ds3231_set(int y, int m, int d, int h, int min, int s)
{
    if (s_ds_dev == NULL) {
        rtc_ds3231_init();
    }

    if (s_ds_dev == NULL) {
        ESP_LOGW(TAG, "DS3231 not ready, cannot set");
        return false;
    }

    ESP_LOGI(TAG, "DS3231 WRITE: %04d-%02d-%02d %02d:%02d:%02d",
             y, m, d, h, min, s);

    uint8_t data[8];

    data[0] = 0x00;
    data[1] = rtc_dec_to_bcd(s) & 0x7F;     // clear CH bit, start oscillator
    data[2] = rtc_dec_to_bcd(min) & 0x7F;
    data[3] = rtc_dec_to_bcd(h) & 0x3F;     // force 24-hour mode
    data[4] = 1;                            // day-of-week placeholder
    data[5] = rtc_dec_to_bcd(d);
    data[6] = rtc_dec_to_bcd(m);
    data[7] = rtc_dec_to_bcd(y - 2000);

    esp_err_t err = i2c_master_transmit(s_ds_dev, data, sizeof(data), 100);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "DS3231 write failed: %s", esp_err_to_name(err));
        return false;
    }

    int ry = 0, rm = 0, rd = 0, rh = 0, rmin = 0, rs = 0;
    if (rtc_ds3231_get(&ry, &rm, &rd, &rh, &rmin, &rs)) {
        ESP_LOGI(TAG, "DS3231 READBACK: %04d-%02d-%02d %02d:%02d:%02d",
                 ry, rm, rd, rh, rmin, rs);
    }

    return true;
}
#endif

// ---------- ESP/NVS FALLBACK ----------

static void fg_rtc_save_epoch_nvs(time_t epoch)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open("forgeui_rtc", NVS_READWRITE, &h);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS open failed on save: %s", esp_err_to_name(err));
        return;
    }

    nvs_set_i64(h, "epoch", (int64_t)epoch);
    err = nvs_commit(h);
    nvs_close(h);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "RTC epoch saved to NVS: %lld", (long long)epoch);
    }
}

static bool fg_rtc_load_epoch_nvs(time_t *out_epoch)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open("forgeui_rtc", NVS_READONLY, &h);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "No RTC NVS epoch yet");
        return false;
    }

    int64_t epoch = 0;
    err = nvs_get_i64(h, "epoch", &epoch);
    nvs_close(h);

    if (err != ESP_OK || epoch <= 0) {
        ESP_LOGW(TAG, "RTC NVS epoch missing/invalid");
        return false;
    }

    *out_epoch = (time_t)epoch;
    ESP_LOGI(TAG, "RTC epoch loaded from NVS: %lld", (long long)epoch);
    return true;
}

static void esp_nvs_set_time(int year, int month, int day, int hour, int min, int sec)
{
    struct tm t = {
        .tm_year = year - 1900,
        .tm_mon  = month - 1,
        .tm_mday = day,
        .tm_hour = hour,
        .tm_min  = min,
        .tm_sec  = sec,
        .tm_isdst = -1
    };

    time_t epoch = mktime(&t);

    struct timeval tv = {
        .tv_sec = epoch,
        .tv_usec = 0
    };

    settimeofday(&tv, NULL);
    fg_rtc_save_epoch_nvs(epoch);
}

static void esp_nvs_get_time(int *year, int *month, int *day, int *hour, int *min, int *sec)
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    if (year)  *year  = timeinfo.tm_year + 1900;
    if (month) *month = timeinfo.tm_mon + 1;
    if (day)   *day   = timeinfo.tm_mday;
    if (hour)  *hour  = timeinfo.tm_hour;
    if (min)   *min   = timeinfo.tm_min;
    if (sec)   *sec   = timeinfo.tm_sec;
}

static bool rtc_values_valid(int y, int m, int d, int h, int min, int s)
{
    return
        (y >= 2024 && y <= 2099) &&
        (m >= 1 && m <= 12) &&
        (d >= 1 && d <= 31) &&
        (h >= 0 && h <= 23) &&
        (min >= 0 && min <= 59) &&
        (s >= 0 && s <= 59);
}

// ---------- PUBLIC API ----------

void fg_rtc_init(void)
{
#if FORGEUI_RTC_BACKEND == FORGEUI_RTC_BACKEND_DS3231
    rtc_ds3231_init();

    int y = 0, m = 0, d = 0, h = 0, min = 0, s = 0;

    if (rtc_ds3231_get(&y, &m, &d, &h, &min, &s) &&
        rtc_values_valid(y, m, d, h, min, s)) {

        esp_nvs_set_time(y, m, d, h, min, s);

        ESP_LOGI(TAG, "DS3231 loaded VALID: %04d-%02d-%02d %02d:%02d:%02d",
                 y, m, d, h, min, s);
        return;
    }

    ESP_LOGW(TAG, "DS3231 invalid/unavailable, using fallback");

#endif

    time_t epoch = 0;
    if (fg_rtc_load_epoch_nvs(&epoch)) {
        struct timeval tv = {
            .tv_sec = epoch,
            .tv_usec = 0
        };
        settimeofday(&tv, NULL);
        ESP_LOGW(TAG, "Fallback loaded from NVS");
    } else {
        esp_nvs_set_time(2026, 4, 26, 12, 0, 0);
        ESP_LOGW(TAG, "Fallback default time loaded");
    }
}

void fg_rtc_set(int year, int month, int day, int hour, int min)
{
#if FORGEUI_RTC_BACKEND == FORGEUI_RTC_BACKEND_DS3231
    rtc_ds3231_set(year, month, day, hour, min, 0);
#endif

    // Runtime clock always uses ESP system time.
    esp_nvs_set_time(year, month, day, hour, min, 0);

    ESP_LOGI(TAG, "RTC set: %04d-%02d-%02d %02d:%02d",
             year, month, day, hour, min);
}

void fg_rtc_get(int *year, int *month, int *day, int *hour, int *min, int *sec)
{
    // Runtime reads ESP system time only.
    // DS3231 is used at boot restore and on Apply/save.
    esp_nvs_get_time(year, month, day, hour, min, sec);
}

void fg_rtc_format_day(char *out, int max)
{
    int y, m, d, h, min, s;
    fg_rtc_get(&y, &m, &d, &h, &min, &s);

    struct tm t = {
        .tm_year = y - 1900,
        .tm_mon  = m - 1,
        .tm_mday = d,
        .tm_hour = h,
        .tm_min  = min,
        .tm_sec  = s,
        .tm_isdst = -1
    };

    mktime(&t);

    snprintf(out, max, "%s %02d",
             g_day_names[t.tm_wday],
             d);
}

void fg_rtc_format_time(char *out, int max)
{
    int y, m, d, h, min, s;
    fg_rtc_get(&y, &m, &d, &h, &min, &s);

    snprintf(out, max, "%02d:%02d", h, min);
}

void fg_rtc_format_header(char *out, int max)
{
    char day[16];
    char time_txt[16];

    fg_rtc_format_day(day, sizeof(day));
    fg_rtc_format_time(time_txt, sizeof(time_txt));

    snprintf(out, max, "%s\n%s", day, time_txt);
}
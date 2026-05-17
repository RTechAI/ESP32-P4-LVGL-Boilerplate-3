// ============================================================
// ForgeUI Audio System
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
// Shared audio backend for ForgeUI.
//
// Responsibilities:
//
// - initialize BSP audio path
// - initialize speaker codec
// - manage output volume
// - provide simple speaker test beep
// - expose audio validation path to UI
//
// Current Hardware Path:
//
// Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
// -> BSP audio initialization
// -> ESP codec device speaker output
//
// Current Features:
//
// - BSP audio initialization
// - speaker codec open
// - volume control
// - sine-wave test beep
// - busy guard for beep playback
//
// Architecture Rules:
//
// - backend owns audio truth
// - UI sends intent only
// - no LVGL ownership here
// - no UI styling here
//
// Controlled Through:
//
//   FORGEUI_ENABLE_AUDIO
//
// Future Direction:
//
// - notification tones
// - alert sounds
// - startup sound
// - touch feedback sound
// - selectable sound profiles
// - product/customer audio themes
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

#include "30_Audio.h"
#include "esp_log.h"
#include "esp_err.h"

#include "bsp/esp-bsp.h"
#include "bsp/esp32_p4_wifi6_touch_lcd_7b.h"
#include "esp_codec_dev.h"

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

static const char *TAG = "FG_AUDIO";

static esp_codec_dev_handle_t g_speaker = NULL;
static bool g_ready = false;
static bool g_busy  = false;
static int  g_volume = 70;

#define SAMPLE_RATE 16000
#define DURATION_MS 150
#define FREQ        1000

static int16_t g_beep_buffer[SAMPLE_RATE * DURATION_MS / 1000];

esp_err_t fg_audio_init(void)
{
    if (g_ready) return ESP_OK;

    ESP_LOGI(TAG, "Init audio...");

    esp_err_t err = bsp_audio_init(NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "bsp_audio_init failed: %s", esp_err_to_name(err));
        return err;
    }

    g_speaker = bsp_audio_codec_speaker_init();
    if (!g_speaker) {
        ESP_LOGE(TAG, "Speaker init failed");
        return ESP_FAIL;
    }

    esp_codec_dev_sample_info_t fs = {
        .sample_rate = SAMPLE_RATE,
        .channel = 1,
        .bits_per_sample = 16,
    };

    err = esp_codec_dev_open(g_speaker, &fs);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_codec_dev_open failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_codec_dev_set_out_vol(g_speaker, g_volume);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Initial volume set failed: %s", esp_err_to_name(err));
    }

    g_ready = true;
    ESP_LOGI(TAG, "Audio ready + opened");

    return ESP_OK;
}

esp_err_t fg_audio_set_volume(int volume)
{
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;

    g_volume = volume;

    esp_err_t err = fg_audio_init();
    if (err != ESP_OK) return err;

    err = esp_codec_dev_set_out_vol(g_speaker, g_volume);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Volume set failed: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Volume set: %d%%", g_volume);
    }

    return err;
}

esp_err_t fg_audio_test_beep(void)
{
    if (g_busy) {
        ESP_LOGW(TAG, "Beep already running");
        return ESP_OK;
    }

    g_busy = true;

    esp_err_t err = fg_audio_init();
    if (err != ESP_OK) {
        g_busy = false;
        return err;
    }

    esp_codec_dev_set_out_vol(g_speaker, g_volume);

    const int samples = SAMPLE_RATE * DURATION_MS / 1000;

    for (int i = 0; i < samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        g_beep_buffer[i] = (int16_t)(sinf(2.0f * (float)M_PI * FREQ * t) * 10000);
    }

    err = esp_codec_dev_write(g_speaker, g_beep_buffer, samples * sizeof(int16_t));

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Write failed: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "BEEP sent at %d%% volume", g_volume);
    }

    g_busy = false;
    return err;
}
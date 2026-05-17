// ============================================================
// ForgeUI Hosted WiFi System
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
// ESP32-P4 hosted WiFi backend.
//
// Responsibilities:
//
// - ESP-Hosted WiFi initialization
// - STA mode management
// - WiFi scanning
// - connection handling
// - IP status tracking
// - runtime WiFi state ownership
//
// Hardware Architecture:
//
// ESP32-P4
//   -> host processor
//
// ESP32-C6
//   -> onboard WiFi radio
//
// Transport:
//
//   ESP-Hosted / WiFi Remote
//
// Current V1 Features:
//
// - hosted WiFi initialization
// - scan
// - connect/disconnect
// - password authentication
// - IP acquisition
// - forget network
// - scan result caching
// - runtime status abstraction
//
// Architecture Rules:
//
// - backend owns WiFi truth and state
// - UI sends intent only
// - no LVGL ownership here
// - no UI styling here
// - no direct UI dependencies
//
// Controlled Through:
//
//   FORGEUI_ENABLE_WIFI
//
// Current Runtime Ownership:
//
// Backend owns:
//
// - WiFi connection state
// - IP state
// - scan state
// - SSID cache
// - hosted transport state
//
// UI reads backend state only.
//
// Future Direction:
//
// - saved networks
// - reconnect manager
// - RSSI reporting
// - NTP sync
// - cloud APIs
// - roaming support
// - signal quality UI
// - captive portal support
//
// Target Hardware:
//
// - Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
//
// Core Stack:
//
// - ESP-IDF
// - ESP-Hosted
// - ESP WiFi Remote
// - LVGL v9
//
// ============================================================

// ============================================================
// Includes
// ============================================================

#include "30_WIFI.h"

#include <string.h>
#include <stdio.h>

#include "esp_log.h"
#include "esp_err.h"

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Hosted Wi-Fi layer: P4 -> C6

static const char *TAG = "FG_WIFI";

#define FG_WIFI_MAX_SCAN 12

static bool g_wifi_ready = false;
static bool g_wifi_connected = false;
static volatile bool g_scan_done_pending = false;

static esp_netif_t *g_sta_netif = NULL;

static char g_status[32] = "OFF";
static char g_ip[32] = "-";

static char g_scan_ssids[FG_WIFI_MAX_SCAN][33];
static int  g_scan_count = 0;

static void fg_wifi_set_status(const char *s)
{
    snprintf(g_status, sizeof(g_status), "%s", s ? s : "-");
}

static void fg_wifi_event_handler(void *arg,
                                  esp_event_base_t event_base,
                                  int32_t event_id,
                                  void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
        {
            ESP_LOGI(TAG, "STA started");
            fg_wifi_set_status("READY");
        }
        else if (event_id == WIFI_EVENT_SCAN_DONE)
        {
            ESP_LOGI(TAG, "SCAN DONE EVENT FIRED");
            g_scan_done_pending = true;
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            ESP_LOGW(TAG, "STA disconnected");
            g_wifi_connected = false;
            snprintf(g_ip, sizeof(g_ip), "-");
            fg_wifi_set_status("DISCONNECTED");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        snprintf(g_ip, sizeof(g_ip), IPSTR, IP2STR(&event->ip_info.ip));

        g_wifi_connected = true;
        fg_wifi_set_status("CONNECTED");

        ESP_LOGI(TAG, "Got IP: %s", g_ip);
    }
}

void fg_wifi_init(void)
{
    ESP_LOGI(TAG, "WiFi hosted init start");
    fg_wifi_set_status("INIT");

    esp_err_t err;

    err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(err));
        fg_wifi_set_status("NETIF_FAIL");
        return;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG, "event loop failed: %s", esp_err_to_name(err));
        fg_wifi_set_status("EVENT_FAIL");
        return;
    }

    if (!g_sta_netif)
    {
        g_sta_netif = esp_netif_create_default_wifi_sta();
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&cfg);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
        fg_wifi_set_status("WIFI_FAIL");
        return;
    }

    esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &fg_wifi_event_handler,
        NULL,
        NULL
    );

    esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &fg_wifi_event_handler,
        NULL,
        NULL
    );

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "set mode failed: %s", esp_err_to_name(err));
        fg_wifi_set_status("MODE_FAIL");
        return;
    }

    err = esp_wifi_start();
    if (err != ESP_OK && err != ESP_ERR_WIFI_CONN)
    {
        ESP_LOGE(TAG, "wifi start failed: %s", esp_err_to_name(err));
        fg_wifi_set_status("START_FAIL");
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(500));

    uint8_t mac[6] = {0};
    esp_err_t mac_err = esp_wifi_get_mac(WIFI_IF_STA, mac);

    ESP_LOGI(TAG, "MAC read err=%s", esp_err_to_name(mac_err));
    ESP_LOGI(TAG, "MAC = %02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2],
             mac[3], mac[4], mac[5]);

    g_wifi_ready = true;
    fg_wifi_set_status("READY");

    ESP_LOGI(TAG, "WiFi hosted init READY");
}

void fg_wifi_pump(void)
{
    if (!g_scan_done_pending)
    {
        return;
    }

    g_scan_done_pending = false;

    uint16_t ap_count = 0;
    esp_err_t err = esp_wifi_scan_get_ap_num(&ap_count);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "scan get count failed: %s", esp_err_to_name(err));
        fg_wifi_set_status("SCAN_COUNT_FAIL");
        return;
    }

    ESP_LOGI(TAG, "Scan AP count: %u", ap_count);

    memset(g_scan_ssids, 0, sizeof(g_scan_ssids));
    g_scan_count = 0;

    if (ap_count == 0)
    {
        fg_wifi_set_status("SCAN_EMPTY");
        return;
    }

    wifi_ap_record_t records[FG_WIFI_MAX_SCAN];
    memset(records, 0, sizeof(records));

    uint16_t max_records = FG_WIFI_MAX_SCAN;

    err = esp_wifi_scan_get_ap_records(&max_records, records);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "scan records failed: %s", esp_err_to_name(err));
        fg_wifi_set_status("SCAN_READ_FAIL");
        return;
    }

    g_scan_count = max_records;

    for (int i = 0; i < g_scan_count; i++)
    {
        snprintf(g_scan_ssids[i],
                 sizeof(g_scan_ssids[i]),
                 "%s",
                 (const char *)records[i].ssid);

        ESP_LOGI(TAG, "AP %d: %s RSSI %d",
                 i,
                 g_scan_ssids[i],
                 records[i].rssi);
    }

    fg_wifi_set_status("SCAN_DONE");
}

bool fg_wifi_is_ready(void)
{
    return g_wifi_ready;
}

bool fg_wifi_is_connected(void)
{
    return g_wifi_connected;
}

const char *fg_wifi_status_text(void)
{
    return g_status;
}

const char *fg_wifi_ip_text(void)
{
    return g_ip;
}

void fg_wifi_scan_start(void)
{
    if (!g_wifi_ready)
    {
        fg_wifi_set_status("NOT_READY");
        return;
    }

    ESP_LOGI(TAG, "WiFi scan start");
    fg_wifi_set_status("SCANNING");

    memset(g_scan_ssids, 0, sizeof(g_scan_ssids));
    g_scan_count = 0;
    g_scan_done_pending = false;

    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false
    };

    esp_err_t err = esp_wifi_scan_start(&scan_config, false);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "scan failed: %s", esp_err_to_name(err));
        fg_wifi_set_status("SCAN_FAIL");
        return;
    }
}

int fg_wifi_get_scan_results(char ssids[][33], int max)
{
    if (!ssids || max <= 0)
    {
        return 0;
    }

    int n = g_scan_count;
    if (n > max) n = max;

    for (int i = 0; i < n; i++)
    {
        snprintf(ssids[i], 33, "%s", g_scan_ssids[i]);
    }

    return n;
}

void fg_wifi_connect(const char *ssid, const char *pass)
{
    if (!g_wifi_ready)
    {
        fg_wifi_set_status("NOT_READY");
        return;
    }

    if (!ssid || ssid[0] == 0)
    {
        fg_wifi_set_status("NO_SSID");
        return;
    }

    ESP_LOGI(TAG, "Connecting to SSID: %s", ssid);
    fg_wifi_set_status("CONNECTING");

    wifi_config_t wifi_config = {0};

    snprintf((char *)wifi_config.sta.ssid,
             sizeof(wifi_config.sta.ssid),
             "%s",
             ssid);

    if (pass)
    {
        snprintf((char *)wifi_config.sta.password,
                 sizeof(wifi_config.sta.password),
                 "%s",
                 pass);
    }

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    esp_wifi_disconnect();

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "set config failed: %s", esp_err_to_name(err));
        fg_wifi_set_status("CONFIG_FAIL");
        return;
    }

    err = esp_wifi_connect();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "connect failed: %s", esp_err_to_name(err));
        fg_wifi_set_status("CONNECT_FAIL");
    }
}

void fg_wifi_disconnect(void)
{
    ESP_LOGI(TAG, "Disconnect requested");
    esp_wifi_disconnect();

    g_wifi_connected = false;
    snprintf(g_ip, sizeof(g_ip), "-");
    fg_wifi_set_status("DISCONNECTED");
}

void fg_wifi_forget(void)
{
    ESP_LOGW(TAG, "Forget WiFi requested");

    fg_wifi_set_status("FORGETTING");

    esp_wifi_disconnect();

    wifi_config_t blank = {0};
    esp_wifi_set_config(WIFI_IF_STA, &blank);

    g_wifi_connected = false;
    snprintf(g_ip, sizeof(g_ip), "-");

    fg_wifi_set_status("FORGOTTEN");
}
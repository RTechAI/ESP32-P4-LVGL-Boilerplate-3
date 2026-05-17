// ============================================================
// ForgeUI SD Storage System
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
// Shared SD card and filesystem backend.
//
// Responsibilities:
//
// - SD card mount and initialization
// - ForgeUI filesystem management
// - storage reset and rebuild
// - SD test/read/write validation
// - runtime storage status
// - filesystem lifecycle ownership
//
// Current V1 Features:
//
// - SD mount
// - SD test file
// - ForgeUI folder structure
// - boot marker support
// - async reset and rebuild
// - folder listing
// - runtime storage status helpers
//
// Runtime Rules:
//
// - backend owns SD state
// - UI sends intent only
// - no LVGL ownership here
// - no UI styling here
//
// Important Hardware Note:
//
// ESP32-P4 Hosted WiFi and SDMMC
// share critical hardware paths.
//
// Proven Stable Boot Order:
//
//   WiFi first
//   -> SD second
//
// Current Stable Baseline:
//
// - Hosted WiFi alive
// - SD alive
// - shared operation stable
//
// Controlled Through:
//
//   FORGEUI_ENABLE_SD
//
// Current Runtime Ownership:
//
// Backend owns:
//
// - SD mount state
// - filesystem lifecycle
// - storage actions
// - folder management
// - reset and rebuild operations
//
// UI reads backend state only.
//
// Future Direction:
//
// - SQLite/light database
// - export manager
// - telemetry storage
// - user database
// - cloud sync/export
// - encrypted storage
// - backup and restore manager
//
// Target Hardware:
//
// - Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
//
// Core Stack:
//
// - ESP-IDF
// - ESP-Hosted
// - SDMMC
// - LVGL v9
//
// ============================================================

// ============================================================
// Includes
// ============================================================

#include "40_SD.h"

#include "esp_log.h"
#include "esp_err.h"

#include "esp_ldo_regulator.h"

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "driver/sdmmc_host.h"

#include "bsp/esp32_p4_wifi6_touch_lcd_7b.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/stat.h>

#include <errno.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "FG_SD";

#define MOUNT_POINT          "/sdcard"
#define FG_SD_TEST_PATH      "/sdcard/sd_test.txt"
#define FG_SD_ROOT           "/sdcard/ForgeUI"
#define FG_SD_BOOT_MARKER    "/sdcard/ForgeUI/system/boot_marker.txt"

static esp_ldo_channel_handle_t g_sd_ldo = NULL;
static sdmmc_card_t *g_card = NULL;

static bool g_sd_ready = false;
static char g_sd_status[32] = "OFF";
static char g_sd_last_action[96] = "No SD action yet";
static char g_sd_size_text[64] = "Size: -";

// ============================================================
// Status helpers
// ============================================================

static void fg_sd_set_status(const char *s)
{
    snprintf(g_sd_status, sizeof(g_sd_status), "%s", s ? s : "-");
}

static void fg_sd_set_action(const char *s)
{
    snprintf(g_sd_last_action, sizeof(g_sd_last_action), "%s", s ? s : "-");
}

bool fg_sd_is_ready(void)
{
    return g_sd_ready;
}

const char *fg_sd_status_text(void)
{
    return g_sd_status;
}

const char *fg_sd_last_action_text(void)
{
    return g_sd_last_action;
}

const char *fg_sd_size_text_get(void)
{
    return g_sd_size_text;
}

// ============================================================
// Folder helpers
// ============================================================

static bool fg_sd_mkdir_one(const char *path)
{
    if (!path || path[0] == 0) return false;

    int ret = mkdir(path, 0775);

    if (ret == 0)
    {
        ESP_LOGI(TAG, "Folder created: %s", path);
        return true;
    }

    if (errno == EEXIST)
    {
        ESP_LOGI(TAG, "Folder already exists: %s", path);
        return true;
    }

    ESP_LOGE(TAG, "Folder create failed: %s errno=%d", path, errno);
    return false;
}

static bool fg_sd_delete_recursive(const char *path)
{
    DIR *dir = opendir(path);

    if (!dir)
    {
        ESP_LOGW(TAG, "Delete skip, folder not found: %s", path);
        return true;
    }

    struct dirent *entry;
    char full[256];

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;

        snprintf(full, sizeof(full), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(full, &st) != 0)
        {
            ESP_LOGW(TAG, "stat failed: %s errno=%d", full, errno);
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            fg_sd_delete_recursive(full);

            if (rmdir(full) != 0)
            {
                ESP_LOGW(TAG, "rmdir failed: %s errno=%d", full, errno);
            }
            else
            {
                ESP_LOGI(TAG, "Folder deleted: %s", full);
            }
        }
        else
        {
            if (unlink(full) != 0)
            {
                ESP_LOGW(TAG, "unlink failed: %s errno=%d", full, errno);
            }
            else
            {
                ESP_LOGI(TAG, "File deleted: %s", full);
            }
        }
    }

    closedir(dir);
    return true;
}

// ============================================================
// SD init
// ============================================================

bool fg_sd_init(void)
{
    ESP_LOGI(TAG, "SD init start - manual LDO + manual mount");
    fg_sd_set_status("INIT");
    fg_sd_set_action("Mounting SD");

    if (g_sd_ready && g_card)
    {
        ESP_LOGI(TAG, "SD already mounted");
        fg_sd_set_status("READY");
        fg_sd_set_action("SD already mounted");
        return true;
    }

    if (g_sd_ldo == NULL)
    {
        esp_ldo_channel_config_t ldo_cfg = {
            .chan_id = 4,
            .voltage_mv = 2500
        };

        esp_err_t ldo_ret = esp_ldo_acquire_channel(&ldo_cfg, &g_sd_ldo);

        if (ldo_ret != ESP_OK)
        {
            ESP_LOGE(TAG, "SD LDO acquire failed: %s", esp_err_to_name(ldo_ret));
            fg_sd_set_status("LDO_FAIL");
            fg_sd_set_action("SD LDO failed");
            snprintf(g_sd_size_text, sizeof(g_sd_size_text), "Size: -");
            return false;
        }

        ESP_LOGI(TAG, "SD LDO forced ON: channel 4 @ 2500mV");
    }

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 8,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.slot = SDMMC_HOST_SLOT_0;
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

    sdmmc_slot_config_t slot_config = {
        .clk = BSP_SD_CLK,
        .cmd = BSP_SD_CMD,
        .d0 = BSP_SD_D0,
        .d1 = BSP_SD_D1,
        .d2 = BSP_SD_D2,
        .d3 = BSP_SD_D3,
        .cd = SDMMC_SLOT_NO_CD,
        .wp = SDMMC_SLOT_NO_WP,
        .width = 4,
        .flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP,
    };

    ESP_LOGI(TAG,
             "Pins: SLOT=0 CLK=%d CMD=%d D0=%d D1=%d D2=%d D3=%d WIDTH=4",
             BSP_SD_CLK,
             BSP_SD_CMD,
             BSP_SD_D0,
             BSP_SD_D1,
             BSP_SD_D2,
             BSP_SD_D3);

    esp_err_t ret = esp_vfs_fat_sdmmc_mount(
        MOUNT_POINT,
        &host,
        &slot_config,
        &mount_config,
        &g_card
    );

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SD mount failed: %s", esp_err_to_name(ret));
        g_sd_ready = false;
        fg_sd_set_status("MOUNT_FAIL");
        fg_sd_set_action("SD mount failed");
        snprintf(g_sd_size_text, sizeof(g_sd_size_text), "Size: -");
        return false;
    }

    g_sd_ready = true;
    fg_sd_set_status("READY");
    fg_sd_set_action("SD mounted OK");

    ESP_LOGI(TAG, "SD mounted OK");
    sdmmc_card_print_info(stdout, g_card);

    uint64_t size_mb = ((uint64_t)g_card->csd.capacity) *
                       g_card->csd.sector_size / (1024 * 1024);

    if (size_mb >= 1024)
    {
        uint64_t size_gb = size_mb / 1024;
        snprintf(g_sd_size_text,
                 sizeof(g_sd_size_text),
                 "Size: %llu GB",
                 (unsigned long long)size_gb);
    }
    else
    {
        snprintf(g_sd_size_text,
                 sizeof(g_sd_size_text),
                 "Size: %llu MB",
                 (unsigned long long)size_mb);
    }

    return true;
}

// ============================================================
// Test
// ============================================================

bool fg_sd_test(void)
{
    if (!g_sd_ready)
    {
        ESP_LOGE(TAG, "SD test requested but SD not ready");
        fg_sd_set_status("NOT_READY");
        fg_sd_set_action("SD not ready");
        return false;
    }

    ESP_LOGI(TAG, "SD write test: %s", FG_SD_TEST_PATH);
    fg_sd_set_action("Running SD test");

    FILE *f = fopen(FG_SD_TEST_PATH, "w");
    if (!f)
    {
        ESP_LOGE(TAG, "Failed to open file for write errno=%d", errno);
        fg_sd_set_action("SD write failed");
        return false;
    }

    fprintf(f, "ForgeUI SD OK\n");
    fclose(f);

    char line[64] = {0};

    f = fopen(FG_SD_TEST_PATH, "r");
    if (!f)
    {
        ESP_LOGE(TAG, "Failed to open file for read errno=%d", errno);
        fg_sd_set_action("SD read failed");
        return false;
    }

    fgets(line, sizeof(line), f);
    fclose(f);

    ESP_LOGI(TAG, "Read back: %s", line);

    if (strstr(line, "ForgeUI SD OK"))
    {
        ESP_LOGI(TAG, "SD TEST PASS");
        fg_sd_set_status("READY");
        fg_sd_set_action("SD test PASS");
        return true;
    }

    ESP_LOGE(TAG, "SD TEST FAIL");
    fg_sd_set_action("SD test FAIL");
    return false;
}




// ============================================================
// ForgeUI filesystem
// ============================================================

bool fg_sd_create_folders(void)
{
    if (!g_sd_ready)
    {
        fg_sd_set_status("NOT_READY");
        fg_sd_set_action("Create folders failed: SD not ready");
        return false;
    }

    bool ok = true;

    ok &= fg_sd_mkdir_one("/sdcard/ForgeUI");
    ok &= fg_sd_mkdir_one("/sdcard/ForgeUI/config");
    ok &= fg_sd_mkdir_one("/sdcard/ForgeUI/logs");
    ok &= fg_sd_mkdir_one("/sdcard/ForgeUI/users");
    ok &= fg_sd_mkdir_one("/sdcard/ForgeUI/preop");
    ok &= fg_sd_mkdir_one("/sdcard/ForgeUI/export");
    ok &= fg_sd_mkdir_one("/sdcard/ForgeUI/backups");
    ok &= fg_sd_mkdir_one("/sdcard/ForgeUI/system");

    if (ok)
    {
        fg_sd_set_status("READY");
        fg_sd_set_action("Folder structure ready");
        ESP_LOGI(TAG, "ForgeUI SD folder structure ready");
        return true;
    }

    fg_sd_set_action("Folder structure failed");
    ESP_LOGE(TAG, "ForgeUI SD folder structure failed");
    return false;
}

bool fg_sd_write_boot_marker(void)
{
    if (!g_sd_ready)
    {
        fg_sd_set_status("NOT_READY");
        fg_sd_set_action("Boot marker failed: SD not ready");
        return false;
    }

    if (!fg_sd_create_folders())
    {
        fg_sd_set_action("Boot marker failed: folders failed");
        return false;
    }

    FILE *f = fopen(FG_SD_BOOT_MARKER, "w");
    if (!f)
    {
        ESP_LOGE(TAG, "Boot marker write failed errno=%d", errno);
        fg_sd_set_action("Boot marker write failed");
        return false;
    }

    fprintf(f, "ForgeUI boot marker OK\n");
    fclose(f);

    fg_sd_set_status("READY");
    fg_sd_set_action("Boot marker written");
    ESP_LOGI(TAG, "Boot marker written: %s", FG_SD_BOOT_MARKER);
    return true;
}

bool fg_sd_reset_storage_blocking(void)
{
    if (!g_sd_ready)
    {
        fg_sd_set_status("NOT_READY");
        fg_sd_set_action("Reset failed: SD not ready");
        return false;
    }

    ESP_LOGW(TAG, "ForgeUI storage reset START");
    fg_sd_set_status("RESETTING");
    fg_sd_set_action("Deleting ForgeUI storage");

    fg_sd_delete_recursive(FG_SD_ROOT);

    if (rmdir(FG_SD_ROOT) != 0)
    {
        if (errno != ENOENT)
        {
            ESP_LOGW(TAG, "Root rmdir warning: %s errno=%d", FG_SD_ROOT, errno);
        }
    }
    else
    {
        ESP_LOGI(TAG, "Root folder deleted: %s", FG_SD_ROOT);
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    if (!fg_sd_create_folders())
    {
        fg_sd_set_status("RESET_FAIL");
        fg_sd_set_action("Reset failed: folder rebuild failed");
        return false;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    // 🔧 Removed boot marker write (was failing on fresh FS)
    ESP_LOGW(TAG, "Boot marker skipped during reset");

    fg_sd_set_status("READY");
    fg_sd_set_action("ForgeUI file system reset SUCCESS");

    ESP_LOGW(TAG, "ForgeUI storage reset COMPLETE");
    return true;
}




// ============================================================
// Async reset for UI button
// ============================================================

static void fg_sd_reset_task(void *arg)
{
    (void)arg;

    ESP_LOGW(TAG, "ForgeUI storage reset task START");

    bool ok = fg_sd_reset_storage_blocking();

    if (ok)
    {
        fg_sd_set_status("READY");
        fg_sd_set_action("ForgeUI file system reset SUCCESS");
        ESP_LOGW(TAG, "ForgeUI storage reset SUCCESS");
    }
    else
    {
        fg_sd_set_status("RESET_FAIL");
        fg_sd_set_action("ForgeUI file system reset FAILED");
        ESP_LOGE(TAG, "ForgeUI storage reset FAILED");
    }

    vTaskDelete(NULL);
}

bool fg_sd_reset_async(void)
{
    if (!g_sd_ready)
    {
        fg_sd_set_status("NOT_READY");
        fg_sd_set_action("Reset failed: SD not ready");
        return false;
    }

    ESP_LOGW(TAG, "ForgeUI storage reset async requested");

    xTaskCreate(
        fg_sd_reset_task,
        "sd_reset",
        8192,
        NULL,
        5,
        NULL
    );

    return true;
}

// ============================================================
// List ForgeUI folder
// ============================================================

bool fg_sd_list_forgeui(char *out, int out_len)
{
    if (!out || out_len <= 0) return false;

    snprintf(out, out_len, "ForgeUI folder:\n");

    if (!g_sd_ready)
    {
        strncat(out, "SD not ready", out_len - strlen(out) - 1);
        fg_sd_set_action("List failed: SD not ready");
        return false;
    }

    DIR *dir = opendir(FG_SD_ROOT);

    if (!dir)
    {
        strncat(out,
                "Missing /ForgeUI\nPress Create Folders",
                out_len - strlen(out) - 1);
        fg_sd_set_action("ForgeUI folder missing");
        return false;
    }

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;

        strncat(out, "- ", out_len - strlen(out) - 1);
        strncat(out, entry->d_name, out_len - strlen(out) - 1);
        strncat(out, "\n", out_len - strlen(out) - 1);
        count++;
    }

    closedir(dir);

    if (count == 0)
    {
        strncat(out, "(empty)", out_len - strlen(out) - 1);
    }

    fg_sd_set_action("Folder list updated");
    return true;
}
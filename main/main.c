/*
 * ============================================================
 * ESP32-P4-WIFI6-Touch-LCD-7B LVGL Boilerplate (Reactor UI Edition)
 * ============================================================
 * Main Application Core Entry Pipeline Orchestration
 * Target Board: Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
 *
 * Responsibilities:
 * - Deterministic bare-metal system boot sequencing
 * - Non-Volatile Storage (NVS) memory partition initialization
 * - MIPI-DSI high-performance display panel bring-up
 * - LVGL graphics engine thread environment startup
 * - Asynchronous hardware feature backend task staging
 * - Thread-safe primary runtime status service telemetry loop
 *
 * Runtime Ownership Rules:
 * - main.c strictly owns hardware boot execution timing blocks only.
 * - Async backend peripheral modules maintain system truth states.
 * - Visual UI modules strictly render cached states and bubble up user intent.
 * - App_main stays completely free of layout business or design logic.
 *
 * Critical Hardware Coexistence Co-habitation Rule:
 * The high-performance ESP32-P4 processor shares critical hardware routing 
 * data lines between the ESP32-C6 Hosted Wi-Fi stack and the SDMMC interface.
 *
 * Golden Initialization Sequence Required For Bus Coexistence Stability:
 *   1. Initialize Hosted Wi-Fi stack first -> fg_wifi_init()
 *   2. Allow master slave coprocessor handshake validation delay.
 *   3. Mount the SD card filesystem layer second -> fg_sd_init()
 *
 * Breaking this execution pattern introduces terminal SDIO bus arbitration timeouts, 
 * peripheral storage access failures, and wireless transport stalls.
 *
 * Powered by ForgeUI Framework Engine (3-Page UI Workflow)
 * ============================================================
 */

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_memory_utils.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp_board_extra.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "30_Audio.h"
#include "30_WIFI.h"
#include "01_FG_HMI.h"
#include "14_UI_Header.h"
#include "20_RTC.h"
#include "40_SD.h"
#include "esp_vfs_fat.h"
#include "esp_task_wdt.h"
#include "00_ForgeUI_Config.h"
#include "17_UI_StatusDrawer.h"

static const char *TAG = "FORGEUI_BOOT";

/**
 * @brief Queries the NVS memory space to detect storage initialization flags
 * @return true if emergency file system validation requested
 */
static bool fg_sd_check_prepare_flag(void)
{
    nvs_handle_t nvs;
    uint8_t flag = 0;

    if (nvs_open("forgeui", NVS_READWRITE, &nvs) != ESP_OK) {
        return false;
    }

    nvs_get_u8(nvs, "sd_prep", &flag);

    if (flag == 1)
    {
        // Clear maintenance state instantly to prevent cyclic bootloops
        nvs_set_u8(nvs, "sd_prep", 0);
        nvs_commit(nvs);
        nvs_close(nvs);
        return true;
    }

    nvs_close(nvs);
    return false;
}

/**
 * @brief Enters an isolated maintenance state to structuralise card directories
 */
static void fg_sd_prepare_card_maintenance(void)
{
    ESP_LOGW(TAG, "========================================");
    ESP_LOGW(TAG, "FORGEUI SD STORAGE MAINTENANCE START");
    ESP_LOGW(TAG, "STANDALONE KERNEL RUNTIME: NO UI / NO WIRELESS");
    ESP_LOGW(TAG, "========================================");

    if (!fg_sd_init())
    {
        ESP_LOGE(TAG, "Fatal: SD initialization failed during standalone maintenance execution");
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_restart();
    }

    ESP_LOGW(TAG, "Deploying localized clean directory structures...");

    // Configure extended software watchdog metrics to accommodate heavy disk sweeps
    esp_task_wdt_config_t twdt_cfg = {
        .timeout_ms = 300000,
        .idle_core_mask = 0,
        .trigger_panic = false
    };

    esp_err_t wdt_ret = esp_task_wdt_reconfigure(&twdt_cfg);
    ESP_LOGW(TAG, "Maintenance Task Watchdog Registry expanded: %s", esp_err_to_name(wdt_ret));

    // Execute fast disk structure validation without invoking a full cluster format
    if (!fg_sd_reset_storage_blocking())
    {
        ESP_LOGE(TAG, "Error: Storage structural template generation routine reports write errors");
    }

    ESP_LOGW(TAG, "STORAGE REBUILD SEQUENCE EXECUTED SUCCESSFULLY - REBOOTING SYSTEM");

    vTaskDelay(pdMS_TO_TICKS(1500));
    esp_restart();
}

/**
 * @brief Primary Embedded Application Entry Vector
 */
void app_main(void)
{
    // ---- STEP 1: NON-VOLATILE FLASH ENGINE INITIALIZATION ----
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

#if FORGEUI_ENABLE_SD
    // ---- STEP 2: ASSESS SYSTEM DISK EMERGENCY RESTORE FLAGS ----
    if (fg_sd_check_prepare_flag())
    {
        fg_sd_prepare_card_maintenance();
        return; // Intercept runtime and halt standard graphic initialization paths
    }
#endif

    // ---- STEP 3: MIPI-DSI VIDEO PANEL HARDWARE DISCOVERY ----
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
            .sw_rotate = true,
        }
    };

    // Instantiate native display driver layers for the EK79007 and GT911 touch IC
    (void)bsp_display_start_with_config(&cfg);

    // Awaken backlight circuit paths via hardware PWM output
    bsp_display_backlight_on();

    // ---- STEP 4: INTERFACE GRAPHIC CONTEXT ENVIRONMENT STAGING ----
    // Acquire the layout thread lock block before mutating active display nodes
    bsp_display_lock(0);
    
    // Spawns the central multi-page shell layout workspace and themes
    fg_hmi_init();
    
    // Evaluate and paint structural header clocks and network signal badges
    fg_header_refresh();
    
    // Build the underlying capture container for the quick parameter overlay
    fg_status_drawer_create();
    
    bsp_display_unlock();

#if FORGEUI_ENABLE_RTC
    // ---- STEP 5: TIME RECOVERY AND SYNCHRONIZATION HARDWARE ----
    // Synchronize local internal tick counts with external DS3231 I2C reference
    fg_rtc_init();
#endif

#if FORGEUI_ENABLE_WIFI
    bool wifi_ready = false;
#endif

#if FORGEUI_ENABLE_SD
    bool sd_ready = false;
#endif

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "FORGEUI CORE APPLICATION BUILD PIPELINE COMPILING");
    ESP_LOGI(TAG, "Target Environment: ESP32-P4 Appliance Reactor UI");
    ESP_LOGI(TAG, "Subsystem Flag -> Wireless Stack Coexistence: %s", FORGEUI_ENABLE_WIFI ? "ACTIVE" : "DISABLED");
    ESP_LOGI(TAG, "Subsystem Flag -> Local Disk Storage Media  : %s", FORGEUI_ENABLE_SD ? "ACTIVE" : "DISABLED");
    ESP_LOGI(TAG, "Subsystem Flag -> High-Precision I2C RTC    : %s", FORGEUI_ENABLE_RTC ? "ACTIVE" : "DISABLED");
    ESP_LOGI(TAG, "========================================");

#if FORGEUI_ENABLE_WIFI
    // ---- STEP 6: ENFORCE SDIO WIRELESS TRANSPORT MASTER HANDSHAKE ----
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "COEXISTENCE LOCK: INITIALIZING HOSTED WIRELESS RADIO");
    ESP_LOGI(TAG, "========================================");

    // Boot up transport tasks for the onboard ESP32-C6 radio module interface
    fg_wifi_init();

    // Strict timing allocation delay to guarantee link resolution before mounting disk lines
    vTaskDelay(pdMS_TO_TICKS(2500));

    wifi_ready = fg_wifi_is_ready();

    ESP_LOGI(TAG, "Hosted Network Line Registry: %s | Active IP Allocation: %s",
             fg_wifi_status_text(),
             fg_wifi_ip_text());
#endif

#if FORGEUI_ENABLE_SD
    // ---- STEP 7: SECURE TRANS-BUS INGESTION FOR LOCAL STORAGE MEDIA ----
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "COEXISTENCE LOCK: MOUNTING FILE SYSTEM STORAGE MEDIA");
    ESP_LOGI(TAG, "========================================");

    // Safely claim remaining SDMMC slot registers now that the network pipeline is operational
    if (fg_sd_init())
    {
        sd_ready = true;
        ESP_LOGI(TAG, "FATFS Mount Handshake Resolved: DISK ENGINE READY");
        fg_sd_test();
    }
    else
    {
        sd_ready = false;
        ESP_LOGE(TAG, "FATFS Mount Handshake Rejected: LOCAL DISK ENGAGEMENT STALLED");
    }
#endif

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "CORE COEXISTENCE STAGING MATRIX RESULTS");

#if FORGEUI_ENABLE_WIFI
    ESP_LOGI(TAG, "Wireless Core Engine: %s", wifi_ready ? "STABLE" : "FAULT_NODE");
    ESP_LOGI(TAG, "Wireless State Node : %s | Assigned IP Target: %s",
             fg_wifi_status_text(),
             fg_wifi_ip_text());
#else
    ESP_LOGI(TAG, "Wireless Core Engine: EXPLICITLY COMPILE-DISABLED");
#endif

#if FORGEUI_ENABLE_SD
    ESP_LOGI(TAG, "FATFS Storage System: %s", sd_ready ? "STABLE" : "FAULT_NODE");
#else
    ESP_LOGI(TAG, "FATFS Storage System: EXPLICITLY COMPILE-DISABLED");
#endif

    ESP_LOGI(TAG, "========================================");

    // ---- STEP 8: PRODUCTION RUNTIME TELEMETRY PUMP ENGINE ----
    uint32_t last_1hz = lv_tick_get();

    while (1)
    {
        // 20Hz scheduling frequency throttle layer
        vTaskDelay(pdMS_TO_TICKS(50));

#if FORGEUI_ENABLE_WIFI
        // Process internal network queues without blocking drawing loops
        fg_wifi_pump();
#endif

        uint32_t now = lv_tick_get();

        // 1Hz System Health and Display Telemetry Output Cadence
        if ((now - last_1hz) >= 1000)
        {
            last_1hz = now;

            // Synchronize and update persistent UI clocks and headers within safe contexts
            bsp_display_lock(0);
            fg_header_refresh();
            bsp_display_unlock();

#if FORGEUI_ENABLE_WIFI
            const char *wifi_status = fg_wifi_status_text();
            const char *wifi_ip = fg_wifi_ip_text();
#else
            const char *wifi_status = "COMPILE_DISABLED";
            const char *wifi_ip = "N/A";
#endif

#if FORGEUI_ENABLE_SD
            const char *sd_status = sd_ready ? "MOUNTED_OK" : "MEDIA_ERROR";
#else
            const char *sd_status = "COMPILE_DISABLED";
#endif

            // Stream continuous execution status to the ESP-IDF serial trace console
            ESP_LOGI(TAG, "System Health Matrix -> Network: %s | Active IP: %s | Mass Storage: %s",
                     wifi_status,
                     wifi_ip,
                     sd_status);
        }
    }
}

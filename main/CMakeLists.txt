# ============================================================
# ESP32-P4-WIFI6-Touch-LCD-7B Main Component Registration
# ============================================================
# ESP-IDF Component Build Engine Directives
#
# Target Hardware:
#   Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
#
# Core Framework Stack:
#   ForgeUI Architecture Engine (Reactor UI 3-Page Edition)
#   LVGL v9.2.2 Unified Graphics / ESP-Hosted Wireless Coexistence
# ============================================================

idf_component_register(
    SRCS
        # ---- Core Execution & Staging Orchestration ----
        "main.c"
        "02_FG_HMI.c"
        "05_FG_Icons.c"
        "16_UI_Style.c"

        # ---- Reactor UI 3-Page View Stack ----
        "10_UI_Dashboard.c"
        "11_UI_PreOp.c"
        "12_UI_System.c"
        "13_UI_Admin.c"

        # ---- Persistent Overlays, Keypads & Menus ----
        "14_UI_Header.c"
        "15_UI_Keyboard.c"
        "16_UI_AdminGate.c"
        "17_UI_StatusDrawer.c"
        "17_UI_ReactorModal.c"

        # ---- Peripheral Hardware Abstraction Backends ----
        "20_RTC.c"
        "30_Audio.c"
        "30_WIFI.c"
        "40_SD.c"

        # ---- Modular Asset Icon Management Pipeline ----
        "assets/icons/fg_icon_admin_48px.c"
        "assets/icons/fg_icon_brightness_48px.c"
        "assets/icons/fg_icon_sound_48px.c"
        "assets/icons/fg_icon_system_48px.c"
        "assets/icons/fg_icon_time_48px.c"
        "assets/icons/fg_icon_wifi_48px.c"
        "assets/icons/fg_icon_sd_card_48px.c"
        "assets/icons/fg_icon_home_28px.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        # ---- Native ESP-IDF Subsystem Requirements ----
        nvs_flash
        driver
        esp_event
        esp_netif
        esp_wifi
        fatfs
        sdmmc
        
        # ---- Coexistence Transports & Managed Nodes ----
        esp_wifi_remote
        esp_hosted
        
        # ---- Explicit Waveshare 7B Hardware BSP Targets ----
        waveshare__esp32_p4_wifi6_touch_lcd_7b

    PRIV_REQUIRES
        # ---- Isolated Auxiliary Driver Links ----
        bsp_extra
)

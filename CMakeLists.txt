# ============================================================
# ESP32-P4-WIFI6-Touch-LCD-7B LVGL Boilerplate (3-Page UI Edition)
# ============================================================
# ESP-IDF Project Root Configuration
#
# Target Hardware:
#   Waveshare ESP32-P4-WIFI6-Touch-LCD-7B
#
# Core Framework Stack:
#   ForgeUI Architecture Engine
#   ESP-IDF v5.5.4 Baseline / LVGL v9.2.2 
#
# Coprocessor Subsystems:
#   Hosted Wi-Fi 6 Transport over 4-bit SDIO (Onboard ESP32-C6)
# ============================================================

cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# ------------------------------------------------------------
# Global Compiler Warning Suppression Filters
# ------------------------------------------------------------

add_compile_options(-Wno-format)
add_compile_options(-Wno-ignored-qualifiers)
add_compile_options(-Wno-attributes)
add_compile_options(-Wno-unused-function)

# ------------------------------------------------------------
# Central Project Declaration Engine Target
# ------------------------------------------------------------

project(ESP32_P4_WIFI6_Touch_LCD_7B_LVGL_Boilerplate)

/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "driver/i2c.h"
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "lvgl.h"

#include "bsp/display.h"
#include "bsp/esp32_s3_lcd_ev_board.h"
#include "bsp_err_check.h"

#include "esp_lvgl_port.h"

static const char *TAG = "S3-LCD-BOARD";

static lv_display_t *disp;
static lv_indev_t *disp_indev = NULL;

static lv_display_t *bsp_display_lcd_init()
{
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_handle_t panel_handle = NULL;           // LCD panel handle

    bsp_display_config_t disp_config = { 0 };

    BSP_ERROR_CHECK_RETURN_NULL(bsp_display_new(&disp_config, &panel_handle, &io_handle));

    int buffer_size = 0;

    buffer_size = BSP_LCD_H_RES * BSP_LCD_V_RES;

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle    = io_handle,
        .panel_handle = panel_handle,
        .buffer_size  = buffer_size,

        .monochrome = false,
        .hres = BSP_LCD_V_RES, // Swap X & Y
        .vres = BSP_LCD_H_RES,

        //.hres = BSP_LCD_H_RES, // normal
        //.vres = BSP_LCD_V_RES,

        .color_format = LV_COLOR_FORMAT_RGB565,

        .rotation = {

            .swap_xy  = false,
            .mirror_x = false,
            .mirror_y = false,

        },
        .flags = {
            
            .sw_rotate = false,

            .buff_dma = false,
#if CONFIG_BSP_DISPLAY_LVGL_PSRAM
            .buff_spiram = false,
#endif
            .full_refresh = 1,
            .direct_mode = 0,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = false,
#endif
        }
    };
    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
//#if CONFIG_BSP_LCD_RGB_BOUNCE_BUFFER_MODE
//            .bb_mode = 1,
//#else
            .bb_mode = 0,
//#endif
            .avoid_tearing = true,
        }
    };


    return lvgl_port_add_disp_rgb(&disp_cfg, &rgb_cfg);
}


/**********************************************************************************************************
 *
 * Display Function
 *
 **********************************************************************************************************/
lv_display_t *bsp_display_start(void)
{
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG()
    };

    return bsp_display_start_with_config(&cfg);
}

lv_display_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg)
{
    BSP_ERROR_CHECK_RETURN_NULL(lvgl_port_init(&cfg->lvgl_port_cfg)); /* lvgl task, tick etc*/

    BSP_NULL_CHECK(disp = bsp_display_lcd_init(), NULL);

    return disp;
}

lv_indev_t *bsp_display_get_input_dev(void)
{
    return disp_indev;
}

esp_err_t bsp_display_brightness_init(void)
{
    ESP_LOGW(TAG, "This board doesn't support to change brightness of LCD");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_display_brightness_set(int brightness_percent)
{
    ESP_LOGW(TAG, "This board doesn't support to change brightness of LCD");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t bsp_display_backlight_off(void)
{
    return bsp_display_brightness_set(0);
}

esp_err_t bsp_display_backlight_on(void)
{
    return bsp_display_brightness_set(100);
}

void bsp_display_rotate(lv_display_t *disp, lv_display_rotation_t rotation)
{
    lv_disp_set_rotation(disp, rotation);
}

bool bsp_display_lock(uint32_t timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void bsp_display_unlock(void)
{
    lvgl_port_unlock();
}

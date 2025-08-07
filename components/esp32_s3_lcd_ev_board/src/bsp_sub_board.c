/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_check.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_log.h"

#include "sdkconfig.h"
#include "bsp_err_check.h"
#include "bsp/display.h"
#include "bsp/esp32_s3_lcd_ev_board.h"

#include "st7701.h"
#include "esp_lcd_ssd1963.h"

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 1, 2)
#warning "Due to significant updates of the RGB LCD drivers, it's recommended to develop using ESP-IDF v5.1.2 or later"
#endif

#if CONFIG_ESP32S3_DATA_CACHE_LINE_64B && !(CONFIG_SPIRAM_SPEED_120M || CONFIG_BSP_LCD_RGB_BOUNCE_BUFFER_MODE)
#warning "Enabling the `ESP32S3_DATA_CACHE_LINE_64B` configuration when the PSRAM speed is not set to 120MHz (`SPIRAM_SPEED_120M`) and the LCD is not in bounce buffer mode (`BSP_LCD_RGB_BOUNCE_BUFFER_MODE`) may result in screen drift, please enable `ESP32S3_DATA_CACHE_LINE_32B` instead"
#endif

static const char *TAG = "bsp_sub_board";


void example_init_i80_bus(esp_lcd_panel_io_handle_t *io_handle)
{
    ESP_LOGI(TAG, "Initialize Intel 8080 bus");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = GPIO_LCD4_PIN_NUM_DC,
        .wr_gpio_num = GPIO_LCD4_PIN_NUM_WR,
        .data_gpio_nums = {
            GPIO_LCD4_PIN_NUM_DATA0,
            GPIO_LCD4_PIN_NUM_DATA1,
            GPIO_LCD4_PIN_NUM_DATA2,
            GPIO_LCD4_PIN_NUM_DATA3,
            GPIO_LCD4_PIN_NUM_DATA4,
            GPIO_LCD4_PIN_NUM_DATA5,
            GPIO_LCD4_PIN_NUM_DATA6,
            GPIO_LCD4_PIN_NUM_DATA7,
            GPIO_LCD4_PIN_NUM_DATA8,
            GPIO_LCD4_PIN_NUM_DATA9,
            GPIO_LCD4_PIN_NUM_DATA10,
            GPIO_LCD4_PIN_NUM_DATA11,
            GPIO_LCD4_PIN_NUM_DATA12,
            GPIO_LCD4_PIN_NUM_DATA13,
            GPIO_LCD4_PIN_NUM_DATA14,
            GPIO_LCD4_PIN_NUM_DATA15,
        },
        .bus_width = 16,//8,//GPIO_LCD4_LCD_I80_BUS_WIDTH,
        .max_transfer_bytes = BSP_LCD_H_RES * 100 * sizeof(uint16_t),
        .dma_burst_size = GPIO_LCD4_DMA_BURST_SIZE,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = GPIO_LCD4_PIN_NUM_CS,
        .pclk_hz = GPIO_LCD4_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth  = 10,
        .dc_levels = {
            .dc_idle_level  = 0,
            .dc_cmd_level   = 0,
            .dc_dummy_level = 0,
            .dc_data_level  = 1,
        },
        .lcd_cmd_bits   = 16,//8, // GPIO_LCD4_LCD_CMD_BITS,
        .lcd_param_bits = 8, // GPIO_LCD4_LCD_PARAM_BITS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, io_handle));
}


/**************************************************************************************************
 *
 * Display Panel Function
 *
 **************************************************************************************************/

esp_err_t bsp_display_new(const bsp_display_config_t *config, esp_lcd_panel_handle_t *ret_panel, esp_lcd_panel_io_handle_t *ret_io)
{
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 1, 2)
    ESP_LOGW(TAG, "Due to significant updates of the RGB LCD drivers, it's recommended to develop using ESP-IDF v5.1.2 or later");
#endif

#if CONFIG_ESP32S3_DATA_CACHE_LINE_64B && !(CONFIG_SPIRAM_SPEED_120M || CONFIG_BSP_LCD_RGB_BOUNCE_BUFFER_MODE)
    ESP_LOGW(TAG, "Enabling the `ESP32S3_DATA_CACHE_LINE_64B` configuration when the PSRAM speed is not set to 120MHz \
(`SPIRAM_SPEED_120M`) and the LCD is not in bounce buffer mode (`BSP_LCD_RGB_BOUNCE_BUFFER_MODE`) may result in screen \
drift, please enable `ESP32S3_DATA_CACHE_LINE_32B` instead");
#endif

    ESP_LOGI(TAG, "Initialize I80 panel (NEW LCD)");

    ESP_LOGI(TAG, "st7701_reg_init");
    st7701_reg_init();     // st7701 register config

    esp_lcd_panel_io_handle_t io_handle = NULL;
    example_init_i80_bus(&io_handle);

    esp_lcd_panel_handle_t panel_handle = NULL;

    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_LCD4_PIN_NUM_RD
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    ESP_LOGI(TAG, "GPIO_LCD4_PIN_NUM_RD=1 always...");
    gpio_set_level(GPIO_LCD4_PIN_NUM_RD, 1);


//    BSP_ERROR_CHECK_RETURN_ERR(esp_lcd_panel_init(panel_handle));

    ESP_LOGI(TAG, "Install SSD1963 panel driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1963(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
	
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    if (ret_panel) {
        *ret_panel = panel_handle;
    }
    if (ret_io) {
        *ret_io = io_handle;
    }

    return ESP_OK;
}

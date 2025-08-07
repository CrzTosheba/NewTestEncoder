/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ESP BSP: S3-LCD-EV board
 */

#pragma once

#include "driver/gpio.h"
#include "esp_err.h"
#include "lvgl.h"
#include "bsp/display.h"
#include "esp_lvgl_port.h"

#include "sdkconfig.h"
/**************************************************************************************************
 *  BSP Capabilities
 **************************************************************************************************/

/**************************************************************************************************
 *  ESP32-S3-LCD-EV-Board Pinout
 **************************************************************************************************/

// New LCD ----------------------------------------------------

// GPIO inteface ESP -> LCD
#define GPIO_LCD4_RST    (GPIO_NUM_NC)
#define GPIO_LCD4_CS     (GPIO_NUM_15)

#define GPIO_LCD4_SDA    (GPIO_NUM_6)
#define GPIO_LCD4_SCK    (GPIO_NUM_7)


//#define GPIO_LCD4_LCD_PIXEL_CLOCK_HZ     (10000000)
#define GPIO_LCD4_LCD_PIXEL_CLOCK_HZ     (5000000)

#define GPIO_LCD4_DMA_BURST_SIZE         (64) // 16, 32, 64. Higher burst size can improve the performance when the DMA buffer comes from PSRAM

// GPIO interface ESP(I80) -> LCD
#define GPIO_LCD4_PIN_NUM_RD       (GPIO_NUM_10) // RD

#define GPIO_LCD4_PIN_NUM_CS       (GPIO_NUM_17)
#define GPIO_LCD4_PIN_NUM_DC       (GPIO_NUM_46)
#define GPIO_LCD4_PIN_NUM_WR       (GPIO_NUM_3) // wr

#define GPIO_LCD4_PIN_NUM_DATA0    (GPIO_NUM_9)
#define GPIO_LCD4_PIN_NUM_DATA1    (GPIO_NUM_12)
#define GPIO_LCD4_PIN_NUM_DATA2    (GPIO_NUM_11)
#define GPIO_LCD4_PIN_NUM_DATA3    (GPIO_NUM_14)
#define GPIO_LCD4_PIN_NUM_DATA4    (GPIO_NUM_13)
#define GPIO_LCD4_PIN_NUM_DATA5    (GPIO_NUM_47)
#define GPIO_LCD4_PIN_NUM_DATA6    (GPIO_NUM_21)
#define GPIO_LCD4_PIN_NUM_DATA7    (GPIO_NUM_45)
#define GPIO_LCD4_PIN_NUM_DATA8    (GPIO_NUM_48)
#define GPIO_LCD4_PIN_NUM_DATA9    (GPIO_NUM_38)
#define GPIO_LCD4_PIN_NUM_DATA10   (GPIO_NUM_39)
#define GPIO_LCD4_PIN_NUM_DATA11   (GPIO_NUM_40)
#define GPIO_LCD4_PIN_NUM_DATA12   (GPIO_NUM_41)
#define GPIO_LCD4_PIN_NUM_DATA13   (GPIO_NUM_42)
#define GPIO_LCD4_PIN_NUM_DATA14   (GPIO_NUM_2)
#define GPIO_LCD4_PIN_NUM_DATA15   (GPIO_NUM_1)

// Rotary encoder pins
#define GPIO_ROT_ENC_A     (GPIO_NUM_16)
#define GPIO_ROT_ENC_B     (GPIO_NUM_8)
#define GPIO_ROT_ENC_SW    (GPIO_NUM_4)


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BSP display configuration structure
 *
 */
typedef struct {
    lvgl_port_cfg_t lvgl_port_cfg;
} bsp_display_cfg_t;

#include "lcd.h"


/**
 * @brief Initialize display
 *
 * @note This function initializes display controller and starts LVGL handling task.
 * @note Users can get LCD panel handle from `user_data` in returned display.
 *
 * @return Pointer to LVGL display or NULL when error occurred
 */
lv_display_t *bsp_display_start(void);

/**
 * @brief Initialize display
 *
 * This function initializes SPI, display controller and starts LVGL handling task.
 * LCD backlight must be enabled separately by calling `bsp_display_brightness_set()`
 *
 * @param cfg display configuration
 *
 * @return Pointer to LVGL display or NULL when error occurred
 */
lv_display_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg);

/**
 * @brief Get pointer to input device (touch, buttons, ...)
 *
 * @note  The LVGL input device is initialized in `bsp_display_start()` function.
 * @note  This function should be called after calling `bsp_display_start()`.
 *
 * @return Pointer to LVGL input device or NULL when not initialized
 */
lv_indev_t *bsp_display_get_input_dev(void);

/**
 * @brief Take LVGL mutex
 *
 * @note  Display must be already initialized by calling `bsp_display_start()`
 *
 * @param[in] timeout_ms: Timeout in [ms]. 0 will block indefinitely.
 *
 * @return
 *      - true:  Mutex was taken
 *      - false: Mutex was NOT taken
 */
bool bsp_display_lock(uint32_t timeout_ms);

/**
 * @brief Give LVGL mutex
 *
 * @note  Display must be already initialized by calling `bsp_display_start()`
 *
 */
void bsp_display_unlock(void);

/**
 * @brief Rotate screen
 *
 * @note  Display must be already initialized by calling `bsp_display_start()`
 * @note  This function can't work with the anti-tearing function. Please use the `BSP_DISPLAY_LVGL_ROTATION` configuration instead.
 *
 * @param[in] disp:     Pointer to LVGL display
 * @param[in] rotation: Angle of the display rotation
 */
void bsp_display_rotate(lv_display_t *disp, lv_display_rotation_t rotation);

/**
 * @brief Get display horizontal resolution
 *
 * @note  This function should be called after calling `bsp_display_new()` or `bsp_display_start()`
 *
 * @return Horizontal resolution. Return 0 if error occurred.
 */
uint16_t bsp_display_get_h_res(void);

/**
 * @brief Get display vertical resolution
 *
 * @note  This function should be called after calling `bsp_display_new()` or `bsp_display_start()`
 *
 * @return Vertical resolution. Return 0 if error occurred.
 */
uint16_t bsp_display_get_v_res(void);


#ifdef __cplusplus
}
#endif

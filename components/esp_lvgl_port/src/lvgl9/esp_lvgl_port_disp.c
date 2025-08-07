/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"
#include "esp_lvgl_port_priv.h"
#include "../../../esp32_s3_lcd_ev_board/include/bsp/lcd.h"
#include "esp_lcd_panel_rgb.h"

#define LVGL_PORT_HANDLE_FLUSH_READY 1

static const char *TAG = "LVGL PORT";
/*******************************************************************************
* Types definitions
*******************************************************************************/

typedef struct {
    lvgl_port_disp_type_t     disp_type;    /* Display type */
    esp_lcd_panel_io_handle_t io_handle;      /* LCD panel IO handle */
    esp_lcd_panel_handle_t    panel_handle;   /* LCD panel handle */
    esp_lcd_panel_handle_t    control_handle; /* LCD panel control handle */
    lvgl_port_rotation_cfg_t  rotation;       /* Default values of the screen rotation */
    lv_color_t                *draw_buffs[3]; /* Display draw buffers */
    lv_display_t              *disp_drv;      /* LVGL display driver */
    lv_display_rotation_t     current_rotation;
    struct {
        unsigned int monochrome: 1;  /* True, if display is monochrome and using 1bit for 1px */
        unsigned int swap_bytes: 1;  /* Swap bytes in RGB656 (16-bit) before send to LCD driver */
        unsigned int full_refresh: 1;   /* Always make the whole screen redrawn */
        unsigned int direct_mode: 1;    /* Use screen-sized buffers and draw to absolute coordinates */
        unsigned int sw_rotate: 1;    /* Use software rotation (slower) or PPA if available */
    } flags;
} lvgl_port_display_ctx_t;

/*******************************************************************************
* Function definitions
*******************************************************************************/
static lv_display_t *lvgl_port_add_disp_priv(const lvgl_port_display_cfg_t *disp_cfg, const lvgl_port_disp_priv_cfg_t *priv_cfg);
#if LVGL_PORT_HANDLE_FLUSH_READY
//static bool lvgl_port_flush_io_ready_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
#if CONFIG_IDF_TARGET_ESP32S3 && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
//static bool lvgl_port_flush_vsync_ready_callback(esp_lcd_panel_handle_t panel_io, const esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx);
#endif
#endif

//static bool lvgl_port_flush_vsync_ready_callback(esp_lcd_panel_handle_t panel_io, const esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx);
static void lvgl_port_flush_callback(lv_display_t *drv, const lv_area_t *area, uint8_t *color_map);

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_display_t *display = (lv_display_t *)user_ctx;
    lv_display_flush_ready(display);
    return false;
}




/*******************************************************************************
* Public API functions
*******************************************************************************/
/*
lv_display_t *lvgl_port_add_disp(const lvgl_port_display_cfg_t *disp_cfg)
{

    ESP_LOGI(TAG, "11111111111111111111111111111111");

    lvgl_port_lock(0);
    lv_disp_t *disp = lvgl_port_add_disp_priv(disp_cfg, NULL);

    if (disp != NULL) {
        lvgl_port_display_ctx_t *disp_ctx = (lvgl_port_display_ctx_t *)lv_display_get_user_data(disp);
        // Set display type
        disp_ctx->disp_type = LVGL_PORT_DISP_TYPE_OTHER;

        assert(disp_cfg->io_handle != NULL);

        const esp_lcd_panel_io_callbacks_t cbs = {
            .on_color_trans_done = lvgl_port_flush_io_ready_callback,
        };
        // Register done callback
        esp_lcd_panel_io_register_event_callbacks(disp_ctx->io_handle, &cbs, disp);

    }
    lvgl_port_unlock();

    return disp;
}
*/

lv_display_t *lvgl_port_add_disp_rgb(const lvgl_port_display_cfg_t *disp_cfg, const lvgl_port_display_rgb_cfg_t *rgb_cfg)
{

    lvgl_port_lock(0);
    assert(rgb_cfg != NULL);
    const lvgl_port_disp_priv_cfg_t priv_cfg = {
        .avoid_tearing = rgb_cfg->flags.avoid_tearing,
    };
    lv_disp_t *disp = lvgl_port_add_disp_priv(disp_cfg, &priv_cfg);

    //if (disp != NULL) {
    //    lvgl_port_display_ctx_t *disp_ctx = (lvgl_port_display_ctx_t *)lv_display_get_user_data(disp);
    //}
    lvgl_port_unlock();

    return disp;
}

esp_err_t lvgl_port_remove_disp(lv_display_t *disp)
{
    assert(disp);
    lvgl_port_display_ctx_t *disp_ctx = (lvgl_port_display_ctx_t *)lv_display_get_user_data(disp);

    lvgl_port_lock(0);
    lv_disp_remove(disp);
    lvgl_port_unlock();

    if (disp_ctx->draw_buffs[0]) {
        free(disp_ctx->draw_buffs[0]);
    }

    if (disp_ctx->draw_buffs[1]) {
        free(disp_ctx->draw_buffs[1]);
    }

    if (disp_ctx->draw_buffs[2]) {
        free(disp_ctx->draw_buffs[2]);
    }

    free(disp_ctx);

    return ESP_OK;
}

void lvgl_port_flush_ready(lv_display_t *disp)
{
    assert(disp);
    lv_disp_flush_ready(disp);
}

/*******************************************************************************
* Private functions
*******************************************************************************/

static lv_display_t *lvgl_port_add_disp_priv(const lvgl_port_display_cfg_t *disp_cfg, const lvgl_port_disp_priv_cfg_t *priv_cfg)
{
    esp_err_t ret = ESP_OK;
    lv_display_t *disp = NULL;
    lv_color_t *buf1 = NULL;
    lv_color_t *buf2 = NULL;
    assert(disp_cfg != NULL);
    assert(disp_cfg->panel_handle != NULL);
    assert(disp_cfg->buffer_size > 0);
    assert(disp_cfg->hres > 0);
    assert(disp_cfg->vres > 0);

    lv_color_format_t display_color_format = disp_cfg->color_format;
    if (disp_cfg->flags.swap_bytes) {
        /* Swap bytes can be used only in RGB656 color format */
        ESP_RETURN_ON_FALSE(display_color_format == LV_COLOR_FORMAT_RGB565, NULL, TAG, "Swap bytes can be used only in display color format RGB565!");
    }

    if (disp_cfg->flags.buff_dma) {
        /* DMA buffer can be used only in RGB656 color format */
        ESP_RETURN_ON_FALSE(display_color_format == LV_COLOR_FORMAT_RGB565, NULL, TAG, "DMA buffer can be used only in display color format RGB565 (not alligned copy)!");
    }

    /* Display context */
    lvgl_port_display_ctx_t *disp_ctx = malloc(sizeof(lvgl_port_display_ctx_t));
    ESP_GOTO_ON_FALSE(disp_ctx, ESP_ERR_NO_MEM, err, TAG, "Not enough memory for display context allocation!");
    memset(disp_ctx, 0, sizeof(lvgl_port_display_ctx_t));
    disp_ctx->io_handle         = disp_cfg->io_handle;
    disp_ctx->panel_handle      = disp_cfg->panel_handle;
    disp_ctx->control_handle    = disp_cfg->control_handle;
    disp_ctx->rotation.swap_xy  = disp_cfg->rotation.swap_xy;
    disp_ctx->rotation.mirror_x = disp_cfg->rotation.mirror_x;
    disp_ctx->rotation.mirror_y = disp_cfg->rotation.mirror_y;
    disp_ctx->flags.swap_bytes  = disp_cfg->flags.swap_bytes;
    disp_ctx->flags.sw_rotate   = disp_cfg->flags.sw_rotate;
    disp_ctx->current_rotation  = LV_DISPLAY_ROTATION_0;

    uint32_t buff_caps = 0;
#if SOC_PSRAM_DMA_CAPABLE == 0
    if (disp_cfg->flags.buff_dma && disp_cfg->flags.buff_spiram) {
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "Alloc DMA capable buffer in SPIRAM is not supported!");
    }
#endif
    if (disp_cfg->flags.buff_dma) {
        buff_caps |= MALLOC_CAP_DMA;
    }
    if (disp_cfg->flags.buff_spiram) {
        buff_caps |= MALLOC_CAP_SPIRAM;
    }
    if (buff_caps == 0) {
        buff_caps |= MALLOC_CAP_DEFAULT;
    }

    // it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
//    size_t draw_buffer_sz = EXAMPLE_LCD_H_RES * EXAMPLE_LVGL_DRAW_BUF_LINES * sizeof(lv_color16_t);    // alloc draw buffers used by LVGL
    size_t draw_buffer_sz = BSP_LCD_H_RES * 100 * sizeof(lv_color16_t);

    uint32_t draw_buf_alloc_caps = 0;

//#if CONFIG_EXAMPLE_LCD_I80_COLOR_IN_PSRAM
    draw_buf_alloc_caps |= MALLOC_CAP_SPIRAM;
//#endif

    buf1 = esp_lcd_i80_alloc_draw_buffer(disp_cfg->io_handle, draw_buffer_sz, draw_buf_alloc_caps);
    buf2 = esp_lcd_i80_alloc_draw_buffer(disp_cfg->io_handle, draw_buffer_sz, draw_buf_alloc_caps);

    assert(buf1);
    assert(buf2);

    disp = lv_display_create(disp_cfg->hres, disp_cfg->vres);

//    disp_ctx->flags.full_refresh = 1;
    lv_display_set_buffers(disp, buf1, buf2, draw_buffer_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);//LV_DISPLAY_RENDER_MODE_FULL);

    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);//display_color_format);
    lv_display_set_flush_cb(disp, lvgl_port_flush_callback);

    lv_display_set_user_data(disp, disp_ctx);
    disp_ctx->disp_drv = disp;


    ESP_LOGI(TAG, "Register io panel event callback for LVGL flush ready notification");
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = example_notify_lvgl_flush_ready,
    };
    /* Register done callback */
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(disp_cfg->io_handle, &cbs, disp));

    return disp;

err:
    if (ret != ESP_OK) {
        if (buf1) {
            free(buf1);
        }
        if (buf2) {
            free(buf2);
        }
        if (disp_ctx->draw_buffs[2]) {
            free(disp_ctx->draw_buffs[2]);
        }
        if (disp_ctx) {
            free(disp_ctx);
        }
    }

    return disp;
}

#if LVGL_PORT_HANDLE_FLUSH_READY
/*
static bool lvgl_port_flush_io_ready_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_display_t *disp_drv = (lv_display_t *)user_ctx;
    assert(disp_drv != NULL);
    lv_disp_flush_ready(disp_drv);
    return false;
}


static bool lvgl_port_flush_vsync_ready_callback(esp_lcd_panel_handle_t panel_io, const esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx)
{
    BaseType_t need_yield = pdFALSE;
    lv_display_t *disp_drv = (lv_display_t *)user_ctx;
    assert(disp_drv != NULL);
    need_yield = lvgl_port_task_notify(ULONG_MAX);
    lvgl_port_task_wake(LVGL_PORT_EVENT_DISPLAY, disp_drv);

    return (need_yield == pdTRUE);
}
*/
#endif

static void lvgl_port_flush_callback(lv_display_t *display, const lv_area_t *area, uint8_t *color_map)
{
    lvgl_port_display_ctx_t *disp_ctx = (lvgl_port_display_ctx_t *)lv_display_get_user_data(display);
    esp_lcd_panel_handle_t panel_handle = disp_ctx->panel_handle;

    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}


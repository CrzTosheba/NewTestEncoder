#include "CO_alarms_menu.h"
#include "CO_dry_run_menu.h"
#include "CO_ext_alarm_menu.h"
#include "CO_sensor_break_menu.h"
#include "CO_dev_alarm_menu.h"
#include "encoder/encoder.h"
#include "encoder/encoder_manager.h"
#include "my_widgets/w_rad_mask.h"
#include "screen_logic/arc_menu.h"
#include "screen_logic/menu_config.h"
#include "screen_logic/screen_navigation.h"
#include "screen_logic/screen_container_manager.h"
#include "screen_logic/access_control.h"
#include <stdint.h>
#include <inttypes.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CO_ALARMS_MENU";

// Структура элемента меню аварий
typedef struct {
    const char *label_text;     // Текст элемента
    const void *img_src;        // Иконка (только для "Назад")
} CoAlarmsMenuItem;

// Элементы меню аварий
static const CoAlarmsMenuItem co_alarms_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right},
    {"Сухой ход", NULL},
    {"Внешняя авария", NULL},
    {"Обрыв датчика", NULL},
    {"Авар. отклонение", NULL},
};

// Локальные переменные для меню аварий
lv_obj_t *co_alarms_cont = NULL;
static bool co_alarms_menu_initialized = false;
static bool co_alarms_menu_creation_in_progress = false;
static lv_obj_t *co_alarms_mask = NULL;

/**
 * @brief Проверяет, является ли объект валидным
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Создание элемента меню аварий
 */
static void create_co_alarms_menu_item(lv_obj_t *cont, const CoAlarmsMenuItem *item) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_co_alarms_menu_item");
        return;
    }
    
    // Создаем контейнер для элемента (высота 40px)
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_co_alarms_menu_item");
        return;
    }
    
    lv_obj_set_size(box, 462, 40);
    lv_obj_set_style_border_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_radius(box, 0, 0);
    
    // Основная надпись
    lv_obj_t *label = lv_label_create(box);
    if (is_obj_valid(label)) {
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
        lv_label_set_text(label, item->label_text);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);
    }
    
    // Иконка (только для "Назад")
    if (item->img_src != NULL) {
        lv_obj_t *img = lv_img_create(box);
        if (is_obj_valid(img)) {
            lv_img_set_src(img, item->img_src);
            lv_obj_set_style_img_recolor(img, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
            lv_obj_align(img, LV_ALIGN_CENTER, 90, 0);
        }
    }
    
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);
    vTaskDelay(pdMS_TO_TICKS(1));
}

/**
 * @brief Подсветка выбранного элемента меню аварий
 */
static void co_alarms_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
    if (!is_obj_valid(cont)) return;
    
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!is_obj_valid(child)) continue;
        
        uint32_t grand_child_cnt = lv_obj_get_child_cnt(child);
        
        for (uint32_t j = 0; j < grand_child_cnt; j++) {
            lv_obj_t *grand_child = lv_obj_get_child(child, j);
            if (!is_obj_valid(grand_child)) continue;
            
            if (lv_obj_check_type(grand_child, &lv_label_class)) {
                if (i == cursor_index) {
                    lv_obj_set_style_text_color(grand_child, lv_color_hex(0x000000), LV_PART_MAIN);
                } else {
                    lv_obj_set_style_text_color(grand_child, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                }
            } else if (lv_obj_check_type(grand_child, &lv_image_class)) {
                if (i == cursor_index) {
                    lv_obj_set_style_img_recolor(grand_child, lv_color_hex(0x000000), 0);
                } else {
                    lv_obj_set_style_img_recolor(grand_child, lv_color_hex(0xFFFFFF), 0);
                }
            }
        }
        
        // Меняем фон контейнера
        if (i == cursor_index) {
            lv_obj_set_style_bg_color(child, lv_color_hex(0xFFCC00), LV_PART_MAIN);
        } else {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x2B3639), LV_PART_MAIN);
        }
    }
}

/**
 * @brief Показывает меню аварий
 */
void co_alarms_menu_show(void) {
    ESP_LOGI(TAG, "Showing CO alarms menu");
    if (is_obj_valid(co_alarms_cont)) {
        lv_obj_clear_flag(co_alarms_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(co_alarms_mask)) {
        co_alarms_mask = radial();
        if (is_obj_valid(co_alarms_mask)) {
            lv_obj_set_pos(co_alarms_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(co_alarms_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает меню аварий
 */
void co_alarms_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding CO alarms menu");
    if (is_obj_valid(co_alarms_cont)) {
        lv_obj_add_flag(co_alarms_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(co_alarms_mask)) {
        lv_obj_add_flag(co_alarms_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Обработчик событий энкодера для меню аварий
 */
void co_alarms_menu_encoder_event_cb(uint8_t e) {
    // Обновляем таймер активности при любом действии пользователя
    access_control_update_activity_timer();
    
    if (!is_obj_valid(co_alarms_cont)) {
        ESP_LOGE(TAG, "Контейнер меню аварий не инициализирован");
        return;
    }
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_ALARMS);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, co_alarms_cont, menu_state, MENU_TYPE_CO_ALARMS);
    
    if (prev_cursor != menu_state->cursor_index) {
        co_alarms_highlight_box(co_alarms_cont, menu_state->cursor_index);
    }
    
    // Обработка нажатия кнопки
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в меню отопления
            ESP_LOGI(TAG, "Returning to CO menu from alarms menu");
            co_alarms_menu_hide();
            extern void co_menu_show(void);
            co_menu_show();
            // Переключаем обработчик энкодера
            extern void co_menu_encoder_event_cb(uint8_t e);
            encoder_manager_register_callback(co_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 1) {
            // Нажали на "Сухой ход" - открываем подменю сухого хода
            ESP_LOGI(TAG, "Opening dry run menu");
            co_alarms_menu_hide();
            CO_Dry_Run_Menu_List();
            co_dry_run_menu_show();
            encoder_manager_register_callback(co_dry_run_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 2) {
            // Нажали на "Внешняя авария" - открываем подменю внешней аварии
            ESP_LOGI(TAG, "Opening external alarm menu");
            co_alarms_menu_hide();
            CO_Ext_Alarm_Menu_List();
            co_ext_alarm_menu_show();
            encoder_manager_register_callback(co_ext_alarm_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 3) {
            // Нажали на "Обрыв датчика" - открываем подменю обрыва датчика
            ESP_LOGI(TAG, "Opening sensor break menu");
            co_alarms_menu_hide();
            CO_Sensor_Break_Menu_List();
            co_sensor_break_menu_show();
            encoder_manager_register_callback(co_sensor_break_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 4) {
            // Нажали на "Авар. отклонение" - открываем подменю аварийного отклонения
            ESP_LOGI(TAG, "Opening deviation alarm menu");
            co_alarms_menu_hide();
            CO_Dev_Alarm_Menu_List();
            co_dev_alarm_menu_show();
            encoder_manager_register_callback(co_dev_alarm_menu_encoder_event_cb);
        }
    }
}

/**
 * @brief Очистка меню аварий
 */
void co_alarms_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up CO alarms menu");
    
    co_alarms_menu_creation_in_progress = false;
    
    if (is_obj_valid(co_alarms_mask)) {
        lv_obj_del(co_alarms_mask);
        co_alarms_mask = NULL;
    }
    
    if (is_obj_valid(co_alarms_cont)) {
        lv_obj_del(co_alarms_cont);
        co_alarms_cont = NULL;
    }
    
    co_alarms_menu_initialized = false;
}

/**
 * @brief Инициализация меню аварий
 */
void CO_Alarms_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню аварий");
    
    if (co_alarms_menu_creation_in_progress) {
        ESP_LOGW(TAG, "CO alarms menu creation already in progress, skipping");
        return;
    }
    
    co_alarms_menu_creation_in_progress = true;
    
    if (co_alarms_menu_initialized && is_obj_valid(co_alarms_cont)) {
        ESP_LOGI(TAG, "CO alarms menu already initialized, showing it");
        co_alarms_menu_show();
        co_alarms_menu_creation_in_progress = false;
        return;
    }
    
    co_alarms_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    co_alarms_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(co_alarms_cont)) {
        ESP_LOGE(TAG, "Failed to create CO alarms menu container");
        co_alarms_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(co_alarms_cont, 1200, 1200);
    lv_obj_center(co_alarms_cont);
    lv_obj_add_event_cb(co_alarms_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(co_alarms_cont, &style, 0);
    lv_obj_set_style_radius(co_alarms_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(co_alarms_cont, true, 0);
    lv_obj_set_scroll_dir(co_alarms_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(co_alarms_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(co_alarms_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(co_alarms_cont, 633, 0);
    lv_obj_set_style_bg_color(co_alarms_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(co_alarms_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(co_alarms_cont, 0, 0);
    lv_obj_set_style_pad_row(co_alarms_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(co_alarms_menu_items) / sizeof(CoAlarmsMenuItem); i++) {
        create_co_alarms_menu_item(co_alarms_cont, &co_alarms_menu_items[i]);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    co_alarms_mask = radial();
    if (is_obj_valid(co_alarms_mask)) {
        lv_obj_set_pos(co_alarms_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_CO_ALARMS);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_ALARMS);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(co_alarms_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    co_alarms_highlight_box(co_alarms_cont, menu_state->cursor_index);
    arc_menu_update_slide(co_alarms_cont);
    
    co_alarms_menu_initialized = true;
    co_alarms_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню аварий успешно инициализировано");
}


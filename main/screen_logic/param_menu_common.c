#include "param_menu_common.h"
#include "screen_container_manager.h"
#include "esp_log.h"

static const char *TAG = "PARAM_MENU_COMMON";

/**
 * @brief Проверяет, является ли объект валидным
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Создает label для названия параметра с унифицированными настройками
 */
lv_obj_t* param_menu_create_label(lv_obj_t *parent, const char *text) {
    if (!is_obj_valid(parent)) {
        ESP_LOGE(TAG, "Invalid parent in param_menu_create_label");
        return NULL;
    }
    
    lv_obj_t *label = lv_label_create(parent);
    if (!is_obj_valid(label)) {
        ESP_LOGE(TAG, "Failed to create label in param_menu_create_label");
        return NULL;
    }
    
    lv_obj_set_style_text_color(label, lv_color_hex(PARAM_TEXT_COLOR_DEFAULT), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, PARAM_LABEL_ALIGN_X, 0);
    
    return label;
}

/**
 * @brief Создает контейнер и label для значения параметра с унифицированными настройками
 */
lv_obj_t* param_menu_create_value_container(lv_obj_t *parent, lv_obj_t **value_label_ptr) {
    if (!is_obj_valid(parent)) {
        ESP_LOGE(TAG, "Invalid parent in param_menu_create_value_container");
        return NULL;
    }
    
    // Создаем контейнер для значения параметра
    lv_obj_t *value_container = lv_obj_create(parent);
    if (!is_obj_valid(value_container)) {
        ESP_LOGE(TAG, "Failed to create value container in param_menu_create_value_container");
        return NULL;
    }
    
    lv_obj_set_size(value_container, PARAM_VALUE_CONTAINER_WIDTH, PARAM_VALUE_CONTAINER_HEIGHT);
    lv_obj_set_style_bg_color(value_container, lv_color_hex(PARAM_BG_COLOR_DEFAULT), LV_PART_MAIN);
    lv_obj_set_style_border_color(value_container, lv_color_hex(PARAM_BG_COLOR_DEFAULT), LV_PART_MAIN);
    lv_obj_set_style_radius(value_container, 0, 0);
    lv_obj_set_style_pad_all(value_container, 0, 0);
    lv_obj_set_pos(value_container, PARAM_VALUE_CONTAINER_X, PARAM_VALUE_CONTAINER_Y);
    
    // Помечаем контейнер значения параметра для компенсации движения по дуге
    set_as_param_value(value_container);
    
    // Создаем label для значения
    lv_obj_t *value_label = lv_label_create(value_container);
    if (!is_obj_valid(value_label)) {
        ESP_LOGE(TAG, "Failed to create value label in param_menu_create_value_container");
        lv_obj_del(value_container);
        return NULL;
    }
    
    lv_obj_set_style_text_color(value_label, lv_color_hex(PARAM_TEXT_COLOR_DEFAULT), LV_PART_MAIN);
    lv_obj_set_style_text_font(value_label, &Roboto_bold_24, 0);
    lv_obj_align(value_label, PARAM_VALUE_LABEL_ALIGN, 0, 0);
    
    // Сохраняем указатель на label, если передан
    if (value_label_ptr != NULL) {
        *value_label_ptr = value_label;
    }
    
    return value_container;
}


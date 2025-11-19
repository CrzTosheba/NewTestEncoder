#include "screen_container_manager.h"
#include "esp_log.h"

static const char *TAG = "CONTAINER_MGR";

/**
 * @brief Создает контейнер для контента с заданным типом
 */
lv_obj_t* screen_container_create(container_type_t type) {
    ESP_LOGI(TAG, "Creating container of type: %d", type);
    
    lv_obj_t *container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(container, CONTENT_CONTAINER_WIDTH, CONTENT_CONTAINER_HEIGHT);
    lv_obj_set_pos(container, CONTENT_CONTAINER_X, CONTENT_CONTAINER_Y);
    
    // Базовые стили для всех контейнеров
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);
    
    // Стили в зависимости от типа контейнера
    switch(type) {
        case CONTAINER_TYPE_MAIN_MENU:
        case CONTAINER_TYPE_IN_OUT:
            lv_obj_set_style_bg_color(container, lv_color_hex(0x1e2528), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(container, LV_OPA_COVER, LV_PART_MAIN);
            break;
            
        case CONTAINER_TYPE_PASSWORD:
            lv_obj_set_style_bg_color(container, lv_color_hex(0x2a2a2a), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(container, LV_OPA_COVER, LV_PART_MAIN);
            break;
            
        default:
            lv_obj_set_style_bg_color(container, lv_color_hex(0x1e2528), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(container, LV_OPA_COVER, LV_PART_MAIN);
            break;
    }
    
    ESP_LOGI(TAG, "Container created: %dx%d at (%d,%d)", 
             CONTENT_CONTAINER_WIDTH, CONTENT_CONTAINER_HEIGHT,
             CONTENT_CONTAINER_X, CONTENT_CONTAINER_Y);
    
    return container;
}

/**
 * @brief Создает обертку для контента (100% размера родителя)
 */
lv_obj_t* screen_content_wrapper_create(lv_obj_t* parent) {
    lv_obj_t *wrapper = lv_obj_create(parent);
    lv_obj_set_size(wrapper, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(wrapper, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(wrapper, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollbar_mode(wrapper, LV_SCROLLBAR_MODE_OFF);
    
    ESP_LOGI(TAG, "Content wrapper created: 100%% of parent");
    return wrapper;
}

/**
 * @brief Уничтожает контейнер
 */
void screen_container_destroy(lv_obj_t* container) {
    if (container && lv_obj_is_valid(container)) {
        lv_obj_del(container);
    }
}

/**
 * @brief Показывает контейнер
 */
void screen_container_show(lv_obj_t* container) {
    if (container && lv_obj_is_valid(container)) {
        lv_obj_clear_flag(container, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает контейнер
 */
void screen_container_hide(lv_obj_t* container) {
    if (container && lv_obj_is_valid(container)) {
        lv_obj_add_flag(container, LV_OBJ_FLAG_HIDDEN);
    }
}
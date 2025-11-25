#include "menu_config.h"
#include "esp_log.h"
#include <string.h>  // Добавляем для memcpy

static const char *TAG = "MENU_CONFIG";

// Глобальные конфигурации для каждого типа меню
menu_config_t menu_configs[MENU_TYPE_COUNT] = {
    // Главное меню (6 элементов, высота 70px)
    [MENU_TYPE_MAIN] = {
        .initial_index = 2,     // Начинаем с 3-го элемента
        .scroll_boundary = 2,   // Граница прокрутки
        .visible_items = 5      // Видно 5 элементов
    },
    // Меню входов/выходов (5 элементов, высота 40px)
    [MENU_TYPE_IN_OUT] = {
        .initial_index = 4,     // Начинаем с 1-го элемента
        .scroll_boundary = 6,   // Граница прокрутки
        .visible_items = 7      // Видно примерно 7 элементов, ставим левые значения что бы список не двигался, пунктов мало
    },
    // Меню отопления (11 элементов, высота 40px)
    [MENU_TYPE_CO] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки сдвиг списка вниз когда курсор вверх
        .visible_items = 10      // это настройка позволяет листать почему то после 8 элемента, если считать с 0
    }
};

// Глобальные состояния для каждого типа меню
menu_state_t menu_states[MENU_TYPE_COUNT] = {
    [MENU_TYPE_MAIN] = {0},
    [MENU_TYPE_IN_OUT] = {0},
    [MENU_TYPE_CO] = {0}
};

/**
 * @brief Получить конфигурацию для указанного типа меню
 */
const menu_config_t* get_menu_config(menu_type_t menu_type) {
    if (menu_type >= MENU_TYPE_COUNT) {
        ESP_LOGE(TAG, "Invalid menu type: %d", menu_type);
        return &menu_configs[MENU_TYPE_MAIN]; // Возвращаем конфиг по умолчанию
    }
    return &menu_configs[menu_type];
}

/**
 * @brief Установить конфигурацию для указанного типа меню
 */
void set_menu_config(menu_type_t menu_type, const menu_config_t* config) {
    if (menu_type >= MENU_TYPE_COUNT || config == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for set_menu_config");
        return;
    }
    menu_configs[menu_type] = *config;
    ESP_LOGI(TAG, "Menu config updated for type %d: initial_index=%lu, scroll_boundary=%lu, visible_items=%lu", 
             menu_type, config->initial_index, config->scroll_boundary, config->visible_items);
}

/**
 * @brief Получить состояние для указанного типа меню
 */
menu_state_t* get_menu_state(menu_type_t menu_type) {
    if (menu_type >= MENU_TYPE_COUNT) {
        ESP_LOGE(TAG, "Invalid menu type: %d", menu_type);
        return &menu_states[MENU_TYPE_MAIN]; // Возвращаем состояние по умолчанию
    }
    return &menu_states[menu_type];
}

/**
 * @brief Установить состояние для указанного типа меню
 */
void set_menu_state(menu_type_t menu_type, const menu_state_t* state) {
    if (menu_type >= MENU_TYPE_COUNT || state == NULL) {
        ESP_LOGE(TAG, "Invalid parameters for set_menu_state");
        return;
    }
    menu_states[menu_type] = *state;
    ESP_LOGI(TAG, "Menu state updated for type %d: cursor_index=%lu, list_index=%lu", 
             menu_type, state->cursor_index, state->list_index);
}
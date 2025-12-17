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
    // Меню отопления (8 элементов: Назад + 7 подменю, высота 40px)
    [MENU_TYPE_CO] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки сдвиг списка вниз когда курсор вверх
        .visible_items = 8      // Видно 8 элементов
    },
    // Меню общие настройки отопления (7 элементов: Назад + 6 параметров, высота 40px)
    [MENU_TYPE_CO_GENERAL] = {
        .initial_index = 4,     // 3й элемент будет располагаться по центру
        .scroll_boundary = 6,   // Граница прокрутки
        .visible_items = 7      // Видно 7 элементов
    },
    // Меню графика отопления (15 элементов: Назад + 14 параметров)
    [MENU_TYPE_CO_HEATING_GRAPH] = {
        .initial_index = 4,     // 7й элемент будет располагаться по центру (из 15 элементов)
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 15     // Видно все 15 элементов
    },
    // Меню насосов (18 элементов: Назад + 17 параметров)
    [MENU_TYPE_CO_PUMPS] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 10      // Видно примерно 10 элементов
    },
    // Меню клапан (8 элементов: Назад + 7 параметров)
    [MENU_TYPE_CO_VALVE] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 8      // Видно 8 элементов
    },
    // Меню ручной режим (4 элемента: Назад + 3 параметра)
    [MENU_TYPE_CO_MANUAL] = {
        .initial_index = 3,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 4      // Видно 4 элемента
    },
    // Меню расписания (8 элементов: Назад + 7 дней недели)
    [MENU_TYPE_CO_SCHEDULE] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 8      // Видно 8 элементов
    },
    // Меню дня недели расписания (9 элементов: Назад + 8 параметров)
    [MENU_TYPE_CO_SCHEDULE_DAY] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 9      // Видно 9 элементов
    },
    // Меню аварий (5 элементов: Назад + 4 типа аварий)
    [MENU_TYPE_CO_ALARMS] = {
        .initial_index = 3,     // 3й элемент будет располагаться по центру
        .scroll_boundary = 3,   // Граница прокрутки
        .visible_items = 5      // Видно 5 элементов
    },
    // Меню сухого хода (4 элемента: Назад + 3 параметра)
    [MENU_TYPE_CO_ALARMS_DRY_RUN] = {
        .initial_index = 3,     // 3й элемент будет располагаться по центру
        .scroll_boundary = 3,   // Граница прокрутки
        .visible_items = 4      // Видно 4 элемента
    },
    // Меню внешней аварии (5 элементов: Назад + 4 параметра)
    [MENU_TYPE_CO_ALARMS_EXTERNAL] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 5      // Видно 5 элементов
    },
    // Меню обрыва датчика (4 элемента: Назад + 3 параметра)
    [MENU_TYPE_CO_ALARMS_SENSOR_BREAK] = {
        .initial_index = 3,     // 3й элемент будет располагаться по центру
        .scroll_boundary = 3,   // Граница прокрутки
        .visible_items = 4      // Видно 4 элемента
    },
    // Меню аварийного отклонения (7 элементов: Назад + 6 параметров)
    [MENU_TYPE_CO_ALARMS_DEVIATION] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 7      // Видно 7 элементов
    },
    // Меню ГВС (7 элементов: Назад + 6 подменю, высота 40px)
    [MENU_TYPE_GVS] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки сдвиг списка вниз когда курсор вверх
        .visible_items = 7      // Видно 7 элементов
    },
    // Меню общие настройки ГВС (7 элементов: Назад + 6 параметров, высота 40px)
    [MENU_TYPE_GVS_GENERAL] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 7      // Видно 7 элементов
    },
    // Меню насосов ГВС (18 элементов: Назад + 17 параметров)
    [MENU_TYPE_GVS_PUMPS] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 10      // Видно примерно 10 элементов
    },
    // Меню клапан ГВС (8 элементов: Назад + 7 параметров)
    [MENU_TYPE_GVS_VALVE] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 8      // Видно 8 элементов
    },
    // Меню ручной режим ГВС (4 элемента: Назад + 3 параметра)
    [MENU_TYPE_GVS_MANUAL] = {
        .initial_index = 3,     // 3й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 4      // Видно 4 элемента
    },
    // Меню расписания ГВС (8 элементов: Назад + 7 дней недели)
    [MENU_TYPE_GVS_SCHEDULE] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 8      // Видно 8 элементов
    },
    // Меню дня недели расписания ГВС (5 элементов: Назад + 4 параметра)
    [MENU_TYPE_GVS_SCHEDULE_DAY] = {
        .initial_index = 4,     // 2й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 9      // Видно 5 элементов
    },
    // Меню аварий ГВС (5 элементов: Назад + 4 типа аварий)
    [MENU_TYPE_GVS_ALARMS] = {
        .initial_index = 3,     // 3й элемент будет располагаться по центру
        .scroll_boundary = 3,   // Граница прокрутки
        .visible_items = 5      // Видно 5 элементов
    },
    // Меню сухого хода ГВС (4 элемента: Назад + 3 параметра)
    [MENU_TYPE_GVS_ALARMS_DRY_RUN] = {
        .initial_index = 3,     // 3й элемент будет располагаться по центру
        .scroll_boundary = 3,   // Граница прокрутки
        .visible_items = 4      // Видно 4 элемента
    },
    // Меню внешней аварии ГВС (5 элементов: Назад + 4 параметра)
    [MENU_TYPE_GVS_ALARMS_EXTERNAL] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 5      // Видно 5 элементов
    },
    // Меню обрыва датчика ГВС (4 элемента: Назад + 3 параметра)
    [MENU_TYPE_GVS_ALARMS_SENSOR_BREAK] = {
        .initial_index = 3,     // 3й элемент будет располагаться по центру
        .scroll_boundary = 3,   // Граница прокрутки
        .visible_items = 4      // Видно 4 элемента
    },
    // Меню аварийного отклонения ГВС (7 элементов: Назад + 6 параметров)
    [MENU_TYPE_GVS_ALARMS_DEVIATION] = {
        .initial_index = 4,     // 4й элемент будет располагаться по центру
        .scroll_boundary = 4,   // Граница прокрутки
        .visible_items = 7      // Видно 7 элементов
    }
};

// Глобальные состояния для каждого типа меню
menu_state_t menu_states[MENU_TYPE_COUNT] = {
    [MENU_TYPE_MAIN] = {0},
    [MENU_TYPE_IN_OUT] = {0},
    [MENU_TYPE_CO] = {0},
    [MENU_TYPE_CO_GENERAL] = {0},
    [MENU_TYPE_CO_HEATING_GRAPH] = {0},
    [MENU_TYPE_CO_PUMPS] = {0},
    [MENU_TYPE_CO_VALVE] = {0},
    [MENU_TYPE_CO_MANUAL] = {0},
    [MENU_TYPE_CO_SCHEDULE] = {0},
    [MENU_TYPE_CO_SCHEDULE_DAY] = {0},
    [MENU_TYPE_CO_ALARMS] = {0},
    [MENU_TYPE_CO_ALARMS_DRY_RUN] = {0},
    [MENU_TYPE_CO_ALARMS_EXTERNAL] = {0},
    [MENU_TYPE_CO_ALARMS_SENSOR_BREAK] = {0},
    [MENU_TYPE_CO_ALARMS_DEVIATION] = {0},
    [MENU_TYPE_GVS] = {0},
    [MENU_TYPE_GVS_GENERAL] = {0},
    [MENU_TYPE_GVS_PUMPS] = {0},
    [MENU_TYPE_GVS_VALVE] = {0},
    [MENU_TYPE_GVS_MANUAL] = {0},
    [MENU_TYPE_GVS_SCHEDULE] = {0},
    [MENU_TYPE_GVS_SCHEDULE_DAY] = {0},
    [MENU_TYPE_GVS_ALARMS] = {0},
    [MENU_TYPE_GVS_ALARMS_DRY_RUN] = {0},
    [MENU_TYPE_GVS_ALARMS_EXTERNAL] = {0},
    [MENU_TYPE_GVS_ALARMS_SENSOR_BREAK] = {0},
    [MENU_TYPE_GVS_ALARMS_DEVIATION] = {0}
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
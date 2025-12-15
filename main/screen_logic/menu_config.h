#ifndef MENU_CONFIG_H
#define MENU_CONFIG_H

#include <stdint.h>
#include <stddef.h>  // Добавляем для NULL

#ifdef __cplusplus
extern "C" {
#endif

// Типы меню
typedef enum {
    MENU_TYPE_MAIN,              // Главное меню
    MENU_TYPE_IN_OUT,            // Меню входов/выходов
    MENU_TYPE_CO,                // Меню отопления
    MENU_TYPE_CO_GENERAL,        // Меню общие настройки отопления
    MENU_TYPE_CO_HEATING_GRAPH,  // Меню графика отопления
    MENU_TYPE_CO_PUMPS,          // Меню насосов
    MENU_TYPE_CO_VALVE,          // Меню клапан
    MENU_TYPE_CO_MANUAL,         // Меню ручной режим
    MENU_TYPE_CO_SCHEDULE,       // Меню расписания
    MENU_TYPE_CO_SCHEDULE_DAY,   // Меню дня недели расписания
    MENU_TYPE_CO_ALARMS,         // Меню аварий
    MENU_TYPE_CO_ALARMS_DRY_RUN,      // Меню сухого хода
    MENU_TYPE_CO_ALARMS_EXTERNAL,     // Меню внешней аварии
    MENU_TYPE_CO_ALARMS_SENSOR_BREAK, // Меню обрыва датчика
    MENU_TYPE_CO_ALARMS_DEVIATION,     // Меню аварийного отклонения
    MENU_TYPE_GVS,                // Меню ГВС
    MENU_TYPE_GVS_GENERAL,        // Меню общие настройки ГВС
    MENU_TYPE_GVS_PUMPS,          // Меню насосов ГВС
    MENU_TYPE_GVS_VALVE,          // Меню клапан ГВС
    MENU_TYPE_GVS_MANUAL,         // Меню ручной режим ГВС
    MENU_TYPE_GVS_SCHEDULE,       // Меню расписания ГВС
    MENU_TYPE_GVS_SCHEDULE_DAY,   // Меню дня недели расписания ГВС
    MENU_TYPE_GVS_ALARMS,         // Меню аварий ГВС
    MENU_TYPE_GVS_ALARMS_DRY_RUN, // Меню сухого хода ГВС
    MENU_TYPE_GVS_ALARMS_EXTERNAL, // Меню внешней аварии ГВС
    MENU_TYPE_GVS_ALARMS_SENSOR_BREAK, // Меню обрыва датчика ГВС
    MENU_TYPE_GVS_ALARMS_DEVIATION,     // Меню аварийного отклонения ГВС
    MENU_TYPE_COUNT              // Количество типов меню
} menu_type_t;

// Структура состояния меню (отдельная для каждого меню)
typedef struct {
    uint32_t cursor_index;  // Текущая позиция курсора
    uint32_t list_index;    // Текущая позиция списка
} menu_state_t;

// Структура конфигурации меню
typedef struct {
    uint32_t initial_index;    // Начальный индекс при инициализации
    uint32_t scroll_boundary;  // Граница для прокрутки списка
    uint32_t visible_items;    // Количество видимых элементов
} menu_config_t;

// Функции для работы с конфигурацией меню
const menu_config_t* get_menu_config(menu_type_t menu_type);
void set_menu_config(menu_type_t menu_type, const menu_config_t* config);

// Функции для работы с состоянием меню
menu_state_t* get_menu_state(menu_type_t menu_type);
void set_menu_state(menu_type_t menu_type, const menu_state_t* state);

// Глобальные конфигурации для каждого типа меню
extern menu_config_t menu_configs[MENU_TYPE_COUNT];

// Глобальные состояния для каждого типа меню
extern menu_state_t menu_states[MENU_TYPE_COUNT];

#ifdef __cplusplus
}
#endif

#endif // MENU_CONFIG_H
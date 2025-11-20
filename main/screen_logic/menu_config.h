#ifndef MENU_CONFIG_H
#define MENU_CONFIG_H

#include <stdint.h>
#include <stddef.h>  // Добавляем для NULL

#ifdef __cplusplus
extern "C" {
#endif

// Типы меню
typedef enum {
    MENU_TYPE_MAIN,      // Главное меню
    MENU_TYPE_IN_OUT,    // Меню входов/выходов
    MENU_TYPE_CO,        // Меню отопления
    MENU_TYPE_COUNT      // Количество типов меню
} menu_type_t;

// Структура конфигурации меню
typedef struct {
    uint32_t initial_index;    // Начальный индекс при инициализации
    uint32_t scroll_boundary;  // Граница для прокрутки списка
    uint32_t visible_items;    // Количество видимых элементов
} menu_config_t;

// Функции для работы с конфигурацией меню
const menu_config_t* get_menu_config(menu_type_t menu_type);
void set_menu_config(menu_type_t menu_type, const menu_config_t* config);

// Глобальные конфигурации для каждого типа меню
extern menu_config_t menu_configs[MENU_TYPE_COUNT];

#ifdef __cplusplus
}
#endif

#endif // MENU_CONFIG_H
#ifndef ARC_MENU_H
#define ARC_MENU_H

#include "lvgl.h"
#include "encoder/encoder.h"
#include "menu_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Структура для пользовательских данных объекта
typedef struct {
    bool is_status_icon;     // Флаг, указывающий что это статусная иконка
    bool is_param_value;     // Флаг, указывающий что это контейнер значения параметра
} custom_obj_data_t;

// Функции для работы со статусными иконками
void set_as_status_icon(lv_obj_t *obj);
bool is_status_icon(lv_obj_t *obj);

// Функции для работы с контейнерами значений параметров
void set_as_param_value(lv_obj_t *obj);
bool is_param_value(lv_obj_t *obj);

void free_custom_data(lv_event_t *e);  // Функция для очистки памяти

// Объявления функций дугового меню
void arc_menu_update_slide(lv_obj_t *cont);
void arc_menu_event_cb(lv_event_t *e);
void arc_menu_handle_encoder(uint8_t e, lv_obj_t *cont, menu_state_t *menu_state, menu_type_t menu_type);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // ARC_MENU_H
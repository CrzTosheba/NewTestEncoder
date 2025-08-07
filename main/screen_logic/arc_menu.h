// Файл: arc_menu.h
#ifndef ARC_MENU_H
#define ARC_MENU_H

#include "lvgl.h"
#include "encoder/encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления функций
void arc_menu_update_slide(lv_obj_t *cont);
void arc_menu_event_cb(lv_event_t *e);
void arc_menu_handle_encoder(uint8_t e, lv_obj_t *cont, uint32_t *current_index);

// Глобальные переменные для управления меню
extern uint32_t current_cursor_index; // Текущая позиция курсора
extern const uint8_t VISIBLE_ITEMS;   // Количество видимых элементов на экране

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // ARC_MENU_H
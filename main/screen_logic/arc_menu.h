#ifndef ARC_MENU_H
#define ARC_MENU_H

#include "lvgl.h"
#include "encoder/encoder.h"
#include "menu_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления функций
void arc_menu_update_slide(lv_obj_t *cont);
void arc_menu_event_cb(lv_event_t *e);
void arc_menu_handle_encoder(uint8_t e, lv_obj_t *cont, menu_state_t *menu_state, menu_type_t menu_type);

// Глобальные переменные для управления меню (теперь используются через menu_state_t)
// extern uint32_t current_cursor_index; // УДАЛЕНО - больше не используем глобальную переменную

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // ARC_MENU_H
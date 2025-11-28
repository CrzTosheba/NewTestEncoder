#ifndef CO_MANUAL_MENU_H
#define CO_MANUAL_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции меню ручной режим
void CO_Manual_Menu_List(void);
void co_manual_menu_encoder_event_cb(uint8_t e);
void co_manual_menu_cleanup(void);
void co_manual_menu_show(void);
void co_manual_menu_hide(void);

// Объявляем глобальную переменную для доступа из других файлов
extern lv_obj_t *co_manual_cont;

#ifdef __cplusplus
}
#endif

#endif // CO_MANUAL_MENU_H


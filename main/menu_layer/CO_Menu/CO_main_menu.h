#ifndef CO_MAIN_MENU_H
#define CO_MAIN_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции меню отопления
void CO_Menu_List(void);
void co_menu_encoder_event_cb(uint8_t e);
void co_menu_cleanup(void);
void co_menu_show(void);
void co_menu_hide(void);

// Объявляем глобальную переменную для доступа из других файлов
extern lv_obj_t *co_cont;

#ifdef __cplusplus
}
#endif

#endif // CO_MAIN_MENU_H
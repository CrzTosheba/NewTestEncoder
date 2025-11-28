#ifndef CO_PUMPS_MENU_H
#define CO_PUMPS_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции меню насосов
void CO_Pumps_Menu_List(void);
void co_pumps_menu_encoder_event_cb(uint8_t e);
void co_pumps_menu_cleanup(void);
void co_pumps_menu_show(void);
void co_pumps_menu_hide(void);

// Объявляем глобальную переменную для доступа из других файлов
extern lv_obj_t *co_pumps_cont;

#ifdef __cplusplus
}
#endif

#endif // CO_PUMPS_MENU_H


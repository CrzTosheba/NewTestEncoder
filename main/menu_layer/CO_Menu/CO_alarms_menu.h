#ifndef CO_ALARMS_MENU_H
#define CO_ALARMS_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции главного меню аварий
void CO_Alarms_Menu_List(void);
void co_alarms_menu_encoder_event_cb(uint8_t e);
void co_alarms_menu_cleanup(void);
void co_alarms_menu_show(void);
void co_alarms_menu_hide(void);

// Объявляем глобальные переменные для доступа из других файлов
extern lv_obj_t *co_alarms_cont;

#ifdef __cplusplus
}
#endif

#endif // CO_ALARMS_MENU_H


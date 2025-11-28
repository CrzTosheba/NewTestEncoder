#ifndef CO_EXT_ALARM_MENU_H
#define CO_EXT_ALARM_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции меню внешней аварии
void CO_Ext_Alarm_Menu_List(void);
void co_ext_alarm_menu_encoder_event_cb(uint8_t e);
void co_ext_alarm_menu_cleanup(void);
void co_ext_alarm_menu_show(void);
void co_ext_alarm_menu_hide(void);

#ifdef __cplusplus
}
#endif

#endif // CO_EXT_ALARM_MENU_H


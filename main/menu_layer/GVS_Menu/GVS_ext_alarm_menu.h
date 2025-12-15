#ifndef GVS_EXT_ALARM_MENU_H
#define GVS_EXT_ALARM_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции меню внешней аварии ГВС
void GVS_Ext_Alarm_Menu_List(void);
void gvs_ext_alarm_menu_encoder_event_cb(uint8_t e);
void gvs_ext_alarm_menu_cleanup(void);
void gvs_ext_alarm_menu_show(void);
void gvs_ext_alarm_menu_hide(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_EXT_ALARM_MENU_H


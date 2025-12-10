#ifndef GVS_DEV_ALARM_MENU_H
#define GVS_DEV_ALARM_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции меню аварийного отклонения ГВС
void GVS_Dev_Alarm_Menu_List(void);
void gvs_dev_alarm_menu_encoder_event_cb(uint8_t e);
void gvs_dev_alarm_menu_cleanup(void);
void gvs_dev_alarm_menu_show(void);
void gvs_dev_alarm_menu_hide(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_DEV_ALARM_MENU_H


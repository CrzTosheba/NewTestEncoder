#ifndef GVS_SENSOR_BREAK_MENU_H
#define GVS_SENSOR_BREAK_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции меню обрыва датчика ГВС
void GVS_Sensor_Break_Menu_List(void);
void gvs_sensor_break_menu_encoder_event_cb(uint8_t e);
void gvs_sensor_break_menu_cleanup(void);
void gvs_sensor_break_menu_show(void);
void gvs_sensor_break_menu_hide(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_SENSOR_BREAK_MENU_H


#ifndef CO_SENSOR_BREAK_MENU_H
#define CO_SENSOR_BREAK_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции меню обрыва датчика
void CO_Sensor_Break_Menu_List(void);
void co_sensor_break_menu_encoder_event_cb(uint8_t e);
void co_sensor_break_menu_cleanup(void);
void co_sensor_break_menu_show(void);
void co_sensor_break_menu_hide(void);

#ifdef __cplusplus
}
#endif

#endif // CO_SENSOR_BREAK_MENU_H


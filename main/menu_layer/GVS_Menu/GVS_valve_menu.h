#ifndef GVS_VALVE_MENU_H
#define GVS_VALVE_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции меню клапан ГВС
void GVS_Valve_Menu_List(void);
void gvs_valve_menu_encoder_event_cb(uint8_t e);
void gvs_valve_menu_cleanup(void);
void gvs_valve_menu_show(void);
void gvs_valve_menu_hide(void);

// Объявляем глобальную переменную для доступа из других файлов
extern lv_obj_t *gvs_valve_cont;

#ifdef __cplusplus
}
#endif

#endif // GVS_VALVE_MENU_H


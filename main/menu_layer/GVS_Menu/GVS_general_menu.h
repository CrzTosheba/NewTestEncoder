#ifndef GVS_GENERAL_MENU_H
#define GVS_GENERAL_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Тип режима ГВС
typedef enum {
    GVS_MODE_MANUAL = 0,   // РУЧН
    GVS_MODE_SCHEDULE = 1, // РАСП
    GVS_MODE_ECON = 2,     // ЭКОН
    GVS_MODE_COMF = 3,     // КОМФ
    GVS_MODE_ALARM = 4     // АВАР
} gvs_mode_t;

// Функции меню общие настройки ГВС
void GVS_General_Menu_List(void);
void gvs_general_menu_encoder_event_cb(uint8_t e);
void gvs_general_menu_cleanup(void);
void gvs_general_menu_show(void);
void gvs_general_menu_hide(void);

// Объявляем глобальную переменную для доступа из других файлов
extern lv_obj_t *gvs_general_cont;

#ifdef __cplusplus
}
#endif

#endif // GVS_GENERAL_MENU_H


#ifndef CO_GENERAL_MENU_H
#define CO_GENERAL_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Тип режима отопления
typedef enum {
    MODE_COMF = 0,  // КОМФ
    MODE_ECON = 1   // ЭКОН
} heating_mode_t;

// Функции меню общие настройки отопления
void CO_General_Menu_List(void);
void co_general_menu_encoder_event_cb(uint8_t e);
void co_general_menu_cleanup(void);
void co_general_menu_show(void);
void co_general_menu_hide(void);

// Объявляем глобальную переменную для доступа из других файлов
extern lv_obj_t *co_general_cont;

#ifdef __cplusplus
}
#endif

#endif // CO_GENERAL_MENU_H


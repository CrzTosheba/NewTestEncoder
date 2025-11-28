#ifndef CO_HEATING_GRAPH_MENU_H
#define CO_HEATING_GRAPH_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Тип способа задания графика отопления
typedef enum {
    HEATING_GRAPH_TYPE_POINTS = 0,  // По точкам
    HEATING_GRAPH_TYPE_SLOPE = 1    // По углу наклона
} heating_graph_type_t;

// Функции меню графика отопления
void CO_Heating_Graph_Menu_List(void);
void co_heating_graph_menu_encoder_event_cb(uint8_t e);
void co_heating_graph_menu_cleanup(void);
void co_heating_graph_menu_show(void);
void co_heating_graph_menu_hide(void);

// Объявляем глобальную переменную для доступа из других файлов
extern lv_obj_t *co_heating_graph_cont;

#ifdef __cplusplus
}
#endif

#endif // CO_HEATING_GRAPH_MENU_H


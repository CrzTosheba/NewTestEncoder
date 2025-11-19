#ifndef SCREEN_IN_OUT_H
#define SCREEN_IN_OUT_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов и изображений
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_controller);

// Функции создания виджетов (должны быть объявлены в соответствующих заголовочных файлах)
lv_obj_t* in_out_pic_main(lv_obj_t* parent);
lv_obj_t* digital_out_up(lv_obj_t* parent);
lv_obj_t* all_in_out_down(lv_obj_t* parent);
lv_obj_t* universal_in_down(lv_obj_t* parent);
lv_obj_t* analog_out_down(lv_obj_t* parent);

// Функция создания экрана входов/выходов
void screen_In_Out_create(lv_obj_t *parent);

// Функции управления подсветкой областей
void screen_In_Out_show_all_highlights(void);
void screen_In_Out_show_universal_inputs(void);
void screen_In_Out_show_analog_outputs(void);
void screen_In_Out_show_discrete_outputs(void);
void screen_In_Out_hide_all_highlights(void);
void screen_In_Out_cleanup_highlights(void);

#ifdef __cplusplus
}
#endif

#endif // SCREEN_IN_OUT_H
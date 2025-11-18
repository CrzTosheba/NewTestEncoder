#ifndef SCREEN_IN_OUT_SECOND_H
#define SCREEN_IN_OUT_SECOND_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов и изображений
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_controller);

// Функция создания экрана входов/выходов
void screen_In_Out_create(lv_obj_t *parent);


#ifdef __cplusplus
}
#endif

#endif // SCREEN_IN_OUT_SECOND_H
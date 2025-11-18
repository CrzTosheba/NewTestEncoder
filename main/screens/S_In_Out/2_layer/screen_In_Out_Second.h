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
void screen_In_Out_create_Second(lv_obj_t *parent);

// Функции управления подсветкой областей
void screen_In_Out_show_all_highlights(void);        // Показать подсветку всех областей
void screen_In_Out_show_universal_inputs(void);      // Показать подсветку универсальных входов
void screen_In_Out_show_analog_outputs(void);        // Показать подсветку аналоговых выходов
void screen_In_Out_show_discrete_outputs(void);      // Показать подсветку дискретных выходов
void screen_In_Out_hide_all_highlights(void);        // Скрыть все подсветки
void screen_In_Out_cleanup_highlights(void);         // Очистить указатели на виджеты подсветки

#ifdef __cplusplus
}
#endif

#endif // SCREEN_IN_OUT_SECOND_H
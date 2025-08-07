// Файл: main_menu.h
#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "lvgl.h"
#include "encoder/encoder.h"

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_18);
LV_FONT_DECLARE(Roboto_bold_24);

// Объявления изображений
LV_IMG_DECLARE(lv_im_module_hotwater);
LV_IMG_DECLARE(lv_im_module_lock);
LV_IMG_DECLARE(lv_im_module_heat);
LV_IMG_DECLARE(lv_im_module_podp);
LV_IMG_DECLARE(lv_im_module_input_output);
LV_IMG_DECLARE(lv_im_arrow_down);
LV_IMG_DECLARE(lv_im_arrow_up);
LV_IMG_DECLARE(lv_im_arrow_right);
LV_IMG_DECLARE(lv_im_module_off);
LV_IMG_DECLARE(lv_im_module_on);
LV_IMG_DECLARE(lv_im_module_inout);

// Прототипы функций
void Main_Menu_List(void);
void main_menu_encoder_event_cb(uint8_t e);

#endif // MAIN_MENU_H
// Файл: main_menu.h
#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "lvgl.h"
#include "encoder/encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_16);
LV_FONT_DECLARE(Roboto_bold_18);
LV_FONT_DECLARE(Roboto_bold_20);
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

// Прототипы функций экранов
void screen_Pass_create(lv_obj_t *parent);
void screen_Gvs_create(lv_obj_t *parent);
void screen_CO_create(lv_obj_t *parent);
void screen_Podp_create(lv_obj_t *parent);
void screen_Uv_create(lv_obj_t *parent);
void screen_In_Out_create(lv_obj_t *parent);

// Прототипы функций меню
void Main_Menu_List(void);
void main_menu_encoder_event_cb(uint8_t e);

// Функции управления видимостью главного меню
void main_menu_show(void);
void main_menu_hide(void);

// Глобальные указатели на экраны (объявлены в main_menu.c)
extern lv_obj_t *screens[6];
extern lv_obj_t *_cont;
extern lv_obj_t *content_container;

#ifdef __cplusplus
}
#endif

#endif // MAIN_MENU_H
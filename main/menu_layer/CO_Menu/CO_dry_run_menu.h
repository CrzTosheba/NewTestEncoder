#ifndef CO_DRY_RUN_MENU_H
#define CO_DRY_RUN_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции меню сухого хода
void CO_Dry_Run_Menu_List(void);
void co_dry_run_menu_encoder_event_cb(uint8_t e);
void co_dry_run_menu_cleanup(void);
void co_dry_run_menu_show(void);
void co_dry_run_menu_hide(void);

#ifdef __cplusplus
}
#endif

#endif // CO_DRY_RUN_MENU_H


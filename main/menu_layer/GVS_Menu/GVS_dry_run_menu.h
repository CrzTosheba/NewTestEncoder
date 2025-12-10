#ifndef GVS_DRY_RUN_MENU_H
#define GVS_DRY_RUN_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции меню сухого хода ГВС
void GVS_Dry_Run_Menu_List(void);
void gvs_dry_run_menu_encoder_event_cb(uint8_t e);
void gvs_dry_run_menu_cleanup(void);
void gvs_dry_run_menu_show(void);
void gvs_dry_run_menu_hide(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_DRY_RUN_MENU_H


#ifndef IN_OUT_MAIN_MENU_H
#define IN_OUT_MAIN_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

void Input_Output_Menu_List(void);
void input_output_encoder_event_cb(uint8_t e);
void input_output_menu_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // INPUT_OUTPUT_MENU_H
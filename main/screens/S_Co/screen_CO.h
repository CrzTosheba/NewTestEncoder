#ifndef SCREEN_CO_H_
#define SCREEN_CO_H_

#include "lvgl.h"
#include "my_widgets/w_b_big.h"
#include "my_widgets/w_b_small.h"

LV_FONT_DECLARE(Roboto_bold_18);
LV_FONT_DECLARE(Roboto_bold_24);

LV_IMG_DECLARE(lv_im_scheme);
LV_IMG_DECLARE(lv_im_p_arrow_up);
LV_IMG_DECLARE(lv_im_valve_on);
LV_IMG_DECLARE(lv_im_pump_on);
LV_IMG_DECLARE(lv_im_pump_off);
LV_IMG_DECLARE(lv_im_drop);


void screen_CO_create(lv_obj_t *parent);

#endif /*SCREEN_CO_H_*/
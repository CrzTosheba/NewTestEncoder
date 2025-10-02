#ifndef SCREEN_HEAT_H_
#define SCREEN_HEAT_H_

#include "lvgl.h"
#include "my_widgets/b_big.h"
#include "my_widgets/b_small.h"

LV_IMG_DECLARE(lv_im_scheme);
LV_IMG_DECLARE(lv_im_p_arrow_up);
LV_FONT_DECLARE(Roboto_bold_18);
LV_FONT_DECLARE(Roboto_bold_24);

LV_IMG_DECLARE(lv_im_scheme);
LV_IMG_DECLARE(lv_im_p_arrow_up);

void screen_Heat_create(lv_obj_t *parent);

#endif /*SCREEN_HEAT_H_*/
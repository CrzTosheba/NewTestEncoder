#ifndef SCREEN_IN_OUT_H
#define SCREEN_IN_OUT_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMAGE_DECLARE(lv_im_controller);
void screen_In_Out_create(lv_obj_t *parent);


#ifdef __cplusplus
}
#endif

#endif // SCREEN_IN_OUT_H
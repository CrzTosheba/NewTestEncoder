#ifndef SCREEN_PASS_H
#define SCREEN_PASS_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMAGE_DECLARE(lv_im_big_lock_close);
LV_IMAGE_DECLARE(lv_im_big_lock_open);
void screen_Pass_create(lv_obj_t *parent);


#ifdef __cplusplus
}
#endif

#endif // SCREEN_PASS_H
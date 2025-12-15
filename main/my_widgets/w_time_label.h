#ifndef W_TIME_LABEL_H_
#define W_TIME_LABEL_H_

#include "lvgl.h"

LV_IMG_DECLARE(lv_im_time);
LV_IMG_DECLARE(lv_im_comfort);
LV_IMG_DECLARE(lv_im_day);
LV_IMG_DECLARE(lv_im_hand);
LV_IMG_DECLARE(lv_im_night);

lv_obj_t* status_img(lv_obj_t *parent);  // Добавлен параметр parent

#endif /*W_TIME_LABEL_H_*/
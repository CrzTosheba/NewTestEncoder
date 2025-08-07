#include "time_label.h"

static const lv_image_dsc_t* anim_imgs[5] = {
    &lv_im_time,
    &lv_im_comfort,
    &lv_im_day,
    &lv_im_hand,
    &lv_im_night,
};

lv_obj_t* status_img(void)
{
    lv_obj_t * img = lv_animimg_create(lv_scr_act());
    
    // Правильный вызов для LVGL 9.2
    lv_animimg_set_src(img, (const void **) anim_imgs, 5);
    
    lv_animimg_set_duration(img, 2000);
    lv_animimg_set_repeat_count(img, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(img);
    return img;
}
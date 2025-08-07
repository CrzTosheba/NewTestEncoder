

#include "b_big.h"

LV_IMG_DECLARE(lv_im_bubble_02);

lv_obj_t* bubble_b(void)
{
    lv_obj_t * img = lv_img_create(lv_scr_act());
    lv_img_set_src(img,&lv_im_bubble_02);

    return img;
};
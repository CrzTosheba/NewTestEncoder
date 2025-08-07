
#include "rad_mask.h"


LV_IMG_DECLARE(lv_im_radial_mask);



lv_obj_t* radial(void)
{
    lv_obj_t * img = lv_img_create(lv_scr_act());
    lv_img_set_src(img,&lv_im_radial_mask);
    return img;

};
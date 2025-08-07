
#include "scale_img.h"


LV_IMG_DECLARE(lv_im_pump_off);



lv_obj_t* pump_off_im(void)
{
    lv_obj_t * img = lv_img_create(lv_scr_act());
    lv_img_set_src(img,&lv_im_pump_off);
    return img;

};
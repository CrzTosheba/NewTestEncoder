
#include "w_valve_off.h"


LV_IMG_DECLARE(lv_im_valve_off);



lv_obj_t* valve_off_im(void)
{
    lv_obj_t * img = lv_img_create(lv_scr_act());
    lv_img_set_src(img,&lv_im_valve_off);
    return img;

};
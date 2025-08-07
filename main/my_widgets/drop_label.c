
#include "drop_label.h"


LV_IMG_DECLARE(lv_im_drop);



lv_obj_t* drop_img(void)
{
    lv_obj_t * img = lv_img_create(lv_scr_act());
    lv_img_set_src(img,&lv_im_drop);
    return img;

};
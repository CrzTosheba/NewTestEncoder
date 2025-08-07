
#include "scale_marker_img.h"


LV_IMG_DECLARE(lv_im_scale_marker);



lv_obj_t* scale_mark(void)
{
    lv_obj_t * img = lv_img_create(lv_scr_act());
    lv_img_set_src(img,&lv_im_scale_marker);
    return img;

};
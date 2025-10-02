
#include "b_small.h"




lv_obj_t* bubble_s(lv_obj_t *parent)
{
    lv_obj_t * img = lv_img_create(parent); 
    lv_img_set_src(img,&lv_im_bubble_01);
    
    return img;

};
#include "heat_exch.h"



lv_obj_t* heat_exchanger(lv_obj_t *parent)  // Принимаем parent как параметр
{
    lv_obj_t * img = lv_img_create(parent);  // Создаем на переданном parent
    lv_img_set_src(img, &lv_im_scheme);
    
    return img;
}
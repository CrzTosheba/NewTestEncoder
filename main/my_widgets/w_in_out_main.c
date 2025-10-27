#include "w_in_out_main.h"



lv_obj_t* in_out_pic_main(lv_obj_t *parent)  // Принимаем parent как параметр
{
    lv_obj_t * img = lv_img_create(parent);  // Создаем на переданном parent
    lv_img_set_src(img, &lv_im_controller);
    
    return img;
}
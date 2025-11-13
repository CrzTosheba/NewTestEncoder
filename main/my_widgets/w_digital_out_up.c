
#include "w_digital_out_up.h"



lv_obj_t* digital_out_up(lv_obj_t *parent)  // Принимаем parent как параметр
{
    lv_obj_t * img = lv_img_create(parent);  // Создаем на переданном parent
    lv_img_set_src(img, &lv_im_digital_out);
    return img;
}
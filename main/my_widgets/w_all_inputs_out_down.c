
#include "w_all_inputs_out_down.h"



lv_obj_t* all_in_out_down(lv_obj_t *parent)  // Принимаем parent как параметр
{
    lv_obj_t * img = lv_img_create(parent);  // Создаем на переданном parent
    lv_img_set_src(img, &lv_im_all_in_out);
    return img;
}
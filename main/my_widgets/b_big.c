
#include "b_big.h"



lv_obj_t* bubble_b(lv_obj_t *parent)  // Принимаем parent как параметр
{
    lv_obj_t * img = lv_img_create(parent);  // Создаем на переданном parent
    lv_img_set_src(img, &lv_im_bubble_02);
    return img;
}
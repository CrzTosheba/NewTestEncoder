
#include "w_universal_in_down.h"



lv_obj_t* universal_in_down(lv_obj_t *parent)  // Принимаем parent как параметр
{
    lv_obj_t * img = lv_img_create(parent);  // Создаем на переданном parent
    lv_img_set_src(img, &lv_im_universal_in);
    return img;
}
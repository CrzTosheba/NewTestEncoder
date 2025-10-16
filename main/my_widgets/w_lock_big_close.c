
#include "w_lock_big_close.h"

lv_obj_t* lock_big_close(lv_obj_t *parent)  // Принимаем parent как параметр
{
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x1e2528), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_opa(parent, LV_OPA_TRANSP, 0);
    // Не создаем новый экран, используем parent
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "ДОСТУП ЗАКРЫТ");
    lv_obj_set_style_text_font(label, &Roboto_bold_36, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 10, 50);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);

    lv_obj_t * img = lv_img_create(parent);  // Создаем на переданном parent
    lv_img_set_src(img, &lv_im_big_lock_close);
    lv_obj_align(img, LV_ALIGN_CENTER, 15, -30);
    return img;
}
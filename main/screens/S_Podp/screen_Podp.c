#include "screen_Podp.h"
#include "lvgl.h"



void screen_Podp_create(lv_obj_t *parent) {
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x1e2528), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    // Не создаем новый экран, используем parent
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "Экран ПОДПИТКИ");
    lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 10, 10);
      lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);

  
}
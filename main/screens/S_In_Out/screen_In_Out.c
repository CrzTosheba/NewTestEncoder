#include "screen_In_Out.h"


LV_FONT_DECLARE(Roboto_bold_24);

void screen_In_Out_create(lv_scr_load_anim_t anim_type) {
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_t * label = lv_label_create(scr);
    lv_label_set_text(label, "Экран входов/выходов");
    lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_scr_load_anim(scr, anim_type, 300, 300, false);
}
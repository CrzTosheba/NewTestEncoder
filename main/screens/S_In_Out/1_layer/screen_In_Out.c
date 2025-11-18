#include "screen_In_Out.h"
#include "my_widgets/w_in_out_main.h"
#include "my_widgets/w_digital_out_up.h"
#include "my_widgets/w_all_inputs_out_down.h"
#include "my_widgets/w_universal_in_down.h"
#include "my_widgets/w_analog_out_down.h"

void screen_In_Out_create(lv_obj_t *parent) {
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x1E2528), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_opa(parent, LV_OPA_TRANSP, 0);

    static lv_point_precise_t line_points[] = { {-10, 0}, {100, 0} };
    static lv_point_precise_t line_points1[] = { {320, 0}, {460, 0} };

    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 1);
    lv_style_set_line_color(&style_line, lv_color_hex(0xffffff));
    lv_style_set_line_rounded(&style_line, true);

        /*Create a line and apply the new style*/
    lv_obj_t * line1;
    lv_obj_t * line2;

    line1 = lv_line_create(parent);
    lv_line_set_points(line1, line_points, 2);     /*Set the points*/
    lv_obj_add_style(line1, &style_line, 0);

    line2 = lv_line_create(parent);
    lv_line_set_points(line2, line_points1, 2);     /*Set the points*/
    lv_obj_add_style(line2, &style_line, 0);

    // Не создаем новый экран, используем parent
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "ВХОДЫ/ВЫХОДЫ");
    lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 10, -145);
    lv_obj_set_style_bg_color(label, lv_color_hex(0x1E2528), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);

    //---------------Главная картинка-------------------------//
    lv_obj_t *In_Out_scheme = in_out_pic_main(parent);
    lv_obj_align(In_Out_scheme, LV_ALIGN_CENTER, 10, 20);



    
}
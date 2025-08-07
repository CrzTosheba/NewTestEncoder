

#include "header.h"


    static lv_style_t style;

lv_obj_t* head(void)
{
    lv_style_init(&style);

    /*Create style*/
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 1);
    lv_style_set_line_color(&style_line, lv_color_hex(0xffffff));
    lv_style_set_line_rounded(&style_line, true);

    /*Create a line and apply the new style*/
    lv_obj_t * line1;
    lv_obj_t * line2;
    line1 = lv_line_create(lv_scr_act());
    lv_line_set_points(line1, line_points, 2);     /*Set the points*/
    lv_obj_add_style(line1, &style_line, 0);

    line2 = lv_line_create(lv_scr_act());
    lv_line_set_points(line2, line_points1, 2);     /*Set the points*/
    lv_obj_add_style(line2, &style_line, 0);
    
    /*Create a white label, set its text and align it to the center*/
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "ОТОПЛЕНИЕ");
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0); // Исправлено: установка цвета текста для label
    lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, -168, -150);

    return label;
}
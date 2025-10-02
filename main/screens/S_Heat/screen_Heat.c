#include <stdio.h>
#include <string.h>
#include "screen_Heat.h"
#include "my_widgets/b_big.h"
#include "my_widgets/heat_exch.h"
#include "my_widgets/b_small.h"
#include "my_widgets/drop_label.h"
#include "my_widgets/time_label.h"
#include "my_widgets/rad_mask.h"
#include "my_widgets/pump_on_img.h"
#include "my_widgets/pump_off_img.h"
#include "my_widgets/w_valve_on.h"
#include "my_widgets/w_valve_off.h"
#include "encoder/encoder.h"
#include "counters/count_test.h"






void screen_Heat_create(lv_obj_t *parent) 
{
    

    lv_obj_set_style_bg_color(parent, lv_color_hex(0x1E2528), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_opa(parent, LV_OPA_TRANSP, 0);

    static lv_point_precise_t line_points[] = { {55, 0}, {175, 0} };
    static lv_point_precise_t line_points1[] = { {245, 0}, {460, 0} };

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
    lv_label_set_text(label, "ГВС");
    lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 10, -145);
    lv_obj_set_style_bg_color(label, lv_color_hex(0x1E2528), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);

    //---------------Главная картинка-------------------------//
    lv_obj_t *Heat_Scheme = heat_exchanger(parent);
    lv_obj_align(Heat_Scheme, LV_ALIGN_CENTER, 15, 0);


    //-------------Создаем баблы под показания----------------//

    lv_obj_t *Up_Big_buble = bubble_b(parent);
    lv_obj_set_pos(Up_Big_buble, 280, 31);

    lv_obj_t* Up_Left_Smal_buble =  bubble_s(parent);
    lv_obj_set_pos(Up_Left_Smal_buble, 0, 41);

    lv_obj_t *Down_Left_Smal_buble = bubble_s(parent);
    lv_obj_set_pos(Down_Left_Smal_buble, 0, 200);

    lv_obj_t *Down_Right_Smal_buble = bubble_s(parent);
    lv_obj_set_pos(Down_Right_Smal_buble, 280, 200);


  
    
    //---------------временная шакала внизу экрана------------//

    //time_scale_init();
    //set_time(12, 00);

    
    



    
  
    

    fflush(NULL);
}

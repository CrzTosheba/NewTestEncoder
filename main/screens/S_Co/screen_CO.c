#include <stdio.h>
#include <string.h>
#include "screen_CO.h"
#include "my_widgets/b_big.h"
#include "my_widgets/b_small.h"
#include "my_widgets/header.h"
#include "my_widgets/drop_label.h"
#include "my_widgets/time_label.h"
#include "my_widgets/rad_mask.h"
#include "my_widgets/pump_on_img.h"
#include "my_widgets/pump_off_img.h"
#include "my_widgets/w_valve_on.h"
#include "my_widgets/w_valve_off.h"
#include "encoder/encoder.h"
#include "counters/count_test.h"
#include "scale_logic_time/time_scale.h"


;


void main_CO_screen(void)
{
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &Roboto_bold_18);

    lv_obj_t *heat_exchanger = lv_img_create(lv_scr_act());
    lv_img_set_src(heat_exchanger, &lv_im_scheme);
    lv_obj_set_pos(heat_exchanger, 170, 130);
    lv_obj_set_scrollbar_mode(heat_exchanger, LV_SCROLLBAR_MODE_OFF);


    lv_obj_t *arrow_valve = lv_img_create(lv_scr_act());
    lv_img_set_src(arrow_valve, &lv_im_p_arrow_up);
    lv_obj_set_pos(arrow_valve, 70, 240);



    head();
  
 

    lv_obj_t *Up_Big_buble = bubble_b();
    lv_obj_set_pos(Up_Big_buble, 295, 130);


    lv_obj_t* Up_Left_Smal_buble =  bubble_s();
    lv_obj_set_pos(Up_Left_Smal_buble, 20, 140);


    lv_obj_t *Down_Left_Smal_buble = bubble_s();
    lv_obj_set_pos(Down_Left_Smal_buble, 20, 298);





    lv_obj_t *Down_Right_Smal_buble = bubble_s();
    lv_obj_set_pos(Down_Right_Smal_buble, 295, 298);

    lv_obj_t *drop_icon = drop_img();
    lv_obj_set_pos(drop_icon, 10, 75);

    lv_obj_t *time_icon = status_img();
    lv_obj_set_pos(time_icon, 45, 75);

  //  Main_Menu_List();
    




    
//------------------------шкала------------------------//
time_scale_init();
set_time(10, 00);
//---------------------------------------------------------//





    lv_obj_t *pump_on = pump_on_im();
    lv_obj_set_pos(pump_on, 310, 198);

    lv_obj_t *pump_off = pump_off_im();
    lv_obj_set_pos(pump_off, 310, 246);

    lv_obj_t *valve_on = valve_on_im();
    lv_obj_set_pos(valve_on, 30, 218);
    //------------Перенести потом в другие файлы---------------------//
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "КЛАПАН");
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_add_style(label, &style, 0);
    lv_obj_set_pos(label, 75, 218);

    lv_obj_t *label1 = lv_label_create(lv_scr_act());
    lv_label_set_text(label1, "НАСОС 1");
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_add_style(label1, &style, 0);
    lv_obj_set_pos(label1, 355, 198);

    lv_obj_t *label2 = lv_label_create(lv_scr_act());
    lv_label_set_text(label2, "НАСОС 2");
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_add_style(label2, &style, 0);
    lv_obj_set_pos(label2, 355, 246);
//--------------цифирки в баблах--------------//



//------------------------------------------//
         lv_obj_t * Clapan = lv_label_create(lv_scr_act());
     lv_label_set_text(Clapan, "23 / 24%");
       lv_obj_add_style(Clapan, &style, 0);
     lv_obj_set_pos(Clapan, 90, 240);

    lv_obj_t * Nasos1 = lv_label_create(lv_scr_act());
     lv_label_set_text(Nasos1, "17");
       lv_obj_add_style(Nasos1, &style, 0);
     lv_obj_set_pos(Nasos1, 355, 220);

        lv_obj_t * Nasos2 = lv_label_create(lv_scr_act());
     lv_label_set_text(Nasos2, "0");
       lv_obj_add_style(Nasos2, &style, 0);
     lv_obj_set_pos(Nasos2, 355, 268);






    //------------------------------------------------------------//



   main_test();
   

   //drop_char_logic();
   //  spinbox_no_digit();

    fflush(NULL);
}

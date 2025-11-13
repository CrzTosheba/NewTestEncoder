#include "screen_In_Out_Second.h"
#include "my_widgets/w_in_out_main.h"
#include "my_widgets/w_digital_out_up.h"
#include "my_widgets/w_all_inputs_out_down.h"
#include "my_widgets/w_universal_in_down.h"
#include "my_widgets/w_analog_out_down.h"



void screen_In_Out_create_Second(lv_obj_t *parent) {

        //----------------Выделение всех входов и выходов-------------//

    lv_obj_t *All_In_Out_Up = digital_out_up(parent);
    lv_obj_align(All_In_Out_Up, LV_ALIGN_CENTER, 2, -86); // все верхние клеммы верх
    lv_obj_t *All_In_Out_Down = all_in_out_down(parent);
    lv_obj_align(All_In_Out_Down, LV_ALIGN_CENTER, -20, 125); //все верхние клеммы низ

    

    //---------------Универсальные входы ---------------//

     lv_obj_t *Universal_In_Down_Left = universal_in_down(parent);
     lv_obj_align(Universal_In_Down_Left, LV_ALIGN_CENTER, -60, 125); // входы низ лево
    //----------------Аналоговые выходы---------------//

    lv_obj_t *Analog_Out_Down_Right = analog_out_down(parent);
    lv_obj_align(Analog_Out_Down_Right, LV_ALIGN_CENTER, 95, 125); // выходы низ право
}

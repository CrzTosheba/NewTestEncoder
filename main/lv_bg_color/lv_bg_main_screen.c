
#include <stdio.h>
#include <string.h>
#include "lv_bg_main_screen.h"






void main_screen_bg(void)
{
lv_obj_t* scr_bg = lv_scr_act();
lv_obj_set_style_bg_color(scr_bg, lv_color_hex(0x1e2528), LV_PART_MAIN);
lv_obj_set_scrollbar_mode(scr_bg, LV_SCROLLBAR_MODE_OFF); // дизеблим скрол бар на главном экране
fflush(NULL);
};
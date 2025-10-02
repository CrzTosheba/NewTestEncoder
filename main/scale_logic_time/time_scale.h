#ifndef TIME_SCALE_H
#define TIME_SCALE_H

#include "lvgl.h"
#include "my_widgets/scale_img.h"
#include "my_widgets/scale_marker_img.h"
LV_FONT_DECLARE(Roboto_bold_18);

// Инициализация шкалы и маркера
void time_scale_init(void);

// Установка времени и обновление позиции маркера
void set_time(int hours, int minutes);

// Показать/скрыть шкалу времени
void show_time_scale(bool show);

// Установка временных интервалов для линий
void set_time_intervals(int line1_start_h, int line1_start_m, int line1_end_h, int line1_end_m,
                       int line2_start_h, int line2_start_m, int line2_end_h, int line2_end_m);

#endif // TIME_SCALE_H
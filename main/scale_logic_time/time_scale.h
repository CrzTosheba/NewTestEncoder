#ifndef TIME_SCALE_H
#define TIME_SCALE_H

#include "lvgl.h" // Подключаем LVGL
#include "my_widgets/scale_img.h"
#include "my_widgets/scale_marker_img.h"
LV_FONT_DECLARE(Roboto_bold_18);

// Инициализация шкалы и маркера
void time_scale_init(void);

// Установка времени и обновление позиции маркера
void set_time(int hours, int minutes);

#endif // TIME_SCALE_H
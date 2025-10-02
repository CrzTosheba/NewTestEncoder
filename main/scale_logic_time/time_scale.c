#include "time_scale.h"
#include <stdio.h>
#include "esp_log.h"

// Константы для границ шкалы
#define SCALE_START_X 5
#define SCALE_END_X 461
#define LINE_Y 386
#define TIME_LABEL_Y 358

// Создаем и настраиваем стиль линии
static lv_style_t style_line;

// Объекты шкалы и маркера
static lv_obj_t *scale;
static lv_obj_t *scale_marker;
static lv_obj_t *time_label;
static lv_obj_t *lines[2];

// Точки для линий
static lv_point_precise_t line_points[2];
static lv_point_precise_t line_points1[2];

// Флаг видимости шкалы
static bool is_time_scale_visible = false;

// Функция для вычисления X-координаты на основе времени
static int calculate_x_from_time(int hours, int minutes) {
    if (hours < 0) hours = 0;
    if (hours > 23) hours = 23;
    if (minutes < 0) minutes = 0;
    if (minutes > 59) minutes = 59;

    int total_minutes = hours * 60 + minutes;
    float time_ratio = (float)total_minutes / (24 * 60);
    return SCALE_START_X + (int)(time_ratio * (SCALE_END_X - SCALE_START_X));
}

// Инициализация шкалы и маркера
void time_scale_init(void) {
    // Инициализация стиля линии
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 5);
    lv_style_set_line_color(&style_line, lv_color_hex(0xffffff));
    lv_style_set_line_rounded(&style_line, true);

    // Создаем и позиционируем объекты
    scale = scale_im();
    scale_marker = scale_mark();
    lv_obj_set_pos(scale, 5, 380);
    lv_obj_set_pos(scale_marker, 380, 361);

    // Инициализируем точки линий (по умолчанию)
    line_points[0].y = LINE_Y;
    line_points[1].y = LINE_Y;
    line_points1[0].y = LINE_Y;
    line_points1[1].y = LINE_Y;

    // Создаем линии и применяем стиль
    lines[0] = lv_line_create(lv_scr_act());
    lines[1] = lv_line_create(lv_scr_act());

    for (int i = 0; i < 2; i++) {
        lv_obj_add_style(lines[i], &style_line, 0);
    }

    // Создаем текстовый объект для отображения времени
    time_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(time_label, &Roboto_bold_18, 0);
    lv_obj_set_pos(time_label, 350, 358);

    // Изначально скрываем все элементы шкалы
    show_time_scale(false);
}

// Установка временных интервалов для линий
void set_time_intervals(int line1_start_h, int line1_start_m, int line1_end_h, int line1_end_m,
                       int line2_start_h, int line2_start_m, int line2_end_h, int line2_end_m) {
    // Линия 1
    line_points[0].x = calculate_x_from_time(line1_start_h, line1_start_m);
    line_points[1].x = calculate_x_from_time(line1_end_h, line1_end_m);

    // Линия 2
    line_points1[0].x = calculate_x_from_time(line2_start_h, line2_start_m);
    line_points1[1].x = calculate_x_from_time(line2_end_h, line2_end_m);

    // Обновляем точки на линиях
    lv_line_set_points(lines[0], line_points, 2);
    lv_line_set_points(lines[1], line_points1, 2);

}

// Установка времени и обновление позиции маркера
void set_time(int hours, int minutes) {
    if (!is_time_scale_visible) return;

    int marker_x = calculate_x_from_time(hours, minutes);
    char time_str[10];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", hours, minutes);

    lv_label_set_text(time_label, time_str);
    lv_obj_set_pos(scale_marker, marker_x, 361);
    lv_obj_set_pos(time_label, marker_x + 20, TIME_LABEL_Y);
}

// Показать/скрыть шкалу времени
void show_time_scale(bool show) {
    is_time_scale_visible = show;
    
    if (scale) {
        if (show) {
            lv_obj_clear_flag(scale, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(scale, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    if (scale_marker) {
        if (show) {
            lv_obj_clear_flag(scale_marker, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(scale_marker, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    if (time_label) {
        if (show) {
            lv_obj_clear_flag(time_label, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(time_label, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    for (int i = 0; i < 2; i++) {
        if (lines[i]) {
            if (show) {
                lv_obj_clear_flag(lines[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(lines[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}
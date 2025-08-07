#include <stdio.h>
#include <string.h>
#include "count_test.h"
#include "lvgl/lvgl.h"

// Объявление шрифта
LV_FONT_DECLARE(Roboto_bold_18);

// Глобальные переменные
static lv_obj_t *labels[4]; // Массив для хранения меток
static uint32_t cnt = 0;    // Счетчик для обновления данных

// Позиции меток на экране
static const lv_point_t label_positions[] = {
    {40, 154},  // Позиция lbl_cnt
    {40, 313},  // Позиция lbl_cnt1
    {320, 138}, // Позиция lbl_cnt2
    {320, 313}  // Позиция lbl_cnt3
};

// Функция для обновления данных в метках
static void add_data(lv_timer_t *timer) {
    cnt = (cnt + 1) % 100; // Увеличиваем счетчик с ограничением до 100

    // Обновляем текст в каждой метке
    lv_label_set_text_fmt(labels[0], "%ld °C", cnt);
    lv_label_set_text_fmt(labels[1], "%.1f °C", cnt / 4.3f);
    lv_label_set_text_fmt(labels[2], "%.1f (%.1f)°C\n%.1f б", cnt / 2.3f, cnt / 1.1f, cnt / 1.43f);
    lv_label_set_text_fmt(labels[3], "%.1f б", cnt / 3.11f);
}

// Основная функция для создания интерфейса
void main_test(void) {
    // Инициализация стиля
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &Roboto_bold_18);

    // Создание меток
    for (int i = 0; i < 4; i++) {
        labels[i] = lv_label_create(lv_scr_act());
        lv_obj_set_pos(labels[i], label_positions[i].x, label_positions[i].y);
        lv_obj_add_style(labels[i], &style, 0);
    }

    // Установка цвета фона экрана
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x1E2528), LV_PART_MAIN);

    // Создание таймера для обновления данных
    lv_timer_create(add_data, 500, NULL);

    // Сброс буферов вывода
    fflush(NULL);
}
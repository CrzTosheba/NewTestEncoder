#include "time_scale.h"
//#include "esp_sntp.h" // Для синхронизации времени по SNTP
#include <stdio.h>
#include "esp_log.h" // Подключение библиотеки для логирования в ESP32

static const char *TAG = "main"; 

// Константы для границ шкалы
#define SCALE_START_X 5    // Начало шкалы по оси X
#define SCALE_END_X 461    // Конец шкалы по оси X
#define LINE_Y 386         // Y-координата для линий
#define TIME_LABEL_Y 358   // Y-координата для текста времени

// Создаем и настраиваем стиль линии
static lv_style_t style_line;

// Объекты шкалы и маркера
static lv_obj_t *scale;
static lv_obj_t *scale_marker;

// Текстовый объект для отображения времени
static lv_obj_t *time_label;

// Функция для вычисления X-координаты на основе времени
static int calculate_x_from_time(int hours, int minutes) {
    // Проверка корректности времени
    if (hours < 0) hours = 0;
    if (hours > 23) hours = 23;
    if (minutes < 0) minutes = 0;
    if (minutes > 59) minutes = 59;

    // Вычисление общего количества минут
    int total_minutes = hours * 60 + minutes;

    // Вычисление пропорции времени и соответствующей координаты X
    float time_ratio = (float)total_minutes / (24 * 60);
    return SCALE_START_X + (int)(time_ratio * (SCALE_END_X - SCALE_START_X));
}

// Функция преобразования времени в X-координату
static int time_to_x(int hours, int minutes) {
    return calculate_x_from_time(hours, minutes);
}

lv_point_precise_t line_points[] = {{30, LINE_Y}, {200, LINE_Y}}; // Первая линия
lv_point_precise_t line_points1[] = {{280, LINE_Y}, {420, LINE_Y}}; // Вторая линия

// Инициализация шкалы и маркера
void time_scale_init(void) {
    // Инициализация стиля линии
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 5); // Толщина линии
    lv_style_set_line_color(&style_line, lv_color_hex(0xffffff)); // Белый цвет линии
    lv_style_set_line_rounded(&style_line, true); // Закругленные концы линии

    // Создаем и позиционируем объекты
    scale = scale_im(); // Создаем шкалу
    scale_marker = scale_mark(); // Создаем маркер
    lv_obj_set_pos(scale, 5, 380); // Позиция шкалы (начало шкалы: X=5, Y=380)
    lv_obj_set_pos(scale_marker, 380, 361); // Позиция маркера (начальная позиция: X=380, Y=361)

    // Линия 1: от 08:15 до 10:45
    int16_t line1_x1 = calculate_x_from_time(1, 15);  // Начало линии 1
    int line1_x2 = calculate_x_from_time(10, 45); // Конец линии 1

    int line2_x1 = calculate_x_from_time(14, 0); // Начало в 14:00
    int line2_x2 = calculate_x_from_time(20, 0); // Конец в 20:00

    // Создаем линии и применяем стиль
    lv_obj_t *lines[] = {
        lv_line_create(lv_scr_act()), // Первая линия
        lv_line_create(lv_scr_act())  // Вторая линия
    };

    ESP_LOGI(TAG, "line1_x1=%d, line1_x2=%d", line1_x1, line1_x2);
    
    line_points[0].x = line1_x1;
    line_points[1].x = line1_x2;
    line_points1[0].x = line2_x1;
    line_points1[1].x = line2_x2;

    lv_line_set_points(lines[0], line_points, 2); // Устанавливаем точки для первой линии
    lv_line_set_points(lines[1], line_points1, 2); // Устанавливаем точки для второй линии

    for (int i = 0; i < 2; i++) {
        lv_obj_add_style(lines[i], &style_line, 0); // Применяем стиль к линиям
    }

    // Создаем текстовый объект для отображения времени
    time_label = lv_label_create(lv_scr_act()); // Создаем текстовый объект
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xffffff), 0); // Белый цвет текста
    lv_obj_set_style_text_font(time_label, &Roboto_bold_18, 0); // Шрифт
    lv_obj_set_pos(time_label, 350, 358); // Начальная позиция текста 
}

// Установка времени и обновление позиции маркера
void set_time(int hours, int minutes) {
    // Вычисляем позицию маркера по оси X
    int marker_x = calculate_x_from_time(hours, minutes);

    // Форматируем время в строку "часы:минуты"
    char time_str[10]; // Буфер для строки времени
    snprintf(time_str, sizeof(time_str), "%02d:%02d", hours, minutes); // Форматируем время

    // Обновляем текст на экране
    lv_label_set_text(time_label, time_str);

    // Обновляем позицию маркера
    lv_obj_set_pos(scale_marker, marker_x, 361); // Устанавливаем новую позицию маркера

    // Обновляем позицию текста (слева от маркера, на расстоянии 10 пикселей)
    lv_obj_set_pos(time_label, marker_x + 20, TIME_LABEL_Y); // Текст справа от маркера
}
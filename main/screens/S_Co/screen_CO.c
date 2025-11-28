/**
 * @file screen_CO.c
 * @brief Реализация экрана отопления (CO - Central Heating)
 * 
 * Этот модуль создает интерфейс экрана управления системой отопления.
 * Экран включает в себя схему теплообменника, индикаторы температуры
 * (баблы) и декоративные элементы.
 */

#include <stdio.h>
#include <string.h>
#include "screen_CO.h"
#include "my_widgets/w_heat_exch.h"
#include "my_widgets/w_drop_label.h"
#include "my_widgets/w_time_label.h"
#include "my_widgets/w_rad_mask.h"
#include "my_widgets/w_pump_on_img.h"
#include "my_widgets/w_pump_off_img.h"
#include "my_widgets/w_valve_on.h"
#include "my_widgets/w_valve_off.h"
#include "encoder/encoder.h"
#include "counters/count_test.h"

/**
 * @brief Создает интерфейс экрана отопления
 * 
 * Функция создает все элементы интерфейса экрана отопления:
 * - Заголовок "ОТОПЛЕНИЕ"
 * - Декоративные линии в верхней части
 * - Схему теплообменника в центре
 * - Индикаторы температуры (баблы) в четырех углах
 * 
 * @param parent Указатель на родительский объект LVGL (контейнер экрана)
 * 
 * @note Все созданные объекты автоматически удаляются при удалении parent
 * @note Стили инициализируются только один раз благодаря статическим флагам
 * @note Функция должна вызываться с заблокированным LVGL мьютексом
 */
void screen_CO_create(lv_obj_t *parent) 
{
    // ========== Настройка фона родительского контейнера ==========
    // Устанавливаем темно-серый фон (#1E2528)
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x1E2528), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, LV_PART_MAIN);
    // Отключаем скроллбар для чистого интерфейса
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    // Делаем границу прозрачной
    lv_obj_set_style_border_opa(parent, LV_OPA_TRANSP, 0);

    // ========== Определение координат для декоративных линий ==========
    // Левая линия: от x=55 до x=125, y=0
    static lv_point_precise_t line_points[] = { {55, 0}, {125, 0} };
    // Правая линия: от x=295 до x=460, y=0
    static lv_point_precise_t line_points1[] = { {295, 0}, {460, 0} };

    // ========== Инициализация стиля для линий ==========
    // ВАЖНО: Стиль инициализируется только один раз при первом вызове функции
    // Повторная инициализация уже инициализированного стиля в LVGL недопустима
    // и может привести к повреждению данных и перезагрузке контроллера
    static lv_style_t style_line;
    static bool style_line_inited = false;
    if (!style_line_inited) {
        lv_style_init(&style_line);
        lv_style_set_line_width(&style_line, 1);              // Толщина линии: 1 пиксель
        lv_style_set_line_color(&style_line, lv_color_hex(0xffffff)); // Белый цвет
        lv_style_set_line_rounded(&style_line, true);       // Закругленные концы
        style_line_inited = true;
    }

    // ========== Создание декоративных линий в верхней части экрана ==========
    lv_obj_t *line1 = lv_line_create(parent);
    lv_line_set_points(line1, line_points, 2);  // Устанавливаем точки для левой линии
    lv_obj_add_style(line1, &style_line, 0);     // Применяем стиль

    lv_obj_t *line2 = lv_line_create(parent);
    lv_line_set_points(line2, line_points1, 2); // Устанавливаем точки для правой линии
    lv_obj_add_style(line2, &style_line, 0);     // Применяем стиль

    // ========== Создание заголовка экрана ==========
    // Создаем метку с текстом "ОТОПЛЕНИЕ"
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "ОТОПЛЕНИЕ");
    // Устанавливаем жирный шрифт Roboto размером 24
    lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
    // Выравниваем по центру с небольшим смещением (x: +10, y: -145)
    lv_obj_align(label, LV_ALIGN_CENTER, 10, -145);
    // Устанавливаем фон метки в цвет фона экрана (для прозрачности)
    lv_obj_set_style_bg_color(label, lv_color_hex(0x1E2528), LV_PART_MAIN);
    // Белый цвет текста
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);

    // ========== Создание главной схемы теплообменника ==========
    // Создаем виджет схемы теплообменника и выравниваем по центру
    lv_obj_t *Heat_Scheme = heat_exchanger(parent);
    lv_obj_align(Heat_Scheme, LV_ALIGN_CENTER, -7, 0);

    // ========== Создание индикаторов температуры (баблов) ==========
    // Баблы используются для отображения показаний температуры в разных точках системы
    
    // Большой бабл в правом верхнем углу
    lv_obj_t *Up_Big_buble = bubble_b(parent);
    lv_obj_set_pos(Up_Big_buble, 255, 31);

    // Маленький бабл в левом верхнем углу
    lv_obj_t *Up_Left_Smal_buble = bubble_s(parent);
    lv_obj_set_pos(Up_Left_Smal_buble, -15, 41);

    // Большой бабл в левом нижнем углу
    lv_obj_t *Down_Left_Big_buble = bubble_b(parent);
    lv_obj_set_pos(Down_Left_Big_buble, -15, 200);

    // Маленький бабл в правом нижнем углу
    lv_obj_t *Down_Right_Smal_buble = bubble_s(parent);
    lv_obj_set_pos(Down_Right_Smal_buble, 255, 200);

    // ========== Временная шкала (закомментировано) ==========
    // Функциональность временной шкалы внизу экрана временно отключена
    // Раскомментируйте для активации:
    // time_scale_init();
    // set_time(12, 00);

    // Принудительная очистка буферов вывода (для отладки)
    fflush(NULL);
}

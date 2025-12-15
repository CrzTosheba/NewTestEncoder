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

// ========== СТАТИЧЕСКИЕ ОБЪЕКТЫ И СТИЛИ ==========
static lv_style_t style_line;
static lv_style_t style_label;
static lv_style_t style_bubble_label;
static bool styles_inited = false;

// ========== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ==========
/**
 * @brief Инициализация стилей (вызывается один раз)
 */
static void init_styles(void)
{
    if (styles_inited) return;
    
    // Стиль для линий
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 1);
    lv_style_set_line_color(&style_line, lv_color_hex(0xffffff));
    lv_style_set_line_rounded(&style_line, true);
    
    // Стиль для обычных меток
    lv_style_init(&style_label);
    lv_style_set_text_color(&style_label, lv_color_hex(0xffffff));
    lv_style_set_text_font(&style_label, &Roboto_bold_18);
    lv_style_set_bg_color(&style_label, lv_color_hex(0x1E2528));
    lv_style_set_bg_opa(&style_label, LV_OPA_COVER);
    
    // Стиль для меток на баблах (без фона, выравнивание по левому краю)
    lv_style_init(&style_bubble_label);
    lv_style_set_text_color(&style_bubble_label, lv_color_hex(0xffffff));
    lv_style_set_text_font(&style_bubble_label, &Roboto_bold_18);
    lv_style_set_text_align(&style_bubble_label, LV_TEXT_ALIGN_LEFT);
    lv_style_set_bg_opa(&style_bubble_label, LV_OPA_TRANSP);
    
    styles_inited = true;
}

/**
 * @brief Создает метку на бабле с текстом
 */
static lv_obj_t* create_bubble_label(lv_obj_t *parent, const char *text, lv_coord_t x, lv_coord_t y)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_add_style(label, &style_bubble_label, 0);
    lv_obj_set_pos(label, x, y);
    
    return label;
}

/**
 * @brief Создает заголовок экрана
 */
static lv_obj_t* create_title(lv_obj_t *parent)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "ОТОПЛЕНИЕ");
    lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_color(label, lv_color_hex(0x1E2528), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 10, -145);
    
    return label;
}

/**
 * @brief Создает декоративные линии
 */
static void create_decorative_lines(lv_obj_t *parent)
{
    // Левая линия
    static const lv_point_precise_t line_left[] = {{55, 0}, {125, 0}};
    lv_obj_t *line1 = lv_line_create(parent);
    lv_line_set_points(line1, line_left, 2);
    lv_obj_add_style(line1, &style_line, 0);
    
    // Правая линия
    static const lv_point_precise_t line_right[] = {{295, 0}, {460, 0}};
    lv_obj_t *line2 = lv_line_create(parent);
    lv_line_set_points(line2, line_right, 2);
    lv_obj_add_style(line2, &style_line, 0);
}

/**
 * @brief Создает индикаторы температуры (баблы) с метками
 */
static void create_temperature_bubbles(lv_obj_t *parent)
{
    // Верхний левый маленький бабл (71°C)
    lv_obj_t *bubble_ul = bubble_s(parent);
    lv_obj_set_pos(bubble_ul, -15, 41);
    // Метка на бабле (выравнивание по левому краю внутри бабла)
    create_bubble_label(bubble_ul, "71°C", 20, 15);
    
    // Верхний правый большой бабл
    lv_obj_t *bubble_ur = bubble_b(parent);
    lv_obj_set_pos(bubble_ur, 255, 31);
    // Две строки текста на бабле
    create_bubble_label(bubble_ur, "940.6(56.5)°C", 20, 10);
    create_bubble_label(bubble_ur, "4.1 б", 20, 30);
    
    // Нижний левый большой бабл (31.3°C)
    lv_obj_t *bubble_dl = bubble_s(parent);
    lv_obj_set_pos(bubble_dl, -15, 200);
    create_bubble_label(bubble_dl, "31.3°C", 20, 15);
    
    // Нижний правый маленький бабл (3.2 б)
    lv_obj_t *bubble_dr = bubble_s(parent);
    lv_obj_set_pos(bubble_dr, 255, 200);
    create_bubble_label(bubble_dr, "3.2 б", 20, 15);
}

/**
 * @brief Создает блок клапана
 */
static void create_valve_block(lv_obj_t *parent)
{
    // Надпись "КЛАПАН"
    lv_obj_t *label_valve = lv_label_create(parent);
    lv_label_set_text(label_valve, "КЛАПАН");
    lv_obj_add_style(label_valve, &style_label, 0);
    lv_obj_align(label_valve, LV_ALIGN_CENTER, -140, -10);

    // Рисонок клапана, их должно быть два, на откр и закр
    lv_obj_t *valve_on = lv_img_create(parent);
    lv_img_set_src(valve_on, &lv_im_valve_on);
    lv_obj_align(valve_on, LV_ALIGN_CENTER, -200, 0);
    
    // Стрелка клапана тоже должно быть 2, на октр и закр
    lv_obj_t *arrow_valve = lv_img_create(parent);
    lv_img_set_src(arrow_valve, &lv_im_p_arrow_up);
    lv_obj_align(arrow_valve, LV_ALIGN_CENTER, -170, 12);
    
    // Значение клапана
    lv_obj_t *label_valve_value = lv_label_create(parent);
    lv_label_set_text(label_valve_value, "23 / 24%");
    lv_obj_add_style(label_valve_value, &style_label, 0);
    lv_obj_align(label_valve_value, LV_ALIGN_CENTER, -125, 12);
}

/**
 * @brief Создает блок насосов
 */
static void create_pumps_block(lv_obj_t *parent)
{
    // Насос 1
    lv_obj_t *label_pump1_title = lv_label_create(parent);
    lv_label_set_text(label_pump1_title, "НАСОС 1");
    lv_obj_add_style(label_pump1_title, &style_label, 0);
    lv_obj_set_pos(label_pump1_title, 300, 110);
    // Изображение насоса, должно быть 2 изображения на вкл и выкл
    // Изображение насоса 1 на вкл
    lv_obj_t *pump1_on = lv_img_create(parent);
    lv_img_set_src(pump1_on, &lv_im_pump_on);
    lv_obj_set_pos(pump1_on, 260, 110);
     
    // Значения насоса 1
    lv_obj_t *label_pump1_value = lv_label_create(parent);
    lv_label_set_text(label_pump1_value, "17");
    lv_obj_add_style(label_pump1_value, &style_label, 0);
    lv_obj_set_pos(label_pump1_value, 300, 132);
    
    // Насос 2
    lv_obj_t *label_pump2_title = lv_label_create(parent);
    lv_label_set_text(label_pump2_title, "НАСОС 2");
    lv_obj_add_style(label_pump2_title, &style_label, 0);
    lv_obj_set_pos(label_pump2_title, 300, 158);

    // Изображение насоса 2 выкл
    lv_obj_t *pump2_off = lv_img_create(parent);
    lv_img_set_src(pump2_off, &lv_im_pump_off);
    lv_obj_set_pos(pump2_off, 260, 158);

    // Значения насоса 2
    lv_obj_t *label_pump2_value = lv_label_create(parent);
    lv_label_set_text(label_pump2_value, "0");
    lv_obj_add_style(label_pump2_value, &style_label, 0);
    lv_obj_set_pos(label_pump2_value, 300, 180);
}
static void create_status_icon_block(lv_obj_t *parent)
{

    lv_obj_t *drop_icon_CO = lv_img_create(parent);
    lv_img_set_src(drop_icon_CO, &lv_im_drop);
    lv_obj_set_pos(drop_icon_CO, 20, -17);

    lv_obj_t *time_icon = status_img(parent);
    lv_obj_set_pos(time_icon, -10, -17);



}

/**
 * @brief Создает интерфейс экрана отопления
 */
void screen_CO_create(lv_obj_t *parent) 
{
    // ========== НАСТРОЙКА ФОНА ==========
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x1E2528), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_opa(parent, LV_OPA_TRANSP, 0);
    
    // ========== ИНИЦИАЛИЗАЦИЯ СТИЛЕЙ ==========
    init_styles();
    
    // ========== СОЗДАНИЕ ЭЛЕМЕНТОВ ИНТЕРФЕЙСА ==========
    create_decorative_lines(parent);
    create_title(parent);
    
    // Схема теплообменника
    lv_obj_t *Heat_Scheme = heat_exchanger(parent);
    lv_obj_align(Heat_Scheme, LV_ALIGN_CENTER, -7, 0);
    
    create_temperature_bubbles(parent);
    create_valve_block(parent);
    create_pumps_block(parent);
    create_status_icon_block(parent);
    
    // Принудительная очистка буферов вывода (только для отладки)
    fflush(NULL);
}

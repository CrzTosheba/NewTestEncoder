#include "screen_In_Out_Second.h"
#include "my_widgets/w_in_out_main.h"
#include "my_widgets/w_digital_out_up.h"
#include "my_widgets/w_all_inputs_out_down.h"
#include "my_widgets/w_universal_in_down.h"
#include "my_widgets/w_analog_out_down.h"
#include "esp_log.h"

// Статические переменные для хранения указателей на виджеты подсветки
static lv_obj_t *widget_all_in_out_up = NULL;
static lv_obj_t *widget_all_in_out_down = NULL;
static lv_obj_t *widget_universal_in_down_left = NULL;
static lv_obj_t *widget_analog_out_down_right = NULL;
static lv_obj_t *widget_discrete_outputs = NULL;

static const char *TAG = "IO_SCREEN";

/**
 * @brief Безопасная проверка объекта LVGL
 * Упрощенная версия - используем только lv_obj_is_valid
 */
static bool is_obj_valid_safe(lv_obj_t *obj) {
    if (obj == NULL) return false;
    
    // Простая проверка через встроенную функцию LVGL
    // Если объект был удален, lv_obj_is_valid вернет false
    return lv_obj_is_valid(obj);
}

/**
 * @brief Проверяет, является ли объект валидным и существует ли еще
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return is_obj_valid_safe(obj);
}

/**
 * @brief Создает экран входов/выходов с основной схемой и элементами подсветки
 * @param parent Родительский контейнер для создания экрана
 */
void screen_In_Out_create_Second(lv_obj_t *parent) {
    ESP_LOGI(TAG, "Creating In/Out screen");
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x1E2528), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(parent, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_opa(parent, LV_OPA_TRANSP, 0);

    static lv_point_precise_t line_points[] = { {-10, 0}, {100, 0} };
    static lv_point_precise_t line_points1[] = { {320, 0}, {460, 0} };

    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 1);
    lv_style_set_line_color(&style_line, lv_color_hex(0xffffff));
    lv_style_set_line_rounded(&style_line, true);

    /*Create a line and apply the new style*/
    lv_obj_t * line1;
    lv_obj_t * line2;

    line1 = lv_line_create(parent);
    lv_line_set_points(line1, line_points, 2);     /*Set the points*/
    lv_obj_add_style(line1, &style_line, 0);

    line2 = lv_line_create(parent);
    lv_line_set_points(line2, line_points1, 2);     /*Set the points*/
    lv_obj_add_style(line2, &style_line, 0);

    // Создание заголовка экрана (такие же координаты как в screen_In_Out.c)
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "ВХОДЫ/ВЫХОДЫ");
    lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 10, -145);
    lv_obj_set_style_bg_color(label, lv_color_hex(0x1E2528), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);

    //---------------Главная картинка-------------------------//
    // Используем такие же координаты как в screen_In_Out.c
    lv_obj_t *In_Out_scheme = in_out_pic_main(parent);
    lv_obj_align(In_Out_scheme, LV_ALIGN_CENTER, 10, 20);
    
    //----------------Выделение всех входов и выходов-------------//
    // Создаем виджеты подсветки с правильными координатами
    widget_all_in_out_up = digital_out_up(parent);
    lv_obj_align(widget_all_in_out_up, LV_ALIGN_CENTER, 2, -86); // все верхние клеммы верх
    
    widget_all_in_out_down = all_in_out_down(parent);
    lv_obj_align(widget_all_in_out_down, LV_ALIGN_CENTER, -20, 125); // все верхние клеммы низ

    //---------------Универсальные входы ---------------//
    widget_universal_in_down_left = universal_in_down(parent);
    lv_obj_align(widget_universal_in_down_left, LV_ALIGN_CENTER, -60, 125); // входы низ лево
    
    //----------------Аналоговые выходы---------------//
    widget_analog_out_down_right = analog_out_down(parent);
    lv_obj_align(widget_analog_out_down_right, LV_ALIGN_CENTER, 95, 125); // выходы низ право
    
    //----------------Дискретные выходы---------------//
    // Для дискретных выходов используем ту же картинку digital_out_up, но с другим выравниванием если нужно
    widget_discrete_outputs = digital_out_up(parent); // Используем ту же функцию
    lv_obj_align(widget_discrete_outputs, LV_ALIGN_CENTER, 2, -86); // дискретные выходы - верхние клеммы
    
    // Изначально скрываем все подсветки
    screen_In_Out_hide_all_highlights();
    
    ESP_LOGI(TAG, "In/Out screen created successfully");
}

/**
 * @brief Показывает подсветку всех областей (для пункта "Все")
 */
void screen_In_Out_show_all_highlights(void) {
    ESP_LOGI(TAG, "Showing all highlights");
    
    // Показываем виджеты подсветки всех областей
    if (is_obj_valid(widget_all_in_out_up)) lv_obj_clear_flag(widget_all_in_out_up, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_all_in_out_down)) lv_obj_clear_flag(widget_all_in_out_down, LV_OBJ_FLAG_HIDDEN);
    
    // Скрываем остальные виджеты подсветки
    if (is_obj_valid(widget_universal_in_down_left)) lv_obj_add_flag(widget_universal_in_down_left, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_analog_out_down_right)) lv_obj_add_flag(widget_analog_out_down_right, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_discrete_outputs)) lv_obj_add_flag(widget_discrete_outputs, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief Показывает подсветку универсальных входов (для пункта "Универсальные входы")
 */
void screen_In_Out_show_universal_inputs(void) {
    ESP_LOGI(TAG, "Showing universal inputs highlight");
    
    // Показываем виджет подсветки универсальных входов
    if (is_obj_valid(widget_universal_in_down_left)) lv_obj_clear_flag(widget_universal_in_down_left, LV_OBJ_FLAG_HIDDEN);
    
    // Скрываем остальные виджеты подсветки
    if (is_obj_valid(widget_all_in_out_up)) lv_obj_add_flag(widget_all_in_out_up, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_all_in_out_down)) lv_obj_add_flag(widget_all_in_out_down, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_analog_out_down_right)) lv_obj_add_flag(widget_analog_out_down_right, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_discrete_outputs)) lv_obj_add_flag(widget_discrete_outputs, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief Показывает подсветку аналоговых выходов (для пункта "Аналоговые выходы")
 */
void screen_In_Out_show_analog_outputs(void) {
    ESP_LOGI(TAG, "Showing analog outputs highlight");
    
    // Показываем виджет подсветки аналоговых выходов
    if (is_obj_valid(widget_analog_out_down_right)) lv_obj_clear_flag(widget_analog_out_down_right, LV_OBJ_FLAG_HIDDEN);
    
    // Скрываем остальные виджеты подсветки
    if (is_obj_valid(widget_all_in_out_up)) lv_obj_add_flag(widget_all_in_out_up, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_all_in_out_down)) lv_obj_add_flag(widget_all_in_out_down, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_universal_in_down_left)) lv_obj_add_flag(widget_universal_in_down_left, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_discrete_outputs)) lv_obj_add_flag(widget_discrete_outputs, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief Показывает подсветку дискретных выходов (для пункта "Дискретные выходы")
 */
void screen_In_Out_show_discrete_outputs(void) {
    ESP_LOGI(TAG, "Showing discrete outputs highlight");
    
    // Показываем виджет подсветки дискретных выходов (верхние клеммы)
    if (is_obj_valid(widget_discrete_outputs)) lv_obj_clear_flag(widget_discrete_outputs, LV_OBJ_FLAG_HIDDEN);
    
    // Скрываем остальные виджеты подсветки
    if (is_obj_valid(widget_all_in_out_up)) lv_obj_add_flag(widget_all_in_out_up, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_all_in_out_down)) lv_obj_add_flag(widget_all_in_out_down, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_universal_in_down_left)) lv_obj_add_flag(widget_universal_in_down_left, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_analog_out_down_right)) lv_obj_add_flag(widget_analog_out_down_right, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief Скрывает все подсветки областей
 */
void screen_In_Out_hide_all_highlights(void) {
    ESP_LOGI(TAG, "Hiding all highlights");
    
    // Скрываем все виджеты подсветки только если они валидны
    if (is_obj_valid(widget_all_in_out_up)) lv_obj_add_flag(widget_all_in_out_up, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_all_in_out_down)) lv_obj_add_flag(widget_all_in_out_down, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_universal_in_down_left)) lv_obj_add_flag(widget_universal_in_down_left, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_analog_out_down_right)) lv_obj_add_flag(widget_analog_out_down_right, LV_OBJ_FLAG_HIDDEN);
    if (is_obj_valid(widget_discrete_outputs)) lv_obj_add_flag(widget_discrete_outputs, LV_OBJ_FLAG_HIDDEN);
}

/**
 * @brief Очищает все виджеты подсветки
 * Эта функция должна вызываться перед удалением контейнера
 */
void screen_In_Out_cleanup_highlights(void) {
    ESP_LOGI(TAG, "Cleaning up all highlights");
    
    // Просто сбрасываем указатели, объекты будут удалены вместе с родительским контейнером
    widget_all_in_out_up = NULL;
    widget_all_in_out_down = NULL;
    widget_universal_in_down_left = NULL;
    widget_analog_out_down_right = NULL;
    widget_discrete_outputs = NULL;
}
#include "CO_heating_graph_menu.h"
#include "CO_heating_graph_params.h"
#include "co_params_limits.h"
#include "encoder/encoder.h"
#include "encoder/encoder_manager.h"
#include "my_widgets/w_rad_mask.h"
#include "screen_logic/arc_menu.h"
#include "screen_logic/menu_config.h"
#include "screen_logic/screen_navigation.h"
#include "screen_logic/screen_container_manager.h"
#include "screen_logic/access_control.h"
#include "dialog_screen/screen_YES_NO/yes_no_screen.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CO_HEATING_GRAPH_MENU";

// Forward declarations
static void update_param_display(int param_index);

// Структура элемента меню графика отопления
typedef struct {
    const char *label_text;     // Текст элемента слева
    const void *img_src;        // Иконка (только для "Назад")
    int param_index;            // Индекс параметра (-1 для "Назад")
} CoHeatingGraphMenuItem;

// Элементы меню графика отопления
static const CoHeatingGraphMenuItem co_heating_graph_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Способ задания", NULL, 0},                    // C1-Type (enum)
    {"Угол наклона", NULL, 1},                      // C1-Slope (float)
    {"Количество точек", NULL, 2},                  // C1-Number (int)
    {"Точка 1. Тнв", NULL, 3},                      // C1-T0-1 (float)
    {"Точка 1. Тпод_CO", NULL, 4},                  // C1-T1-Desired-1 (float)
    {"Точка 2. Тнв", NULL, 5},                      // C1-T0-2 (float)
    {"Точка 2. Тпод_CO", NULL, 6},                  // C1-T1-Desired-2 (float)
    {"Точка 3. Тнв", NULL, 7},                      // C1-T0-3 (float)
    {"Точка 3. Тпод_CO", NULL, 8},                  // C1-T1-Desired-3 (float)
    {"Точка 4. Тнв", NULL, 9},                      // C1-T0-4 (float)
    {"Точка 4. Тпод_CO", NULL, 10},                 // C1-T1-Desired-4 (float)
    {"Точка 5. Тнв", NULL, 11},                     // C1-T0-5 (float)
    {"Точка 5. Тпод_CO", NULL, 12},                 // C1-T1-Desired-5 (float)
    {"Точка 6. Тнв", NULL, 13},                     // C1-T0-6 (float)
    {"Точка 6. Тпод_CO", NULL, 14},                 // C1-T1-Desired-6 (float)
    {"Точка 6. Тпод.тс", NULL, 15},                 // C3-T1-6 (float)
    {"Точка 6. Тпод_CO", NULL, 16},                 // C3-T1-Desired-6 (float)
};

// Локальные переменные для меню графика отопления
lv_obj_t *co_heating_graph_cont = NULL;
static bool co_heating_graph_menu_initialized = false;
static bool co_heating_graph_menu_creation_in_progress = false;
static lv_obj_t *co_heating_graph_mask = NULL;

// Массив указателей на label для значений параметров (17 параметров)
static lv_obj_t *value_labels[17] = {NULL};

// Состояние редактирования
static bool edit_mode = false;
static int editing_param_index = -1;
static float editing_float_value = 0.0f;
static int editing_int_value = 0;
static heating_graph_type_t editing_type_value = HEATING_GRAPH_TYPE_POINTS;

// Временные значения для отмены изменений
static float temp_C1_Slope = 0.0f;
static int temp_C1_Number = 0;
static float temp_C1_T0_1 = 0.0f;
static float temp_C1_T1_Desired_1 = 0.0f;
static float temp_C1_T0_2 = 0.0f;
static float temp_C1_T1_Desired_2 = 0.0f;
static float temp_C1_T0_3 = 0.0f;
static float temp_C1_T1_Desired_3 = 0.0f;
static float temp_C1_T0_4 = 0.0f;
static float temp_C1_T1_Desired_4 = 0.0f;
static float temp_C1_T0_5 = 0.0f;
static float temp_C1_T1_Desired_5 = 0.0f;
static float temp_C1_T0_6 = 0.0f;
static float temp_C1_T1_Desired_6 = 0.0f;
static float temp_C3_T1_6 = 0.0f;
static float temp_C3_T1_Desired_6 = 0.0f;
static heating_graph_type_t temp_C1_Type = HEATING_GRAPH_TYPE_POINTS;

/**
 * @brief Проверяет, является ли объект валидным
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Форматирует float значение в строку с одним знаком после точки
 */
static void format_float_value(char *buf, size_t buf_size, float value) {
    if (value < 0.0f) {
        snprintf(buf, buf_size, "-%.1f", -value);
    } else {
        snprintf(buf, buf_size, "%.1f", value);
    }
}

/**
 * @brief Получает строковое представление способа задания
 */
static const char* get_type_string(heating_graph_type_t type) {
    return (type == HEATING_GRAPH_TYPE_POINTS) ? "По точкам" : "По углу";
}

/**
 * @brief Подсветка выбранного элемента меню графика отопления
 */
static void co_heating_graph_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
    if (!is_obj_valid(cont)) return;
    
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!is_obj_valid(child)) continue;
        
        bool is_selected = (i == cursor_index);
        bool is_editing_this = (edit_mode && editing_param_index == co_heating_graph_menu_items[i].param_index);
        
        uint32_t grand_child_cnt = lv_obj_get_child_cnt(child);
        
        for (uint32_t j = 0; j < grand_child_cnt; j++) {
            lv_obj_t *grand_child = lv_obj_get_child(child, j);
            if (!is_obj_valid(grand_child)) continue;
            
            // Проверяем, является ли это контейнером значения параметра
            bool is_value_container = false;
            for (int k = 0; k < 17; k++) {
                if (value_labels[k] != NULL && lv_obj_get_parent(value_labels[k]) == grand_child) {
                    is_value_container = true;
                    break;
                }
            }
            
            if (lv_obj_check_type(grand_child, &lv_label_class)) {
                // Проверяем, является ли это label значения параметра
                bool is_value_label = false;
                for (int k = 0; k < 17; k++) {
                    if (value_labels[k] == grand_child) {
                        is_value_label = true;
                        break;
                    }
                }
                
                if (is_selected) {
                    if (is_value_label && is_editing_this) {
                        // В режиме редактирования не меняем цвет редактируемого значения
                        // (цвет устанавливается в update_param_display)
                    } else {
                        // НЕ в режиме редактирования - черный текст для всей строки (название + значение)
                        lv_obj_set_style_text_color(grand_child, lv_color_hex(0x000000), LV_PART_MAIN);
                    }
                } else {
                    lv_obj_set_style_text_color(grand_child, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                }
            } else if (lv_obj_check_type(grand_child, &lv_image_class)) {
                if (is_selected) {
                    lv_obj_set_style_img_recolor(grand_child, lv_color_hex(0x000000), 0);
                } else {
                    lv_obj_set_style_img_recolor(grand_child, lv_color_hex(0xFFFFFF), 0);
                }
            } else if (is_value_container && is_selected && !is_editing_this) {
                // НЕ в режиме редактирования - устанавливаем желтый фон для контейнера значения
                lv_obj_set_style_bg_color(grand_child, lv_color_hex(0xFFCC00), LV_PART_MAIN);
                // Устанавливаем черный цвет для текста значения параметра
                lv_obj_t *value_label = lv_obj_get_child(grand_child, 0);
                if (is_obj_valid(value_label) && lv_obj_check_type(value_label, &lv_label_class)) {
                    lv_obj_set_style_text_color(value_label, lv_color_hex(0x000000), LV_PART_MAIN);
                }
            } else if (is_value_container && !is_selected) {
                // Не выбранный элемент - обычный фон
                lv_obj_set_style_bg_color(grand_child, lv_color_hex(0x2B3639), LV_PART_MAIN);
                // Восстанавливаем белый цвет для текста значения параметра
                lv_obj_t *value_label = lv_obj_get_child(grand_child, 0);
                if (is_obj_valid(value_label) && lv_obj_check_type(value_label, &lv_label_class)) {
                    lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                }
            }
        }
        
        // Меняем фон контейнера строки
        if (is_selected) {
            // Всегда устанавливаем желтый фон для выбранной строки
            lv_obj_set_style_bg_color(child, lv_color_hex(0xFFCC00), LV_PART_MAIN);
        } else {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x2B3639), LV_PART_MAIN);
        }
    }
}

/**
 * @brief Обновляет отображение значения параметра
 */
static void update_param_display(int param_index) {
    if (param_index < 0 || param_index >= 17) return;
    if (!is_obj_valid(value_labels[param_index])) return;
    
    char value_str[32];
    
    if (edit_mode && editing_param_index == param_index) {
        // В режиме редактирования показываем временное значение
        if (param_index == 0) {
            // Способ задания (enum)
            snprintf(value_str, sizeof(value_str), "%s", get_type_string(editing_type_value));
        } else if (param_index == 2) {
            // Количество точек (int)
            snprintf(value_str, sizeof(value_str), "%d", editing_int_value);
        } else {
            // Float параметры
            format_float_value(value_str, sizeof(value_str), editing_float_value);
        }
    } else {
        // Обычный режим - показываем текущее значение
        switch(param_index) {
            case 0: // C1-Type
                snprintf(value_str, sizeof(value_str), "%s", get_type_string(C1_Type));
                break;
            case 1: // C1-Slope
                format_float_value(value_str, sizeof(value_str), C1_Slope);
                break;
            case 2: // C1-Number
                snprintf(value_str, sizeof(value_str), "%d", C1_Number);
                break;
            case 3: // C1-T0-1
                format_float_value(value_str, sizeof(value_str), C1_T0_1);
                break;
            case 4: // C1-T1-Desired-1
                format_float_value(value_str, sizeof(value_str), C1_T1_Desired_1);
                break;
            case 5: // C1-T0-2
                format_float_value(value_str, sizeof(value_str), C1_T0_2);
                break;
            case 6: // C1-T1-Desired-2
                format_float_value(value_str, sizeof(value_str), C1_T1_Desired_2);
                break;
            case 7: // C1-T0-3
                format_float_value(value_str, sizeof(value_str), C1_T0_3);
                break;
            case 8: // C1-T1-Desired-3
                format_float_value(value_str, sizeof(value_str), C1_T1_Desired_3);
                break;
            case 9: // C1-T0-4
                format_float_value(value_str, sizeof(value_str), C1_T0_4);
                break;
            case 10: // C1-T1-Desired-4
                format_float_value(value_str, sizeof(value_str), C1_T1_Desired_4);
                break;
            case 11: // C1-T0-5
                format_float_value(value_str, sizeof(value_str), C1_T0_5);
                break;
            case 12: // C1-T1-Desired-5
                format_float_value(value_str, sizeof(value_str), C1_T1_Desired_5);
                break;
            case 13: // C1-T0-6
                format_float_value(value_str, sizeof(value_str), C1_T0_6);
                break;
            case 14: // C1-T1-Desired-6
                format_float_value(value_str, sizeof(value_str), C1_T1_Desired_6);
                break;
            case 15: // C3-T1-6
                format_float_value(value_str, sizeof(value_str), C3_T1_6);
                break;
            case 16: // C3-T1-Desired-6
                format_float_value(value_str, sizeof(value_str), C3_T1_Desired_6);
                break;
        }
    }
    
    // Отображаем значение без единиц измерения
    lv_label_set_text(value_labels[param_index], value_str);
    
    // Обновляем цвет в режиме редактирования
    if (!is_obj_valid(value_labels[param_index])) return;
    
    // Убеждаемся, что шрифт установлен такой же, как у названия параметра
    lv_obj_set_style_text_font(value_labels[param_index], &Roboto_bold_24, LV_PART_MAIN);
    
    // Получаем контейнер значения (родитель label)
    lv_obj_t *value_container = lv_obj_get_parent(value_labels[param_index]);
    if (!is_obj_valid(value_container)) return;
    
    if (edit_mode && editing_param_index == param_index) {
        // В режиме редактирования - подсвечиваем только контейнер значения
        lv_obj_set_style_bg_color(value_container, lv_color_hex(0xE9EBEB), LV_PART_MAIN);
        lv_obj_set_style_text_color(value_labels[param_index], lv_color_hex(0x101315), LV_PART_MAIN);
    } else {
        // В обычном режиме - обычный цвет
        lv_obj_set_style_bg_color(value_container, lv_color_hex(0x2B3639), LV_PART_MAIN);
        lv_obj_set_style_text_color(value_labels[param_index], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    }
}


/**
 * @brief Сохраняет изменения параметра
 */
static void save_param_changes(void) {
    ESP_LOGI(TAG, "Saving parameter changes for index %d", editing_param_index);
    
    int saved_index = editing_param_index;
    
    if (editing_param_index == 0) {
        // Способ задания
        ESP_LOGI(TAG, "Saving C1_Type: %d -> %d", C1_Type, editing_type_value);
        C1_Type = editing_type_value;
    } else if (editing_param_index == 2) {
        // Количество точек
        ESP_LOGI(TAG, "Saving C1_Number: %d -> %d", C1_Number, editing_int_value);
        C1_Number = editing_int_value;
    } else {
        // Float параметры
        float old_value = 0.0f;
        switch(editing_param_index) {
            case 1: old_value = C1_Slope; C1_Slope = editing_float_value; break;
            case 3: old_value = C1_T0_1; C1_T0_1 = editing_float_value; break;
            case 4: old_value = C1_T1_Desired_1; C1_T1_Desired_1 = editing_float_value; break;
            case 5: old_value = C1_T0_2; C1_T0_2 = editing_float_value; break;
            case 6: old_value = C1_T1_Desired_2; C1_T1_Desired_2 = editing_float_value; break;
            case 7: old_value = C1_T0_3; C1_T0_3 = editing_float_value; break;
            case 8: old_value = C1_T1_Desired_3; C1_T1_Desired_3 = editing_float_value; break;
            case 9: old_value = C1_T0_4; C1_T0_4 = editing_float_value; break;
            case 10: old_value = C1_T1_Desired_4; C1_T1_Desired_4 = editing_float_value; break;
            case 11: old_value = C1_T0_5; C1_T0_5 = editing_float_value; break;
            case 12: old_value = C1_T1_Desired_5; C1_T1_Desired_5 = editing_float_value; break;
            case 13: old_value = C1_T0_6; C1_T0_6 = editing_float_value; break;
            case 14: old_value = C1_T1_Desired_6; C1_T1_Desired_6 = editing_float_value; break;
            case 15: old_value = C3_T1_6; C3_T1_6 = editing_float_value; break;
            case 16: old_value = C3_T1_Desired_6; C3_T1_Desired_6 = editing_float_value; break;
        }
        ESP_LOGI(TAG, "Saving float param %d: %.1f -> %.1f", editing_param_index, old_value, editing_float_value);
    }
    
    // НЕ сохраняем параметры в NVS (как в ГВС)
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_HEATING_GRAPH);
    if (menu_state) {
        co_heating_graph_highlight_box(co_heating_graph_cont, menu_state->cursor_index);
    }
}

/**
 * @brief Отменяет изменения параметра
 */
static void cancel_param_changes(void) {
    ESP_LOGI(TAG, "Canceling parameter changes");
    
    int saved_index = editing_param_index;
    
    // Восстанавливаем временные значения
    if (editing_param_index == 0) {
        editing_type_value = temp_C1_Type;
    } else if (editing_param_index == 2) {
        editing_int_value = temp_C1_Number;
    } else {
        switch(editing_param_index) {
            case 1: editing_float_value = temp_C1_Slope; break;
            case 3: editing_float_value = temp_C1_T0_1; break;
            case 4: editing_float_value = temp_C1_T1_Desired_1; break;
            case 5: editing_float_value = temp_C1_T0_2; break;
            case 6: editing_float_value = temp_C1_T1_Desired_2; break;
            case 7: editing_float_value = temp_C1_T0_3; break;
            case 8: editing_float_value = temp_C1_T1_Desired_3; break;
            case 9: editing_float_value = temp_C1_T0_4; break;
            case 10: editing_float_value = temp_C1_T1_Desired_4; break;
            case 11: editing_float_value = temp_C1_T0_5; break;
            case 12: editing_float_value = temp_C1_T1_Desired_5; break;
            case 13: editing_float_value = temp_C1_T0_6; break;
            case 14: editing_float_value = temp_C1_T1_Desired_6; break;
            case 15: editing_float_value = temp_C3_T1_6; break;
            case 16: editing_float_value = temp_C3_T1_Desired_6; break;
        }
    }
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_HEATING_GRAPH);
    if (menu_state) {
        co_heating_graph_highlight_box(co_heating_graph_cont, menu_state->cursor_index);
    }
}

/**
 * @brief Входит в режим редактирования параметра
 */
static void enter_edit_mode(int param_index) {
    if (param_index < 0 || param_index >= 17) return;
    
    // Проверяем доступ перед редактированием
    if (!access_control_is_unlocked()) {
        ESP_LOGW(TAG, "Access denied: cannot edit parameters when access is locked");
        return;
    }
    
    ESP_LOGI(TAG, "Entering edit mode for parameter %d", param_index);
    
    edit_mode = true;
    editing_param_index = param_index;
    
    // Сохраняем текущие значения как временные
    if (param_index == 0) {
        temp_C1_Type = C1_Type;
        editing_type_value = C1_Type;
    } else if (param_index == 2) {
        temp_C1_Number = C1_Number;
        editing_int_value = C1_Number;
    } else {
        switch(param_index) {
            case 1: 
                temp_C1_Slope = C1_Slope;
                editing_float_value = C1_Slope;
                break;
            case 3:
                temp_C1_T0_1 = C1_T0_1;
                editing_float_value = C1_T0_1;
                break;
            case 4:
                temp_C1_T1_Desired_1 = C1_T1_Desired_1;
                editing_float_value = C1_T1_Desired_1;
                break;
            case 5:
                temp_C1_T0_2 = C1_T0_2;
                editing_float_value = C1_T0_2;
                break;
            case 6:
                temp_C1_T1_Desired_2 = C1_T1_Desired_2;
                editing_float_value = C1_T1_Desired_2;
                break;
            case 7:
                temp_C1_T0_3 = C1_T0_3;
                editing_float_value = C1_T0_3;
                break;
            case 8:
                temp_C1_T1_Desired_3 = C1_T1_Desired_3;
                editing_float_value = C1_T1_Desired_3;
                break;
            case 9:
                temp_C1_T0_4 = C1_T0_4;
                editing_float_value = C1_T0_4;
                break;
            case 10:
                temp_C1_T1_Desired_4 = C1_T1_Desired_4;
                editing_float_value = C1_T1_Desired_4;
                break;
            case 11:
                temp_C1_T0_5 = C1_T0_5;
                editing_float_value = C1_T0_5;
                break;
            case 12:
                temp_C1_T1_Desired_5 = C1_T1_Desired_5;
                editing_float_value = C1_T1_Desired_5;
                break;
            case 13:
                temp_C1_T0_6 = C1_T0_6;
                editing_float_value = C1_T0_6;
                break;
            case 14:
                temp_C1_T1_Desired_6 = C1_T1_Desired_6;
                editing_float_value = C1_T1_Desired_6;
                break;
            case 15:
                temp_C3_T1_6 = C3_T1_6;
                editing_float_value = C3_T1_6;
                break;
            case 16:
                temp_C3_T1_Desired_6 = C3_T1_Desired_6;
                editing_float_value = C3_T1_Desired_6;
                break;
        }
    }
    
    update_param_display(param_index);
}

/**
 * @brief Выходит из режима редактирования и проверяет изменения
 */
static void exit_edit_mode_with_confirmation(void) {
    if (!edit_mode || editing_param_index < 0) {
        ESP_LOGW(TAG, "exit_edit_mode_with_confirmation: edit_mode=%d, editing_param_index=%d", edit_mode, editing_param_index);
        return;
    }
    
    ESP_LOGI(TAG, "exit_edit_mode_with_confirmation: checking changes for param_index=%d", editing_param_index);
    
    bool value_changed = false;
    
    // Проверяем, изменилось ли значение
    if (editing_param_index == 0) {
        value_changed = (editing_type_value != temp_C1_Type);
        ESP_LOGI(TAG, "Type changed: %d != %d -> %s", editing_type_value, temp_C1_Type, value_changed ? "YES" : "NO");
    } else if (editing_param_index == 2) {
        value_changed = (editing_int_value != temp_C1_Number);
        ESP_LOGI(TAG, "Number changed: %d != %d -> %s", editing_int_value, temp_C1_Number, value_changed ? "YES" : "NO");
    } else {
        float *temp_values[] = {
            NULL, &temp_C1_Slope, NULL, &temp_C1_T0_1, &temp_C1_T1_Desired_1,
            &temp_C1_T0_2, &temp_C1_T1_Desired_2, &temp_C1_T0_3, &temp_C1_T1_Desired_3,
            &temp_C1_T0_4, &temp_C1_T1_Desired_4, &temp_C1_T0_5, &temp_C1_T1_Desired_5,
            &temp_C1_T0_6, &temp_C1_T1_Desired_6, &temp_C3_T1_6, &temp_C3_T1_Desired_6
        };
        if (temp_values[editing_param_index] != NULL) {
            float diff = fabs(editing_float_value - *temp_values[editing_param_index]);
            value_changed = (diff > 0.01f);
            ESP_LOGI(TAG, "Float param %d changed: %.3f != %.3f (diff=%.3f) -> %s", 
                     editing_param_index, editing_float_value, *temp_values[editing_param_index], 
                     diff, value_changed ? "YES" : "NO");
        } else {
            ESP_LOGE(TAG, "temp_values[%d] is NULL!", editing_param_index);
        }
    }
    
    if (value_changed) {
        ESP_LOGI(TAG, "Value changed, showing confirmation dialog");
        // Вызываем окно подтверждения
        create_yes_no_screen_with_callbacks(save_param_changes, cancel_param_changes);
    } else {
        ESP_LOGI(TAG, "Value not changed, canceling without save");
        // Значение не изменилось, просто выходим из режима редактирования
        cancel_param_changes();
    }
}

/**
 * @brief Создание элемента меню графика отопления
 */
static void create_co_heating_graph_menu_item(lv_obj_t *cont, const CoHeatingGraphMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_co_heating_graph_menu_item");
        return;
    }
    
    // Создаем контейнер для элемента (высота 40px)
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_co_heating_graph_menu_item");
        return;
    }
    
    lv_obj_set_size(box, 462, 40);
    lv_obj_set_style_border_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_border_width(box, 0, 0);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_radius(box, 0, 0);
    
    
    // Основная надпись слева
    lv_obj_t *label = lv_label_create(box);
    if (is_obj_valid(label)) {
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
        lv_label_set_text(label, item->label_text);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, -5, 0);
    }
    
    // Иконка (только для "Назад")
    if (item->img_src != NULL) {
        lv_obj_t *img = lv_img_create(box);
        if (is_obj_valid(img)) {
            lv_img_set_src(img, item->img_src);
            lv_obj_set_style_img_recolor(img, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
            lv_obj_align(img, LV_ALIGN_CENTER, 90, 0);
        }
    }
    
    // Значение параметра справа (только для редактируемых параметров)
    if (item->param_index >= 0) {
        // Создаем контейнер для значения параметра
        lv_obj_t *value_container = lv_obj_create(box);
        if (is_obj_valid(value_container)) {
            lv_obj_set_size(value_container, 83, 40);
            lv_obj_set_style_bg_color(value_container, lv_color_hex(0x2B3639), LV_PART_MAIN);
            lv_obj_set_style_border_color(value_container, lv_color_hex(0x2B3639), LV_PART_MAIN);
            lv_obj_set_style_border_width(value_container, 0, 0);
            lv_obj_set_style_radius(value_container, 0, 0);
            lv_obj_set_style_pad_all(value_container, 0, 0);
            // Выравниваем контейнер значения относительно названия параметра (смещение 240px от левого края)
            lv_obj_set_pos(value_container, 240, -23);
            
            // Помечаем контейнер значения параметра для компенсации движения по дуге
            set_as_param_value(value_container);
            
            lv_obj_t *value_label = lv_label_create(value_container);
            if (is_obj_valid(value_label)) {
                value_labels[item->param_index] = value_label;
                lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_font(value_label, &Roboto_bold_24, 0);
                lv_obj_align(value_label, LV_ALIGN_BOTTOM_RIGHT, 0, -2);
                update_param_display(item->param_index);
            }
        }
    }
    
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);
    vTaskDelay(pdMS_TO_TICKS(1));
}

/**
 * @brief Показывает меню графика отопления
 */
void co_heating_graph_menu_show(void) {
    ESP_LOGI(TAG, "Showing CO heating graph menu");
    if (is_obj_valid(co_heating_graph_cont)) {
        lv_obj_clear_flag(co_heating_graph_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(co_heating_graph_mask)) {
        co_heating_graph_mask = radial();
        if (is_obj_valid(co_heating_graph_mask)) {
            lv_obj_set_pos(co_heating_graph_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(co_heating_graph_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает меню графика отопления
 */
void co_heating_graph_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding CO heating graph menu");
    if (is_obj_valid(co_heating_graph_cont)) {
        lv_obj_add_flag(co_heating_graph_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(co_heating_graph_mask)) {
        lv_obj_del(co_heating_graph_mask);
        co_heating_graph_mask = NULL;
    }
}

/**
 * @brief Обработчик событий энкодера для меню графика отопления
 */
void co_heating_graph_menu_encoder_event_cb(uint8_t e) {
    // Если активно окно подтверждения, передаем события ему
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(co_heating_graph_cont)) {
        ESP_LOGE(TAG, "Контейнер меню графика отопления не инициализирован");
        return;
    }
    
    // Если в режиме редактирования
    if (edit_mode && editing_param_index >= 0) {
        if (e & ENC_LEFT) {
            if (editing_param_index == 0) {
                // Способ задания - переключаем между вариантами
                editing_type_value = (editing_type_value == HEATING_GRAPH_TYPE_POINTS) ? 
                                     HEATING_GRAPH_TYPE_SLOPE : HEATING_GRAPH_TYPE_POINTS;
            } else if (editing_param_index == 2) {
                // Количество точек - уменьшаем
                int step = co_heating_graph_param_limits_int[0].step;
                editing_int_value -= step;
                if (editing_int_value < co_heating_graph_param_limits_int[0].min) {
                    editing_int_value = co_heating_graph_param_limits_int[0].min;
                }
            } else {
                // Float параметры - уменьшаем на step
                // Маппинг: param_index 1->float_index 0, param_index 3->float_index 1, param_index 4->float_index 2, ...
                int float_index = (editing_param_index == 1) ? 0 : editing_param_index - 2;
                if (float_index >= 0 && float_index < PARAM_LIMITS_HEATING_GRAPH_COUNT) {
                    float step = co_heating_graph_param_limits_float[float_index].step;
                    editing_float_value -= step;
                    if (editing_float_value < co_heating_graph_param_limits_float[float_index].min) {
                        editing_float_value = co_heating_graph_param_limits_float[float_index].min;
                    }
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_RIGHT) {
            if (editing_param_index == 0) {
                // Способ задания - переключаем между вариантами
                editing_type_value = (editing_type_value == HEATING_GRAPH_TYPE_POINTS) ? 
                                     HEATING_GRAPH_TYPE_SLOPE : HEATING_GRAPH_TYPE_POINTS;
            } else if (editing_param_index == 2) {
                // Количество точек - увеличиваем
                int step = co_heating_graph_param_limits_int[0].step;
                editing_int_value += step;
                if (editing_int_value > co_heating_graph_param_limits_int[0].max) {
                    editing_int_value = co_heating_graph_param_limits_int[0].max;
                }
            } else {
                // Float параметры - увеличиваем на step
                // Маппинг: param_index 1->float_index 0, param_index 3->float_index 1, param_index 4->float_index 2, ...
                int float_index = (editing_param_index == 1) ? 0 : editing_param_index - 2;
                if (float_index >= 0 && float_index < PARAM_LIMITS_HEATING_GRAPH_COUNT) {
                    float step = co_heating_graph_param_limits_float[float_index].step;
                    editing_float_value += step;
                    if (editing_float_value > co_heating_graph_param_limits_float[float_index].max) {
                        editing_float_value = co_heating_graph_param_limits_float[float_index].max;
                    }
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_CLICK) {
            // Выходим из режима редактирования
            exit_edit_mode_with_confirmation();
        }
        return;
    }
    
    // Обычный режим навигации - используем стандартную функцию arc_menu
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_HEATING_GRAPH);
    arc_menu_handle_encoder(e, co_heating_graph_cont, menu_state, MENU_TYPE_CO_HEATING_GRAPH);
    
    // Обновляем подсветку
    co_heating_graph_highlight_box(co_heating_graph_cont, menu_state->cursor_index);
    
    // Обработка нажатия кнопки
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в меню отопления
            ESP_LOGI(TAG, "Returning to CO menu from heating graph menu");
            co_heating_graph_menu_hide();
            extern void co_menu_show(void);
            co_menu_show();
            // Переключаем обработчик энкодера
            extern void co_menu_encoder_event_cb(uint8_t e);
            encoder_manager_register_callback(co_menu_encoder_event_cb);
        } else {
            // Входим в режим редактирования
            int param_index = co_heating_graph_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_edit_mode(param_index);
            }
        }
    }
}

/**
 * @brief Очистка меню графика отопления
 */
void co_heating_graph_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up CO heating graph menu");
    
    co_heating_graph_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    // Очищаем массив указателей на labels
    for (int i = 0; i < 17; i++) {
        value_labels[i] = NULL;
    }
    
    if (is_obj_valid(co_heating_graph_mask)) {
        lv_obj_del(co_heating_graph_mask);
        co_heating_graph_mask = NULL;
    }
    
    if (is_obj_valid(co_heating_graph_cont)) {
        lv_obj_del(co_heating_graph_cont);
        co_heating_graph_cont = NULL;
    }
    
    co_heating_graph_menu_initialized = false;
}

/**
 * @brief Инициализация меню графика отопления
 */
void CO_Heating_Graph_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню графика отопления");
    
    if (co_heating_graph_menu_creation_in_progress) {
        ESP_LOGW(TAG, "CO heating graph menu creation already in progress, skipping");
        return;
    }
    
    co_heating_graph_menu_creation_in_progress = true;
    
    if (co_heating_graph_menu_initialized && is_obj_valid(co_heating_graph_cont)) {
        ESP_LOGI(TAG, "CO heating graph menu already initialized, showing it");
        co_heating_graph_menu_show();
        co_heating_graph_menu_creation_in_progress = false;
        return;
    }
    
    co_heating_graph_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    co_heating_graph_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(co_heating_graph_cont)) {
        ESP_LOGE(TAG, "Failed to create CO heating graph menu container");
        co_heating_graph_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(co_heating_graph_cont, 1200, 1200);
    lv_obj_center(co_heating_graph_cont);
    lv_obj_add_event_cb(co_heating_graph_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(co_heating_graph_cont, &style, 0);
    lv_obj_set_style_radius(co_heating_graph_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(co_heating_graph_cont, true, 0);
    lv_obj_set_scroll_dir(co_heating_graph_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(co_heating_graph_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(co_heating_graph_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(co_heating_graph_cont, 633, 0);
    lv_obj_set_style_bg_color(co_heating_graph_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(co_heating_graph_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(co_heating_graph_cont, 0, 0);
    lv_obj_set_style_pad_row(co_heating_graph_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(co_heating_graph_menu_items) / sizeof(CoHeatingGraphMenuItem); i++) {
        create_co_heating_graph_menu_item(co_heating_graph_cont, &co_heating_graph_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    co_heating_graph_mask = radial();
    if (is_obj_valid(co_heating_graph_mask)) {
        lv_obj_set_pos(co_heating_graph_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_CO_HEATING_GRAPH);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_HEATING_GRAPH);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(co_heating_graph_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    co_heating_graph_highlight_box(co_heating_graph_cont, menu_state->cursor_index);
    arc_menu_update_slide(co_heating_graph_cont);
    
    co_heating_graph_menu_initialized = true;
    co_heating_graph_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню графика отопления успешно инициализировано");
}


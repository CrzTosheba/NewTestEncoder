#include "CO_pumps_menu.h"
#include "CO_pumps_params.h"
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

static const char *TAG = "CO_PUMPS_MENU";

// Forward declarations
static void update_param_display(int param_index);

// Структура элемента меню насосов
typedef struct {
    const char *label_text;     // Текст элемента слева
    const void *img_src;        // Иконка (только для "Назад")
    int param_index;            // Индекс параметра (-1 для "Назад")
} CoPumpsMenuItem;

// Элементы меню насосов
static const CoPumpsMenuItem co_pumps_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Количество насосов", NULL, 0},                    // N-Number (int)
    {"Пауза перед старт", NULL, 1},                   // N-BeforeStartPause (int)
    {"Пауза перед стоп", NULL, 2},                   // N-BeforeStopPause (int)
    {"Пауза переключ.", NULL, 3},                    // N-ChangeOverPause (int)
    {"Режим переключения", NULL, 4},                    // N-ChangeMode (enum)
    {"Период работы", NULL, 5},                       // N-ChangeWHours (int)
    {"Период работы", NULL, 6},                    // N-ChangeWDays (int)
    {"Время переключ.", NULL, 7},                    // N-ChangeHours (int)
    {"Время переключ.", NULL, 8},                   // N-ChangeMinutes (int)
    {"Сброс.наработку Н1", NULL, 9},                    // N1-ResetWHours (enum)
    {"Время наработки Н1", NULL, 10},                  // N1-WHours (int)
    {"Кол-во запусков Н1", NULL, 11},                    // N1-WStarts (int)
    {"Сброс.наработку Н2", NULL, 12},                    // N2-ResetWHours (enum)
    {"Время наработки Н2", NULL, 13},                  // N2-WHours (int)
    {"Кол-во запусков Н2", NULL, 14},                    // N2-WStarts (int)
    {"Тренировать", NULL, 15},                           // N-Training-En (enum)
    {"Период тренировки", NULL, 16},                  // N-Training-Period (int)
};

// Локальные переменные для меню насосов
lv_obj_t *co_pumps_cont = NULL;
static bool co_pumps_menu_initialized = false;
static bool co_pumps_menu_creation_in_progress = false;
static lv_obj_t *co_pumps_mask = NULL;

// Массив указателей на label для значений параметров (17 параметров)
static lv_obj_t *value_labels[17] = {NULL};

// Состояние редактирования
static bool edit_mode = false;
static int editing_param_index = -1;
static int editing_int_value = 0;
static pump_change_mode_t editing_change_mode_value = PUMP_CHANGE_MODE_TIME;
static pump_reset_t editing_reset1_value = PUMP_RESET_OFF;
static pump_reset_t editing_reset2_value = PUMP_RESET_OFF;
static pump_training_t editing_training_value = PUMP_TRAINING_OFF;

// Временные значения для отмены изменений
static int temp_N_Number = 0;
static int temp_N_BeforeStartPause = 0;
static int temp_N_BeforeStopPause = 0;
static int temp_N_ChangeOverPause = 0;
static pump_change_mode_t temp_N_ChangeMode = PUMP_CHANGE_MODE_TIME;
static int temp_N_ChangeWHours = 0;
static int temp_N_ChangeWDays = 0;
static int temp_N_ChangeHours = 0;
static int temp_N_ChangeMinutes = 0;
static pump_reset_t temp_N1_ResetWHours = PUMP_RESET_OFF;
static int temp_N1_WHours = 0;
static int temp_N1_WStarts = 0;
static pump_reset_t temp_N2_ResetWHours = PUMP_RESET_OFF;
static int temp_N2_WHours = 0;
static int temp_N2_WStarts = 0;
static pump_training_t temp_N_Training_En = PUMP_TRAINING_OFF;
static int temp_N_Training_Period = 0;

/**
 * @brief Проверяет, является ли объект валидным
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Получает строковое представление режима переключения
 */
static const char* get_change_mode_string(pump_change_mode_t mode) {
    return (mode == PUMP_CHANGE_MODE_TIME) ? "ЧАСЫ" : "ДЕНЬ";
}

/**
 * @brief Получает строковое представление сброса
 */
static const char* get_reset_string(pump_reset_t reset) {
    return (reset == PUMP_RESET_OFF) ? "НЕТ" : "ДА";
}

/**
 * @brief Получает строковое представление тренировки
 */
static const char* get_training_string(pump_training_t training) {
    return (training == PUMP_TRAINING_OFF) ? "ВЫКЛ" : "ВКЛ";
}

/**
 * @brief Получает строковое представление количества насосов
 */
static const char* get_pump_number_string(int number) {
    switch(number) {
        case 0: return "НЕТ";
        case 1: return "1";
        case 2: return "2";
        default: return "???";
    }
}

/**
 * @brief Подсветка выбранного элемента меню насосов
 */
static void co_pumps_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
    if (!is_obj_valid(cont)) return;
    
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!is_obj_valid(child)) continue;
        
        bool is_selected = (i == cursor_index);
        bool is_editing_this = (edit_mode && editing_param_index == co_pumps_menu_items[i].param_index);
        
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
        switch(param_index) {
            case 0: // N-Number (enum: НЕТ/1/2)
                snprintf(value_str, sizeof(value_str), "%s", get_pump_number_string(editing_int_value));
                break;
            case 4: // N-ChangeMode (enum)
                snprintf(value_str, sizeof(value_str), "%s", get_change_mode_string(editing_change_mode_value));
                break;
            case 9: // N1-ResetWHours (enum)
                snprintf(value_str, sizeof(value_str), "%s", get_reset_string(editing_reset1_value));
                break;
            case 12: // N2-ResetWHours (enum)
                snprintf(value_str, sizeof(value_str), "%s", get_reset_string(editing_reset2_value));
                break;
            case 15: // N-Training-En (enum)
                snprintf(value_str, sizeof(value_str), "%s", get_training_string(editing_training_value));
                break;
            default: // Int параметры
                snprintf(value_str, sizeof(value_str), "%d", editing_int_value);
                break;
        }
    } else {
        // Обычный режим - показываем текущее значение
        switch(param_index) {
            case 0: // N-Number
                snprintf(value_str, sizeof(value_str), "%s", get_pump_number_string(N_Number));
                break;
            case 1: // N-BeforeStartPause
                snprintf(value_str, sizeof(value_str), "%d", N_BeforeStartPause);
                break;
            case 2: // N-BeforeStopPause
                snprintf(value_str, sizeof(value_str), "%d", N_BeforeStopPause);
                break;
            case 3: // N-ChangeOverPause
                snprintf(value_str, sizeof(value_str), "%d", N_ChangeOverPause);
                break;
            case 4: // N-ChangeMode
                snprintf(value_str, sizeof(value_str), "%s", get_change_mode_string(N_ChangeMode));
                break;
            case 5: // N-ChangeWHours
                snprintf(value_str, sizeof(value_str), "%d", N_ChangeWHours);
                break;
            case 6: // N-ChangeWDays
                snprintf(value_str, sizeof(value_str), "%d", N_ChangeWDays);
                break;
            case 7: // N-ChangeHours
                snprintf(value_str, sizeof(value_str), "%d", N_ChangeHours);
                break;
            case 8: // N-ChangeMinutes
                snprintf(value_str, sizeof(value_str), "%d", N_ChangeMinutes);
                break;
            case 9: // N1-ResetWHours
                snprintf(value_str, sizeof(value_str), "%s", get_reset_string(N1_ResetWHours));
                break;
            case 10: // N1-WHours (только для отображения)
                snprintf(value_str, sizeof(value_str), "%d", N1_WHours);
                break;
            case 11: // N1-WStarts (только для отображения)
                snprintf(value_str, sizeof(value_str), "%d", N1_WStarts);
                break;
            case 12: // N2-ResetWHours
                snprintf(value_str, sizeof(value_str), "%s", get_reset_string(N2_ResetWHours));
                break;
            case 13: // N2-WHours (только для отображения)
                snprintf(value_str, sizeof(value_str), "%d", N2_WHours);
                break;
            case 14: // N2-WStarts (только для отображения)
                snprintf(value_str, sizeof(value_str), "%d", N2_WStarts);
                break;
            case 15: // N-Training-En
                snprintf(value_str, sizeof(value_str), "%s", get_training_string(N_Training_En));
                break;
            case 16: // N-Training-Period
                snprintf(value_str, sizeof(value_str), "%d", N_Training_Period);
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
    
    // Сохраняем значение в зависимости от типа параметра
    switch(editing_param_index) {
        case 0: N_Number = editing_int_value; break;
        case 1: N_BeforeStartPause = editing_int_value; break;
        case 2: N_BeforeStopPause = editing_int_value; break;
        case 3: N_ChangeOverPause = editing_int_value; break;
        case 4: N_ChangeMode = editing_change_mode_value; break;
        case 5: N_ChangeWHours = editing_int_value; break;
        case 6: N_ChangeWDays = editing_int_value; break;
        case 7: N_ChangeHours = editing_int_value; break;
        case 8: N_ChangeMinutes = editing_int_value; break;
        case 9: N1_ResetWHours = editing_reset1_value; break;
        // case 10, 11, 13, 14 - только для отображения, не сохраняем
        case 12: N2_ResetWHours = editing_reset2_value; break;
        // case 13, 14 - только для отображения, не сохраняем
        case 15: N_Training_En = editing_training_value; break;
        case 16: N_Training_Period = editing_int_value; break;
    }
    
    // НЕ сохраняем параметры в NVS (как в ГВС)
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_PUMPS);
    if (menu_state) {
        co_pumps_highlight_box(co_pumps_cont, menu_state->cursor_index);
    }
}

/**
 * @brief Отменяет изменения параметра
 */
static void cancel_param_changes(void) {
    ESP_LOGI(TAG, "Canceling parameter changes");
    
    int saved_index = editing_param_index;
    
    // Восстанавливаем временные значения
    switch(editing_param_index) {
        case 0: editing_int_value = temp_N_Number; break;
        case 1: editing_int_value = temp_N_BeforeStartPause; break;
        case 2: editing_int_value = temp_N_BeforeStopPause; break;
        case 3: editing_int_value = temp_N_ChangeOverPause; break;
        case 4: editing_change_mode_value = temp_N_ChangeMode; break;
        case 5: editing_int_value = temp_N_ChangeWHours; break;
        case 6: editing_int_value = temp_N_ChangeWDays; break;
        case 7: editing_int_value = temp_N_ChangeHours; break;
        case 8: editing_int_value = temp_N_ChangeMinutes; break;
        case 9: editing_reset1_value = temp_N1_ResetWHours; break;
        case 10: editing_int_value = temp_N1_WHours; break;
        case 11: editing_int_value = temp_N1_WStarts; break;
        case 12: editing_reset2_value = temp_N2_ResetWHours; break;
        case 13: editing_int_value = temp_N2_WHours; break;
        case 14: editing_int_value = temp_N2_WStarts; break;
        case 15: editing_training_value = temp_N_Training_En; break;
        case 16: editing_int_value = temp_N_Training_Period; break;
    }
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_PUMPS);
    if (menu_state) {
        co_pumps_highlight_box(co_pumps_cont, menu_state->cursor_index);
    }
}

/**
 * @brief Входит в режим редактирования параметра
 */
static void enter_edit_mode(int param_index) {
    if (param_index < 0 || param_index >= 17) return;
    
    // Параметры только для отображения не редактируются
    if (param_index == 10 || param_index == 11 || param_index == 13 || param_index == 14) {
        ESP_LOGI(TAG, "Parameter %d is read-only (display only)", param_index);
        return;
    }
    
    // Проверяем доступ перед редактированием
    if (!access_control_is_unlocked()) {
        ESP_LOGW(TAG, "Access denied: cannot edit parameters when access is locked");
        return;
    }
    
    ESP_LOGI(TAG, "Entering edit mode for parameter %d", param_index);
    
    edit_mode = true;
    editing_param_index = param_index;
    
    // Сохраняем текущие значения как временные
    switch(param_index) {
        case 0:
            temp_N_Number = N_Number;
            editing_int_value = N_Number;
            break;
        case 1:
            temp_N_BeforeStartPause = N_BeforeStartPause;
            editing_int_value = N_BeforeStartPause;
            break;
        case 2:
            temp_N_BeforeStopPause = N_BeforeStopPause;
            editing_int_value = N_BeforeStopPause;
            break;
        case 3:
            temp_N_ChangeOverPause = N_ChangeOverPause;
            editing_int_value = N_ChangeOverPause;
            break;
        case 4:
            temp_N_ChangeMode = N_ChangeMode;
            editing_change_mode_value = N_ChangeMode;
            break;
        case 5:
            temp_N_ChangeWHours = N_ChangeWHours;
            editing_int_value = N_ChangeWHours;
            break;
        case 6:
            temp_N_ChangeWDays = N_ChangeWDays;
            editing_int_value = N_ChangeWDays;
            break;
        case 7:
            temp_N_ChangeHours = N_ChangeHours;
            editing_int_value = N_ChangeHours;
            break;
        case 8:
            temp_N_ChangeMinutes = N_ChangeMinutes;
            editing_int_value = N_ChangeMinutes;
            break;
        case 9:
            temp_N1_ResetWHours = N1_ResetWHours;
            editing_reset1_value = N1_ResetWHours;
            break;
        // case 10, 11, 13, 14 - только для отображения, не редактируются
        case 12:
            temp_N2_ResetWHours = N2_ResetWHours;
            editing_reset2_value = N2_ResetWHours;
            break;
        // case 13, 14 - только для отображения, не редактируются
        case 15:
            temp_N_Training_En = N_Training_En;
            editing_training_value = N_Training_En;
            break;
        case 16:
            temp_N_Training_Period = N_Training_Period;
            editing_int_value = N_Training_Period;
            break;
    }
    
    update_param_display(param_index);
}

/**
 * @brief Выходит из режима редактирования и проверяет изменения
 */
static void exit_edit_mode_with_confirmation(void) {
    if (!edit_mode || editing_param_index < 0) return;
    
    bool value_changed = false;
    
    // Проверяем, изменилось ли значение
    switch(editing_param_index) {
        case 4:
            value_changed = (editing_change_mode_value != temp_N_ChangeMode);
            break;
        case 9:
            value_changed = (editing_reset1_value != temp_N1_ResetWHours);
            break;
        case 12:
            value_changed = (editing_reset2_value != temp_N2_ResetWHours);
            break;
        case 15:
            value_changed = (editing_training_value != temp_N_Training_En);
            break;
        default:
            // Int параметры
            switch(editing_param_index) {
                case 0: value_changed = (editing_int_value != temp_N_Number); break;
                case 1: value_changed = (editing_int_value != temp_N_BeforeStartPause); break;
                case 2: value_changed = (editing_int_value != temp_N_BeforeStopPause); break;
                case 3: value_changed = (editing_int_value != temp_N_ChangeOverPause); break;
                case 5: value_changed = (editing_int_value != temp_N_ChangeWHours); break;
                case 6: value_changed = (editing_int_value != temp_N_ChangeWDays); break;
                case 7: value_changed = (editing_int_value != temp_N_ChangeHours); break;
                case 8: value_changed = (editing_int_value != temp_N_ChangeMinutes); break;
                case 10: value_changed = (editing_int_value != temp_N1_WHours); break;
                case 11: value_changed = (editing_int_value != temp_N1_WStarts); break;
                case 13: value_changed = (editing_int_value != temp_N2_WHours); break;
                case 14: value_changed = (editing_int_value != temp_N2_WStarts); break;
                case 16: value_changed = (editing_int_value != temp_N_Training_Period); break;
            }
            break;
    }
    
    if (value_changed) {
        // Вызываем окно подтверждения
        create_yes_no_screen_with_callbacks(save_param_changes, cancel_param_changes);
    } else {
        // Значение не изменилось, просто выходим из режима редактирования
        cancel_param_changes();
    }
}

/**
 * @brief Создание элемента меню насосов
 */
static void create_co_pumps_menu_item(lv_obj_t *cont, const CoPumpsMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_co_pumps_menu_item");
        return;
    }
    
    // Создаем контейнер для элемента (высота 40px)
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_co_pumps_menu_item");
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
            // Выравниваем контейнер значения относительно названия параметра (смещение 200px от левого края)
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
 * @brief Показывает меню насосов
 */
void co_pumps_menu_show(void) {
    ESP_LOGI(TAG, "Showing CO pumps menu");
    if (is_obj_valid(co_pumps_cont)) {
        lv_obj_clear_flag(co_pumps_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(co_pumps_mask)) {
        co_pumps_mask = radial();
        if (is_obj_valid(co_pumps_mask)) {
            lv_obj_set_pos(co_pumps_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(co_pumps_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает меню насосов
 */
void co_pumps_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding CO pumps menu");
    if (is_obj_valid(co_pumps_cont)) {
        lv_obj_add_flag(co_pumps_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(co_pumps_mask)) {
        lv_obj_del(co_pumps_mask);
        co_pumps_mask = NULL;
    }
}

/**
 * @brief Обработчик событий энкодера для меню насосов
 */
void co_pumps_menu_encoder_event_cb(uint8_t e) {
    // Обновляем таймер активности при любом действии пользователя
    access_control_update_activity_timer();
    
    // Если активно окно подтверждения, передаем события ему
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(co_pumps_cont)) {
        ESP_LOGE(TAG, "Контейнер меню насосов не инициализирован");
        return;
    }
    
    // Если в режиме редактирования
    if (edit_mode && editing_param_index >= 0) {
        if (e & ENC_LEFT) {
            // Уменьшаем значение
            if (editing_param_index == 0) {
                // N-Number - переключаем циклически: НЕТ -> 2 -> 1 -> НЕТ
                if (editing_int_value == 0) {
                    editing_int_value = 2;  // НЕТ -> 2
                } else {
                    editing_int_value--;    // 2 -> 1, 1 -> 0 (НЕТ)
                }
            } else if (editing_param_index == 4) {
                // N-ChangeMode - переключаем enum
                editing_change_mode_value = (editing_change_mode_value == PUMP_CHANGE_MODE_TIME) ? 
                                           PUMP_CHANGE_MODE_WORK : PUMP_CHANGE_MODE_TIME;
            } else if (editing_param_index == 9) {
                // N1-ResetWHours - переключаем enum
                editing_reset1_value = (editing_reset1_value == PUMP_RESET_OFF) ? 
                                      PUMP_RESET_ON : PUMP_RESET_OFF;
            } else if (editing_param_index == 12) {
                // N2-ResetWHours - переключаем enum
                editing_reset2_value = (editing_reset2_value == PUMP_RESET_OFF) ? 
                                      PUMP_RESET_ON : PUMP_RESET_OFF;
            } else if (editing_param_index == 15) {
                // N-Training-En - переключаем enum
                editing_training_value = (editing_training_value == PUMP_TRAINING_OFF) ? 
                                      PUMP_TRAINING_ON : PUMP_TRAINING_OFF;
            } else {
                // Int параметры - уменьшаем на step
                int step = co_pumps_param_limits_int[editing_param_index].step;
                editing_int_value -= step;
                if (editing_int_value < co_pumps_param_limits_int[editing_param_index].min) {
                    editing_int_value = co_pumps_param_limits_int[editing_param_index].min;
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_RIGHT) {
            // Увеличиваем значение
            if (editing_param_index == 0) {
                // N-Number - переключаем циклически: НЕТ -> 1 -> 2 -> НЕТ
                if (editing_int_value == 2) {
                    editing_int_value = 0;  // 2 -> НЕТ
                } else {
                    editing_int_value++;    // НЕТ (0) -> 1, 1 -> 2
                }
            } else if (editing_param_index == 4) {
                // N-ChangeMode - переключаем enum
                editing_change_mode_value = (editing_change_mode_value == PUMP_CHANGE_MODE_TIME) ? 
                                           PUMP_CHANGE_MODE_WORK : PUMP_CHANGE_MODE_TIME;
            } else if (editing_param_index == 9) {
                // N1-ResetWHours - переключаем enum
                editing_reset1_value = (editing_reset1_value == PUMP_RESET_OFF) ? 
                                      PUMP_RESET_ON : PUMP_RESET_OFF;
            } else if (editing_param_index == 12) {
                // N2-ResetWHours - переключаем enum
                editing_reset2_value = (editing_reset2_value == PUMP_RESET_OFF) ? 
                                      PUMP_RESET_ON : PUMP_RESET_OFF;
            } else if (editing_param_index == 15) {
                // N-Training-En - переключаем enum
                editing_training_value = (editing_training_value == PUMP_TRAINING_OFF) ? 
                                      PUMP_TRAINING_ON : PUMP_TRAINING_OFF;
            } else {
                // Int параметры - увеличиваем на step
                int step = co_pumps_param_limits_int[editing_param_index].step;
                editing_int_value += step;
                if (editing_int_value > co_pumps_param_limits_int[editing_param_index].max) {
                    editing_int_value = co_pumps_param_limits_int[editing_param_index].max;
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_CLICK) {
            // Выходим из режима редактирования
            exit_edit_mode_with_confirmation();
        }
        return;
    }
    
    // Обычный режим навигации
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_PUMPS);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, co_pumps_cont, menu_state, MENU_TYPE_CO_PUMPS);
    
    if (prev_cursor != menu_state->cursor_index) {
        co_pumps_highlight_box(co_pumps_cont, menu_state->cursor_index);
    }
    
    // Обработка нажатия кнопки
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в меню отопления
            ESP_LOGI(TAG, "Returning to CO menu from pumps menu");
            co_pumps_menu_hide();
            co_menu_show();
            // Переключаем обработчик энкодера
            extern void co_menu_encoder_event_cb(uint8_t e);
            encoder_manager_register_callback(co_menu_encoder_event_cb);
        } else {
            // Входим в режим редактирования
            int param_index = co_pumps_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_edit_mode(param_index);
            }
        }
    }
}

/**
 * @brief Очистка меню насосов
 */
void co_pumps_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up CO pumps menu");
    
    co_pumps_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    // Очищаем массив указателей на labels
    for (int i = 0; i < 17; i++) {
        value_labels[i] = NULL;
    }
    
    if (is_obj_valid(co_pumps_mask)) {
        lv_obj_del(co_pumps_mask);
        co_pumps_mask = NULL;
    }
    
    if (is_obj_valid(co_pumps_cont)) {
        lv_obj_del(co_pumps_cont);
        co_pumps_cont = NULL;
    }
    
    co_pumps_menu_initialized = false;
}

/**
 * @brief Инициализация меню насосов
 */
void CO_Pumps_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню насосов");
    
    if (co_pumps_menu_creation_in_progress) {
        ESP_LOGW(TAG, "CO pumps menu creation already in progress, skipping");
        return;
    }
    
    co_pumps_menu_creation_in_progress = true;
    
    if (co_pumps_menu_initialized && is_obj_valid(co_pumps_cont)) {
        ESP_LOGI(TAG, "CO pumps menu already initialized, showing it");
        co_pumps_menu_show();
        co_pumps_menu_creation_in_progress = false;
        return;
    }
    
    co_pumps_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    co_pumps_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(co_pumps_cont)) {
        ESP_LOGE(TAG, "Failed to create CO pumps menu container");
        co_pumps_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(co_pumps_cont, 1200, 1200);
    lv_obj_center(co_pumps_cont);
    lv_obj_add_event_cb(co_pumps_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(co_pumps_cont, &style, 0);
    lv_obj_set_style_radius(co_pumps_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(co_pumps_cont, true, 0);
    lv_obj_set_scroll_dir(co_pumps_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(co_pumps_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(co_pumps_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(co_pumps_cont, 633, 0);
    lv_obj_set_style_bg_color(co_pumps_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(co_pumps_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(co_pumps_cont, 0, 0);
    lv_obj_set_style_pad_row(co_pumps_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(co_pumps_menu_items) / sizeof(CoPumpsMenuItem); i++) {
        create_co_pumps_menu_item(co_pumps_cont, &co_pumps_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    co_pumps_mask = radial();
    if (is_obj_valid(co_pumps_mask)) {
        lv_obj_set_pos(co_pumps_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_CO_PUMPS);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_PUMPS);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(co_pumps_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    co_pumps_highlight_box(co_pumps_cont, menu_state->cursor_index);
    arc_menu_update_slide(co_pumps_cont);
    
    co_pumps_menu_initialized = true;
    co_pumps_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню насосов успешно инициализировано");
}


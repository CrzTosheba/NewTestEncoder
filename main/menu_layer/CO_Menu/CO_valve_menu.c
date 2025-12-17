#include "CO_valve_menu.h"
#include "CO_valve_params.h"
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

static const char *TAG = "CO_VALVE_MENU";

// Forward declarations
static void update_param_display(int param_index);

// Структура элемента меню клапан
typedef struct {
    const char *label_text;     // Текст элемента слева
    const void *img_src;        // Иконка (только для "Назад")
    int param_index;            // Индекс параметра (-1 для "Назад")
} CoValveMenuItem;

// Элементы меню клапан
static const CoValveMenuItem co_valve_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Тип регулятора", NULL, 0},                  // M-RegType (int)
    {"Длина штока, мм", NULL, 1},                 // M-Length (int)
    {"Скорость, с/мм", NULL, 2},                   // M-Speed (float)
    {"П-коэффициент", NULL, 3},                    // M-PCoef (float)
    {"И-коэффициент", NULL, 4},                    // M-ICoef (float)
    {"Нейтральная зона, °C", NULL, 5},            // M-Deadband (float)
    {"Мин. ширина ИМПС, мс", NULL, 6},            // M-IControl-Min (int)
};

// Локальные переменные для меню клапан
lv_obj_t *co_valve_cont = NULL;
static bool co_valve_menu_initialized = false;
static bool co_valve_menu_creation_in_progress = false;
static lv_obj_t *co_valve_mask = NULL;

// Массив указателей на label для значений параметров (7 параметров)
static lv_obj_t *value_labels[7] = {NULL};

// Состояние редактирования
static bool edit_mode = false;
static int editing_param_index = -1;
static int editing_int_value = 0;
static float editing_float_value = 0.0f;
static co_reg_type_t editing_reg_type_value = CO_REG_TYPE_PI;

// Временные значения для отмены изменений
static co_reg_type_t temp_M_RegType = CO_REG_TYPE_PI;
static int temp_M_Length = 0;
static float temp_M_Speed = 0.0f;
static float temp_M_PCoef = 0.0f;
static float temp_M_ICoef = 0.0f;
static float temp_M_Deadband = 0.0f;
static int temp_M_IControl_Min = 0;

/**
 * @brief Получает строковое представление типа регулятора
 */
static const char* get_reg_type_string(co_reg_type_t reg_type) {
    switch(reg_type) {
        case CO_REG_TYPE_P: return "П";
        case CO_REG_TYPE_PI: return "ПИ";
        case CO_REG_TYPE_PID: return "ПИД";
        default: return "???";
    }
}

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
 * @brief Подсветка выбранного элемента меню клапан
 */
static void co_valve_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
    if (!is_obj_valid(cont)) return;
    
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!is_obj_valid(child)) continue;
        
        bool is_selected = (i == cursor_index);
        bool is_editing_this = (edit_mode && editing_param_index == co_valve_menu_items[i].param_index);
        
        uint32_t grand_child_cnt = lv_obj_get_child_cnt(child);
        
        for (uint32_t j = 0; j < grand_child_cnt; j++) {
            lv_obj_t *grand_child = lv_obj_get_child(child, j);
            if (!is_obj_valid(grand_child)) continue;
            
            // Проверяем, является ли это контейнером значения параметра
            bool is_value_container = false;
            for (int k = 0; k < 7; k++) {
                if (value_labels[k] != NULL && lv_obj_get_parent(value_labels[k]) == grand_child) {
                    is_value_container = true;
                    break;
                }
            }
            
            if (lv_obj_check_type(grand_child, &lv_label_class)) {
                // Проверяем, является ли это label значения параметра
                bool is_value_label = false;
                for (int k = 0; k < 7; k++) {
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
    if (param_index < 0 || param_index >= 7) return;
    if (!is_obj_valid(value_labels[param_index])) return;
    
    char value_str[32];
    
    // Float параметры: индексы 2, 3, 4, 5
    bool is_float = (param_index >= 2 && param_index <= 5);
    
    if (edit_mode && editing_param_index == param_index) {
        // В режиме редактирования показываем временное значение
        if (param_index == 0) {
            // Тип регулятора (enum)
            snprintf(value_str, sizeof(value_str), "%s", get_reg_type_string(editing_reg_type_value));
        } else if (is_float) {
            format_float_value(value_str, sizeof(value_str), editing_float_value);
        } else {
            snprintf(value_str, sizeof(value_str), "%d", editing_int_value);
        }
    } else {
        // Обычный режим - показываем текущее значение
        switch(param_index) {
            case 0: // M-RegType
                snprintf(value_str, sizeof(value_str), "%s", get_reg_type_string(M_RegType));
                break;
            case 1: // M-Length
                snprintf(value_str, sizeof(value_str), "%d", M_Length);
                break;
            case 2: // M-Speed
                format_float_value(value_str, sizeof(value_str), M_Speed);
                break;
            case 3: // M-PCoef
                format_float_value(value_str, sizeof(value_str), M_PCoef);
                break;
            case 4: // M-ICoef
                format_float_value(value_str, sizeof(value_str), M_ICoef);
                break;
            case 5: // M-Deadband
                format_float_value(value_str, sizeof(value_str), M_Deadband);
                break;
            case 6: // M-IControl-Min
                snprintf(value_str, sizeof(value_str), "%d", M_IControl_Min);
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
        case 0: M_RegType = editing_reg_type_value; break;
        case 1: M_Length = editing_int_value; break;
        case 2: M_Speed = editing_float_value; break;
        case 3: M_PCoef = editing_float_value; break;
        case 4: M_ICoef = editing_float_value; break;
        case 5: M_Deadband = editing_float_value; break;
        case 6: M_IControl_Min = editing_int_value; break;
    }
    
    // НЕ сохраняем параметры в NVS (как в ГВС)
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_VALVE);
    if (menu_state) {
        co_valve_highlight_box(co_valve_cont, menu_state->cursor_index);
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
        case 0: editing_reg_type_value = temp_M_RegType; break;
        case 1: editing_int_value = temp_M_Length; break;
        case 2: editing_float_value = temp_M_Speed; break;
        case 3: editing_float_value = temp_M_PCoef; break;
        case 4: editing_float_value = temp_M_ICoef; break;
        case 5: editing_float_value = temp_M_Deadband; break;
        case 6: editing_int_value = temp_M_IControl_Min; break;
    }
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_VALVE);
    if (menu_state) {
        co_valve_highlight_box(co_valve_cont, menu_state->cursor_index);
    }
}

/**
 * @brief Входит в режим редактирования параметра
 */
static void enter_edit_mode(int param_index) {
    if (param_index < 0 || param_index >= 7) return;
    
    // Проверяем доступ перед редактированием
    if (!access_control_is_unlocked()) {
        ESP_LOGW(TAG, "Access denied: cannot edit parameters when access is locked");
        return;
    }
    
    ESP_LOGI(TAG, "Entering edit mode for parameter %d", param_index);
    
    edit_mode = true;
    editing_param_index = param_index;
    
    // Float параметры: индексы 2, 3, 4, 5
    bool is_float = (param_index >= 2 && param_index <= 5);
    
    // Сохраняем текущие значения как временные
    if (param_index == 0) {
        // Тип регулятора (enum)
        temp_M_RegType = M_RegType;
        editing_reg_type_value = M_RegType;
    } else if (is_float) {
        switch(param_index) {
            case 2:
                temp_M_Speed = M_Speed;
                editing_float_value = M_Speed;
                break;
            case 3:
                temp_M_PCoef = M_PCoef;
                editing_float_value = M_PCoef;
                break;
            case 4:
                temp_M_ICoef = M_ICoef;
                editing_float_value = M_ICoef;
                break;
            case 5:
                temp_M_Deadband = M_Deadband;
                editing_float_value = M_Deadband;
                break;
        }
    } else {
        switch(param_index) {
            case 1:
                temp_M_Length = M_Length;
                editing_int_value = M_Length;
                break;
            case 6:
                temp_M_IControl_Min = M_IControl_Min;
                editing_int_value = M_IControl_Min;
                break;
        }
    }
    
    update_param_display(param_index);
}

/**
 * @brief Выходит из режима редактирования и проверяет изменения
 */
static void exit_edit_mode_with_confirmation(void) {
    if (!edit_mode || editing_param_index < 0) return;
    
    bool value_changed = false;
    
    // Float параметры: индексы 2, 3, 4, 5
    bool is_float = (editing_param_index >= 2 && editing_param_index <= 5);
    
    // Проверяем, изменилось ли значение
    if (editing_param_index == 0) {
        // Тип регулятора (enum)
        value_changed = (editing_reg_type_value != temp_M_RegType);
    } else if (is_float) {
        switch(editing_param_index) {
            case 2: value_changed = (fabs(editing_float_value - temp_M_Speed) > 0.01f); break;
            case 3: value_changed = (fabs(editing_float_value - temp_M_PCoef) > 0.01f); break;
            case 4: value_changed = (fabs(editing_float_value - temp_M_ICoef) > 0.01f); break;
            case 5: value_changed = (fabs(editing_float_value - temp_M_Deadband) > 0.01f); break;
        }
    } else {
        switch(editing_param_index) {
            case 1: value_changed = (editing_int_value != temp_M_Length); break;
            case 6: value_changed = (editing_int_value != temp_M_IControl_Min); break;
        }
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
 * @brief Создание элемента меню клапан
 */
static void create_co_valve_menu_item(lv_obj_t *cont, const CoValveMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_co_valve_menu_item");
        return;
    }
    
    // Создаем контейнер для элемента (высота 40px)
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_co_valve_menu_item");
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
 * @brief Показывает меню клапан
 */
void co_valve_menu_show(void) {
    ESP_LOGI(TAG, "Showing CO valve menu");
    if (is_obj_valid(co_valve_cont)) {
        lv_obj_clear_flag(co_valve_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(co_valve_mask)) {
        co_valve_mask = radial();
        if (is_obj_valid(co_valve_mask)) {
            lv_obj_set_pos(co_valve_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(co_valve_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает меню клапан
 */
void co_valve_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding CO valve menu");
    if (is_obj_valid(co_valve_cont)) {
        lv_obj_add_flag(co_valve_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(co_valve_mask)) {
        lv_obj_del(co_valve_mask);
        co_valve_mask = NULL;
    }
}

/**
 * @brief Обработчик событий энкодера для меню клапан
 */
void co_valve_menu_encoder_event_cb(uint8_t e) {
    // Обновляем таймер активности при любом действии пользователя
    access_control_update_activity_timer();
    
    // Если активно окно подтверждения, передаем события ему
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(co_valve_cont)) {
        ESP_LOGE(TAG, "Контейнер меню клапан не инициализирован");
        return;
    }
    
    // Если в режиме редактирования
    if (edit_mode && editing_param_index >= 0) {
        // Float параметры: индексы 2, 3, 4, 5
        bool is_float = (editing_param_index >= 2 && editing_param_index <= 5);
        
        if (e & ENC_LEFT) {
            if (editing_param_index == 0) {
                // Тип регулятора (enum) - циклическое переключение П -> ПИ -> ПИД -> П
                if (editing_reg_type_value == CO_REG_TYPE_P) {
                    editing_reg_type_value = CO_REG_TYPE_PID;
                } else {
                    editing_reg_type_value = (co_reg_type_t)((int)editing_reg_type_value - 1);
                }
            } else if (is_float) {
                // Float параметры - уменьшаем на step
                int float_index = editing_param_index - 2; // Маппинг: 2->0, 3->1, 4->2, 5->3
                float step = co_valve_param_limits_float[float_index].step;
                editing_float_value -= step;
                if (editing_float_value < co_valve_param_limits_float[float_index].min) {
                    editing_float_value = co_valve_param_limits_float[float_index].min;
                }
            } else {
                // Int параметры - уменьшаем на step
                // Маппинг: 1->1, 6->2
                int int_index;
                if (editing_param_index == 1) {
                    int_index = 1;  // M_Length
                } else { // editing_param_index == 6
                    int_index = 2;  // M_IControl_Min
                }
                if (int_index >= 0 && int_index < PARAM_LIMITS_VALVE_INT_COUNT) {
                    int step = co_valve_param_limits_int[int_index].step;
                    editing_int_value -= step;
                    if (editing_int_value < co_valve_param_limits_int[int_index].min) {
                        editing_int_value = co_valve_param_limits_int[int_index].min;
                    }
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_RIGHT) {
            if (editing_param_index == 0) {
                // Тип регулятора (enum) - циклическое переключение П -> ПИ -> ПИД -> П
                if (editing_reg_type_value == CO_REG_TYPE_PID) {
                    editing_reg_type_value = CO_REG_TYPE_P;
                } else {
                    editing_reg_type_value = (co_reg_type_t)((int)editing_reg_type_value + 1);
                }
            } else if (is_float) {
                // Float параметры - увеличиваем на step
                int float_index = editing_param_index - 2; // Маппинг: 2->0, 3->1, 4->2, 5->3
                float step = co_valve_param_limits_float[float_index].step;
                editing_float_value += step;
                if (editing_float_value > co_valve_param_limits_float[float_index].max) {
                    editing_float_value = co_valve_param_limits_float[float_index].max;
                }
            } else {
                // Int параметры - увеличиваем на step
                // Маппинг: 1->1, 6->2
                int int_index;
                if (editing_param_index == 1) {
                    int_index = 1;  // M_Length
                } else { // editing_param_index == 6
                    int_index = 2;  // M_IControl_Min
                }
                if (int_index >= 0 && int_index < PARAM_LIMITS_VALVE_INT_COUNT) {
                    int step = co_valve_param_limits_int[int_index].step;
                    editing_int_value += step;
                    if (editing_int_value > co_valve_param_limits_int[int_index].max) {
                        editing_int_value = co_valve_param_limits_int[int_index].max;
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
    
    // Обычный режим навигации
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_VALVE);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, co_valve_cont, menu_state, MENU_TYPE_CO_VALVE);
    
    if (prev_cursor != menu_state->cursor_index) {
        co_valve_highlight_box(co_valve_cont, menu_state->cursor_index);
    }
    
    // Обработка нажатия кнопки
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в меню отопления
            ESP_LOGI(TAG, "Returning to CO menu from valve menu");
            co_valve_menu_hide();
            co_menu_show();
            // Переключаем обработчик энкодера
            extern void co_menu_encoder_event_cb(uint8_t e);
            encoder_manager_register_callback(co_menu_encoder_event_cb);
        } else {
            // Входим в режим редактирования
            int param_index = co_valve_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_edit_mode(param_index);
            }
        }
    }
}

/**
 * @brief Очистка меню клапан
 */
void co_valve_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up CO valve menu");
    
    co_valve_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    // Очищаем массив указателей на labels
    for (int i = 0; i < 7; i++) {
        value_labels[i] = NULL;
    }
    
    if (is_obj_valid(co_valve_mask)) {
        lv_obj_del(co_valve_mask);
        co_valve_mask = NULL;
    }
    
    if (is_obj_valid(co_valve_cont)) {
        lv_obj_del(co_valve_cont);
        co_valve_cont = NULL;
    }
    
    co_valve_menu_initialized = false;
}

/**
 * @brief Инициализация меню клапан
 */
void CO_Valve_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню клапан");
    
    if (co_valve_menu_creation_in_progress) {
        ESP_LOGW(TAG, "CO valve menu creation already in progress, skipping");
        return;
    }
    
    co_valve_menu_creation_in_progress = true;
    
    if (co_valve_menu_initialized && is_obj_valid(co_valve_cont)) {
        ESP_LOGI(TAG, "CO valve menu already initialized, showing it");
        co_valve_menu_show();
        co_valve_menu_creation_in_progress = false;
        return;
    }
    
    co_valve_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    co_valve_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(co_valve_cont)) {
        ESP_LOGE(TAG, "Failed to create CO valve menu container");
        co_valve_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(co_valve_cont, 1200, 1200);
    lv_obj_center(co_valve_cont);
    lv_obj_add_event_cb(co_valve_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(co_valve_cont, &style, 0);
    lv_obj_set_style_radius(co_valve_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(co_valve_cont, true, 0);
    lv_obj_set_scroll_dir(co_valve_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(co_valve_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(co_valve_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(co_valve_cont, 633, 0);
    lv_obj_set_style_bg_color(co_valve_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(co_valve_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(co_valve_cont, 0, 0);
    lv_obj_set_style_pad_row(co_valve_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(co_valve_menu_items) / sizeof(CoValveMenuItem); i++) {
        create_co_valve_menu_item(co_valve_cont, &co_valve_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    co_valve_mask = radial();
    if (is_obj_valid(co_valve_mask)) {
        lv_obj_set_pos(co_valve_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_CO_VALVE);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_VALVE);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(co_valve_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    co_valve_highlight_box(co_valve_cont, menu_state->cursor_index);
    arc_menu_update_slide(co_valve_cont);
    
    co_valve_menu_initialized = true;
    co_valve_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню клапан успешно инициализировано");
}


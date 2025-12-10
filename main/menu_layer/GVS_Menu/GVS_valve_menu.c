#include "GVS_valve_menu.h"
#include "GVS_valve_params.h"
#include "gvs_valve_params_limits.h"
#include "GVS_main_menu.h"
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

static const char *TAG = "GVS_VALVE_MENU";

// Forward declarations
static void update_param_display(int param_index);

// Структура элемента меню клапан ГВС
typedef struct {
    const char *label_text;     // Текст элемента слева
    const void *img_src;        // Иконка (только для "Назад")
    int param_index;            // Индекс параметра (-1 для "Назад")
} GvsValveMenuItem;

// Элементы меню клапан ГВС
static const GvsValveMenuItem gvs_valve_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Тип регулятора", NULL, 0},                  // GVS_M_RegType (enum)
    {"Длина штока, мм", NULL, 1},                 // GVS_M_Length (M-Length: int)
    {"Скорость, с/мм", NULL, 2},                   // GVS_M_Speed (M-Speed: float)
    {"П-коэффициент", NULL, 3},                    // GVS_M_PCoef (float)
    {"И-коэффициент", NULL, 4},                    // GVS_M_ICoef (float)
    {"Нейтральная зона, °C", NULL, 5},            // GVS_M_Deadband (float)
    {"Мин. ширина ИМПС, мс", NULL, 6},            // GVS_M_IControl_Min (int)
};

// Локальные переменные для меню клапан ГВС
lv_obj_t *gvs_valve_cont = NULL;
static bool gvs_valve_menu_initialized = false;
static bool gvs_valve_menu_creation_in_progress = false;
static lv_obj_t *gvs_valve_mask = NULL;

// Массив указателей на label для значений параметров (7 параметров)
static lv_obj_t *value_labels[7] = {NULL};

// Состояние редактирования
static bool edit_mode = false;
static int editing_param_index = -1;
static int editing_int_value = 0;
static float editing_float_value = 0.0f;
static gvs_reg_type_t editing_reg_type_value = GVS_REG_TYPE_PI;

// Временные значения для отмены изменений
static gvs_reg_type_t temp_GVS_M_RegType = GVS_REG_TYPE_PI;
static int temp_GVS_M_Length = 0;
static float temp_GVS_M_Speed = 0.0f;
static float temp_GVS_M_PCoef = 0.0f;
static float temp_GVS_M_ICoef = 0.0f;
static float temp_GVS_M_Deadband = 0.0f;
static int temp_GVS_M_IControl_Min = 0;

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
 * @brief Получает строковое представление типа регулятора
 */
static const char* get_reg_type_string(gvs_reg_type_t reg_type) {
    switch(reg_type) {
        case GVS_REG_TYPE_P: return "П";
        case GVS_REG_TYPE_PI: return "ПИ";
        case GVS_REG_TYPE_PID: return "ПИД";
        default: return "???";
    }
}

/**
 * @brief Подсветка выбранного элемента меню клапан ГВС
 */
static void gvs_valve_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
    if (!is_obj_valid(cont)) return;
    
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!is_obj_valid(child)) continue;
        
        uint32_t grand_child_cnt = lv_obj_get_child_cnt(child);
        
        for (uint32_t j = 0; j < grand_child_cnt; j++) {
            lv_obj_t *grand_child = lv_obj_get_child(child, j);
            if (!is_obj_valid(grand_child)) continue;
            
            if (lv_obj_check_type(grand_child, &lv_label_class)) {
                if (i == cursor_index) {
                    // В режиме редактирования не меняем цвет редактируемого параметра
                    if (!(edit_mode && editing_param_index == gvs_valve_menu_items[i].param_index)) {
                        lv_obj_set_style_text_color(grand_child, lv_color_hex(0x000000), LV_PART_MAIN);
                    }
                } else {
                    lv_obj_set_style_text_color(grand_child, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                }
            } else if (lv_obj_check_type(grand_child, &lv_image_class)) {
                if (i == cursor_index) {
                    lv_obj_set_style_img_recolor(grand_child, lv_color_hex(0x000000), 0);
                } else {
                    lv_obj_set_style_img_recolor(grand_child, lv_color_hex(0xFFFFFF), 0);
                }
            }
        }
        
        // Меняем фон контейнера
        if (i == cursor_index) {
            // В режиме редактирования фон уже установлен в update_param_display
            if (!(edit_mode && editing_param_index == gvs_valve_menu_items[i].param_index)) {
                lv_obj_set_style_bg_color(child, lv_color_hex(0xFFCC00), LV_PART_MAIN);
            }
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
            case 0: // GVS_M_RegType
                snprintf(value_str, sizeof(value_str), "%s", get_reg_type_string(GVS_M_RegType));
                break;
            case 1: // GVS_M_Length (M-Length: Длина штока, мм)
                snprintf(value_str, sizeof(value_str), "%d", GVS_M_Length);
                break;
            case 2: // GVS_M_Speed (M-Speed: Скорость, с/мм)
                format_float_value(value_str, sizeof(value_str), GVS_M_Speed);
                break;
            case 3: // GVS_M_PCoef (П-коэффициент)
                format_float_value(value_str, sizeof(value_str), GVS_M_PCoef);
                break;
            case 4: // GVS_M_ICoef (И-коэффициент)
                format_float_value(value_str, sizeof(value_str), GVS_M_ICoef);
                break;
            case 5: // GVS_M_Deadband
                format_float_value(value_str, sizeof(value_str), GVS_M_Deadband);
                break;
            case 6: // GVS_M_IControl_Min
                snprintf(value_str, sizeof(value_str), "%d", GVS_M_IControl_Min);
                break;
        }
    }
    
    // Добавляем единицы измерения
    char full_str[40];
    if (param_index == 1) {
        // Длина штока - миллиметры
        snprintf(full_str, sizeof(full_str), "%s мм", value_str);
    } else if (param_index == 2) {
        // Скорость - с/мм
        snprintf(full_str, sizeof(full_str), "%s с/мм", value_str);
    } else if (param_index == 5) {
        // Нейтральная зона - градусы Цельсия
        snprintf(full_str, sizeof(full_str), "%s °C", value_str);
    } else if (param_index == 6) {
        // Мин. ширина ИМПС - миллисекунды
        snprintf(full_str, sizeof(full_str), "%s мс", value_str);
    } else {
        // Для остальных параметров без единиц
        snprintf(full_str, sizeof(full_str), "%s", value_str);
    }
    
    lv_label_set_text(value_labels[param_index], full_str);
    
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
        case 0: GVS_M_RegType = editing_reg_type_value; break;
        case 1: GVS_M_Length = editing_int_value; break;
        case 2: GVS_M_Speed = editing_float_value; break;
        case 3: GVS_M_PCoef = editing_float_value; break;
        case 4: GVS_M_ICoef = editing_float_value; break;
        case 5: GVS_M_Deadband = editing_float_value; break;
        case 6: GVS_M_IControl_Min = editing_int_value; break;
    }
    
    // НЕ сохраняем параметры в NVS (как в ГВС)
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_VALVE);
    if (menu_state) {
        gvs_valve_highlight_box(gvs_valve_cont, menu_state->cursor_index);
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
        case 0: editing_reg_type_value = temp_GVS_M_RegType; break;
        case 1: editing_int_value = temp_GVS_M_Length; break;
        case 2: editing_float_value = temp_GVS_M_Speed; break;
        case 3: editing_float_value = temp_GVS_M_PCoef; break;
        case 4: editing_float_value = temp_GVS_M_ICoef; break;
        case 5: editing_float_value = temp_GVS_M_Deadband; break;
        case 6: editing_int_value = temp_GVS_M_IControl_Min; break;
    }
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_VALVE);
    if (menu_state) {
        gvs_valve_highlight_box(gvs_valve_cont, menu_state->cursor_index);
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
        temp_GVS_M_RegType = GVS_M_RegType;
        editing_reg_type_value = GVS_M_RegType;
    } else if (is_float) {
        switch(param_index) {
            case 2:
                temp_GVS_M_Speed = GVS_M_Speed;
                editing_float_value = GVS_M_Speed;
                break;
            case 3:
                temp_GVS_M_PCoef = GVS_M_PCoef;
                editing_float_value = GVS_M_PCoef;
                break;
            case 4:
                temp_GVS_M_ICoef = GVS_M_ICoef;
                editing_float_value = GVS_M_ICoef;
                break;
            case 5:
                temp_GVS_M_Deadband = GVS_M_Deadband;
                editing_float_value = GVS_M_Deadband;
                break;
        }
    } else {
        switch(param_index) {
            case 1:
                temp_GVS_M_Length = GVS_M_Length;
                editing_int_value = GVS_M_Length;
                break;
            case 6:
                temp_GVS_M_IControl_Min = GVS_M_IControl_Min;
                editing_int_value = GVS_M_IControl_Min;
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
        value_changed = (editing_reg_type_value != temp_GVS_M_RegType);
    } else if (is_float) {
        switch(editing_param_index) {
            case 2: value_changed = (fabs(editing_float_value - temp_GVS_M_Speed) > 0.01f); break;
            case 3: value_changed = (fabs(editing_float_value - temp_GVS_M_PCoef) > 0.01f); break;
            case 4: value_changed = (fabs(editing_float_value - temp_GVS_M_ICoef) > 0.01f); break;
            case 5: value_changed = (fabs(editing_float_value - temp_GVS_M_Deadband) > 0.01f); break;
        }
    } else {
        switch(editing_param_index) {
            case 1: value_changed = (editing_int_value != temp_GVS_M_Length); break;
            case 6: value_changed = (editing_int_value != temp_GVS_M_IControl_Min); break;
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
 * @brief Создание элемента меню клапан ГВС
 */
static void create_gvs_valve_menu_item(lv_obj_t *cont, const GvsValveMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_gvs_valve_menu_item");
        return;
    }
    
    // Создаем контейнер для элемента (высота 40px)
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_gvs_valve_menu_item");
        return;
    }
    
    lv_obj_set_size(box, 462, 40);
    lv_obj_set_style_border_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_radius(box, 0, 0);
    
    // Основная надпись слева
    lv_obj_t *label = lv_label_create(box);
    if (is_obj_valid(label)) {
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
        lv_label_set_text(label, item->label_text);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);
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
            lv_obj_set_size(value_container, 150, 40);
            lv_obj_set_style_bg_color(value_container, lv_color_hex(0x2B3639), LV_PART_MAIN);
            lv_obj_set_style_border_color(value_container, lv_color_hex(0x2B3639), LV_PART_MAIN);
            lv_obj_set_style_radius(value_container, 0, 0);
            lv_obj_set_style_pad_all(value_container, 0, 0);
            // Выравниваем контейнер значения относительно названия параметра (смещение 200px от левого края)
            lv_obj_set_pos(value_container, 200, -23);
            
            // Помечаем контейнер значения параметра для компенсации движения по дуге
            set_as_param_value(value_container);
            
            lv_obj_t *value_label = lv_label_create(value_container);
            if (is_obj_valid(value_label)) {
                value_labels[item->param_index] = value_label;
                lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_font(value_label, &Roboto_bold_24, 0);
                lv_obj_align(value_label, LV_ALIGN_CENTER, 0, 0);
                update_param_display(item->param_index);
            }
        }
    }
    
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);
    vTaskDelay(pdMS_TO_TICKS(1));
}

/**
 * @brief Показывает меню клапан ГВС
 */
void gvs_valve_menu_show(void) {
    ESP_LOGI(TAG, "Showing GVS valve menu");
    if (is_obj_valid(gvs_valve_cont)) {
        lv_obj_clear_flag(gvs_valve_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(gvs_valve_mask)) {
        gvs_valve_mask = radial();
        if (is_obj_valid(gvs_valve_mask)) {
            lv_obj_set_pos(gvs_valve_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(gvs_valve_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает меню клапан ГВС
 */
void gvs_valve_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding GVS valve menu");
    if (is_obj_valid(gvs_valve_cont)) {
        lv_obj_add_flag(gvs_valve_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(gvs_valve_mask)) {
        lv_obj_del(gvs_valve_mask);
        gvs_valve_mask = NULL;
    }
}

/**
 * @brief Обработчик событий энкодера для меню клапан ГВС
 */
void gvs_valve_menu_encoder_event_cb(uint8_t e) {
    // Если активно окно подтверждения, передаем события ему
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(gvs_valve_cont)) {
        ESP_LOGE(TAG, "Контейнер меню клапан ГВС не инициализирован");
        return;
    }
    
    // Если в режиме редактирования
    if (edit_mode && editing_param_index >= 0) {
        // Float параметры: индексы 2, 3, 4, 5
        bool is_float = (editing_param_index >= 2 && editing_param_index <= 5);
        
        if (e & ENC_LEFT) {
            if (editing_param_index == 0) {
                // Тип регулятора (enum) - циклическое переключение П -> ПИ -> ПИД -> П
                if (editing_reg_type_value == GVS_REG_TYPE_P) {
                    editing_reg_type_value = GVS_REG_TYPE_PID;
                } else {
                    editing_reg_type_value = (gvs_reg_type_t)((int)editing_reg_type_value - 1);
                }
            } else if (is_float) {
                // Float параметры - уменьшаем на step
                int float_index = editing_param_index - 2; // Маппинг: 2->0, 3->1, 4->2, 5->3
                float step = gvs_valve_param_limits_float[float_index].step;
                editing_float_value -= step;
                if (editing_float_value < gvs_valve_param_limits_float[float_index].min) {
                    editing_float_value = gvs_valve_param_limits_float[float_index].min;
                }
            } else {
                // Int параметры - уменьшаем на step
                // Маппинг: 1->0, 6->1 (param_index 0 - это enum, обрабатывается отдельно)
                // В массиве limits: [0]=RegType(enum), [1]=Length, [2]=IControl_Min
                int int_index;
                if (editing_param_index == 1) {
                    int_index = 1;  // GVS_M_Length
                } else { // editing_param_index == 6
                    int_index = 2;  // GVS_M_IControl_Min
                }
                if (int_index >= 0 && int_index < PARAM_LIMITS_GVS_VALVE_INT_COUNT) {
                    int step = gvs_valve_param_limits_int[int_index].step;
                    editing_int_value -= step;
                    if (editing_int_value < gvs_valve_param_limits_int[int_index].min) {
                        editing_int_value = gvs_valve_param_limits_int[int_index].min;
                    }
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_RIGHT) {
            if (editing_param_index == 0) {
                // Тип регулятора (enum) - циклическое переключение П -> ПИ -> ПИД -> П
                if (editing_reg_type_value == GVS_REG_TYPE_PID) {
                    editing_reg_type_value = GVS_REG_TYPE_P;
                } else {
                    editing_reg_type_value = (gvs_reg_type_t)((int)editing_reg_type_value + 1);
                }
            } else if (is_float) {
                // Float параметры - увеличиваем на step
                int float_index = editing_param_index - 2; // Маппинг: 2->0, 3->1, 4->2, 5->3
                float step = gvs_valve_param_limits_float[float_index].step;
                editing_float_value += step;
                if (editing_float_value > gvs_valve_param_limits_float[float_index].max) {
                    editing_float_value = gvs_valve_param_limits_float[float_index].max;
                }
            } else {
                // Int параметры - увеличиваем на step
                // Маппинг: 1->0, 6->1 (param_index 0 - это enum, обрабатывается отдельно)
                // В массиве limits: [0]=RegType(enum), [1]=Length, [2]=IControl_Min
                int int_index;
                if (editing_param_index == 1) {
                    int_index = 1;  // GVS_M_Length
                } else { // editing_param_index == 6
                    int_index = 2;  // GVS_M_IControl_Min
                }
                if (int_index >= 0 && int_index < PARAM_LIMITS_GVS_VALVE_INT_COUNT) {
                    int step = gvs_valve_param_limits_int[int_index].step;
                    editing_int_value += step;
                    if (editing_int_value > gvs_valve_param_limits_int[int_index].max) {
                        editing_int_value = gvs_valve_param_limits_int[int_index].max;
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
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_VALVE);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, gvs_valve_cont, menu_state, MENU_TYPE_GVS_VALVE);
    
    if (prev_cursor != menu_state->cursor_index) {
        gvs_valve_highlight_box(gvs_valve_cont, menu_state->cursor_index);
    }
    
    // Обработка нажатия кнопки
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в меню ГВС
            ESP_LOGI(TAG, "Returning to GVS menu from valve menu");
            gvs_valve_menu_hide();
            gvs_menu_show();
            // Переключаем обработчик энкодера
            extern void gvs_menu_encoder_event_cb(uint8_t e);
            encoder_manager_register_callback(gvs_menu_encoder_event_cb);
        } else {
            // Входим в режим редактирования
            int param_index = gvs_valve_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_edit_mode(param_index);
            }
        }
    }
}

/**
 * @brief Очистка меню клапан ГВС
 */
void gvs_valve_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up GVS valve menu");
    
    gvs_valve_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    // Очищаем массив указателей на labels
    for (int i = 0; i < 7; i++) {
        value_labels[i] = NULL;
    }
    
    if (is_obj_valid(gvs_valve_mask)) {
        lv_obj_del(gvs_valve_mask);
        gvs_valve_mask = NULL;
    }
    
    if (is_obj_valid(gvs_valve_cont)) {
        lv_obj_del(gvs_valve_cont);
        gvs_valve_cont = NULL;
    }
    
    gvs_valve_menu_initialized = false;
}

/**
 * @brief Инициализация меню клапан ГВС
 */
void GVS_Valve_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню клапан ГВС");
    
    if (gvs_valve_menu_creation_in_progress) {
        ESP_LOGW(TAG, "GVS valve menu creation already in progress, skipping");
        return;
    }
    
    gvs_valve_menu_creation_in_progress = true;
    
    if (gvs_valve_menu_initialized && is_obj_valid(gvs_valve_cont)) {
        ESP_LOGI(TAG, "GVS valve menu already initialized, showing it");
        gvs_valve_menu_show();
        gvs_valve_menu_creation_in_progress = false;
        return;
    }
    
    gvs_valve_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    gvs_valve_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(gvs_valve_cont)) {
        ESP_LOGE(TAG, "Failed to create GVS valve menu container");
        gvs_valve_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(gvs_valve_cont, 1200, 1200);
    lv_obj_center(gvs_valve_cont);
    lv_obj_add_event_cb(gvs_valve_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(gvs_valve_cont, &style, 0);
    lv_obj_set_style_radius(gvs_valve_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(gvs_valve_cont, true, 0);
    lv_obj_set_scroll_dir(gvs_valve_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(gvs_valve_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(gvs_valve_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(gvs_valve_cont, 633, 0);
    lv_obj_set_style_bg_color(gvs_valve_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(gvs_valve_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(gvs_valve_cont, 0, 0);
    lv_obj_set_style_pad_row(gvs_valve_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(gvs_valve_menu_items) / sizeof(GvsValveMenuItem); i++) {
        create_gvs_valve_menu_item(gvs_valve_cont, &gvs_valve_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    gvs_valve_mask = radial();
    if (is_obj_valid(gvs_valve_mask)) {
        lv_obj_set_pos(gvs_valve_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_GVS_VALVE);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_VALVE);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(gvs_valve_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    gvs_valve_highlight_box(gvs_valve_cont, menu_state->cursor_index);
    arc_menu_update_slide(gvs_valve_cont);
    
    gvs_valve_menu_initialized = true;
    gvs_valve_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню клапан ГВС успешно инициализировано");
}


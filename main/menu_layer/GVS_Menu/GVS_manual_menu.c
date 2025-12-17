#include "GVS_manual_menu.h"
#include "GVS_manual_params.h"
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

static const char *TAG = "GVS_MANUAL_MENU";

// Forward declarations
static void update_param_display(int param_index);

// Структура элемента меню ручной режим ГВС
typedef struct {
    const char *label_text;     // Текст элемента слева
    const void *img_src;        // Иконка (только для "Назад")
    int param_index;            // Индекс параметра (-1 для "Назад")
} GvsManualMenuItem;

// Элементы меню ручной режим ГВС
static const GvsManualMenuItem gvs_manual_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Насос 1", NULL, 0},                       // GVS_N1-DControl (enum)
    {"Насос 2", NULL, 1},                       // GVS_N2-DControl (enum)
    {"Клапан", NULL, 2},                         // GVS_M-IControl (enum)
};

// Локальные переменные для меню ручной режим ГВС
lv_obj_t *gvs_manual_cont = NULL;
static bool gvs_manual_menu_initialized = false;
static bool gvs_manual_menu_creation_in_progress = false;
static lv_obj_t *gvs_manual_mask = NULL;

// Массив указателей на label для значений параметров (3 параметра)
static lv_obj_t *value_labels[3] = {NULL};

// Состояние редактирования
static bool edit_mode = false;
static int editing_param_index = -1;
static gvs_manual_pump_t editing_pump1_value = GVS_MANUAL_PUMP_OFF;
static gvs_manual_pump_t editing_pump2_value = GVS_MANUAL_PUMP_OFF;
static gvs_manual_valve_t editing_valve_value = GVS_MANUAL_VALVE_STOP;

// Временные значения для отмены изменений
static gvs_manual_pump_t temp_GVS_N1_DControl = GVS_MANUAL_PUMP_OFF;
static gvs_manual_pump_t temp_GVS_N2_DControl = GVS_MANUAL_PUMP_OFF;
static gvs_manual_valve_t temp_GVS_M_IControl = GVS_MANUAL_VALVE_STOP;

/**
 * @brief Проверяет, является ли объект валидным
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Получает строковое представление состояния насоса 1
 */
static const char* get_pump1_string(gvs_manual_pump_t pump) {
    return (pump == GVS_MANUAL_PUMP_OFF) ? "ВЫКЛ" : "ВКЛ";
}

/**
 * @brief Получает строковое представление состояния насоса 2
 */
static const char* get_pump2_string(gvs_manual_pump_t pump) {
    return (pump == GVS_MANUAL_PUMP_OFF) ? "ВЫКЛ" : "ВКЛ";
}

/**
 * @brief Получает строковое представление состояния клапана
 */
static const char* get_valve_string(gvs_manual_valve_t valve) {
    switch(valve) {
        case GVS_MANUAL_VALVE_CLOSED: return "ЗАКР";
        case GVS_MANUAL_VALVE_OPEN: return "ОТКР";
        case GVS_MANUAL_VALVE_STOP: return "СТОП";
        default: return "???";
    }
}

/**
 * @brief Подсветка выбранного элемента меню ручной режим ГВС
 */
static void gvs_manual_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
    if (!is_obj_valid(cont)) return;
    
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!is_obj_valid(child)) continue;
        
        bool is_selected = (i == cursor_index);
        bool is_editing_this = (edit_mode && editing_param_index == gvs_manual_menu_items[i].param_index);
        
        uint32_t grand_child_cnt = lv_obj_get_child_cnt(child);
        
        for (uint32_t j = 0; j < grand_child_cnt; j++) {
            lv_obj_t *grand_child = lv_obj_get_child(child, j);
            if (!is_obj_valid(grand_child)) continue;
            
            // Проверяем, является ли это контейнером значения параметра
            bool is_value_container = false;
            for (int k = 0; k < 3; k++) {
                if (value_labels[k] != NULL && lv_obj_get_parent(value_labels[k]) == grand_child) {
                    is_value_container = true;
                    break;
                }
            }
            
            if (lv_obj_check_type(grand_child, &lv_label_class)) {
                // Проверяем, является ли это label значения параметра
                bool is_value_label = false;
                for (int k = 0; k < 3; k++) {
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
    if (param_index < 0 || param_index >= 3) return;
    if (!is_obj_valid(value_labels[param_index])) return;
    
    char value_str[32];
    
    if (edit_mode && editing_param_index == param_index) {
        // В режиме редактирования показываем временное значение
        switch(param_index) {
            case 0: // GVS_N1-DControl
                snprintf(value_str, sizeof(value_str), "%s", get_pump1_string(editing_pump1_value));
                break;
            case 1: // GVS_N2-DControl
                snprintf(value_str, sizeof(value_str), "%s", get_pump2_string(editing_pump2_value));
                break;
            case 2: // GVS_M-IControl
                snprintf(value_str, sizeof(value_str), "%s", get_valve_string(editing_valve_value));
                break;
        }
    } else {
        // Обычный режим - показываем текущее значение
        switch(param_index) {
            case 0: // GVS_N1-DControl
                snprintf(value_str, sizeof(value_str), "%s", get_pump1_string(GVS_N1_DControl));
                break;
            case 1: // GVS_N2-DControl
                snprintf(value_str, sizeof(value_str), "%s", get_pump2_string(GVS_N2_DControl));
                break;
            case 2: // GVS_M-IControl
                snprintf(value_str, sizeof(value_str), "%s", get_valve_string(GVS_M_IControl));
                break;
        }
    }
    
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
        case 0: GVS_N1_DControl = editing_pump1_value; break;
        case 1: GVS_N2_DControl = editing_pump2_value; break;
        case 2: GVS_M_IControl = editing_valve_value; break;
    }
    
    // НЕ сохраняем параметры в NVS (как в ГВС)
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_MANUAL);
    if (menu_state) {
        gvs_manual_highlight_box(gvs_manual_cont, menu_state->cursor_index);
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
        case 0: editing_pump1_value = temp_GVS_N1_DControl; break;
        case 1: editing_pump2_value = temp_GVS_N2_DControl; break;
        case 2: editing_valve_value = temp_GVS_M_IControl; break;
    }
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_MANUAL);
    if (menu_state) {
        gvs_manual_highlight_box(gvs_manual_cont, menu_state->cursor_index);
    }
}

/**
 * @brief Входит в режим редактирования параметра
 */
static void enter_edit_mode(int param_index) {
    if (param_index < 0 || param_index >= 3) return;
    
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
            temp_GVS_N1_DControl = GVS_N1_DControl;
            editing_pump1_value = GVS_N1_DControl;
            break;
        case 1:
            temp_GVS_N2_DControl = GVS_N2_DControl;
            editing_pump2_value = GVS_N2_DControl;
            break;
        case 2:
            temp_GVS_M_IControl = GVS_M_IControl;
            editing_valve_value = GVS_M_IControl;
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
        case 0:
            value_changed = (editing_pump1_value != temp_GVS_N1_DControl);
            break;
        case 1:
            value_changed = (editing_pump2_value != temp_GVS_N2_DControl);
            break;
        case 2:
            value_changed = (editing_valve_value != temp_GVS_M_IControl);
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
 * @brief Создание элемента меню ручной режим ГВС
 */
static void create_gvs_manual_menu_item(lv_obj_t *cont, const GvsManualMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_gvs_manual_menu_item");
        return;
    }
    
    // Создаем контейнер для элемента (высота 40px)
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_gvs_manual_menu_item");
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
 * @brief Показывает меню ручной режим ГВС
 */
void gvs_manual_menu_show(void) {
    ESP_LOGI(TAG, "Showing GVS manual menu");
    if (is_obj_valid(gvs_manual_cont)) {
        lv_obj_clear_flag(gvs_manual_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(gvs_manual_mask)) {
        gvs_manual_mask = radial();
        if (is_obj_valid(gvs_manual_mask)) {
            lv_obj_set_pos(gvs_manual_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(gvs_manual_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает меню ручной режим ГВС
 */
void gvs_manual_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding GVS manual menu");
    if (is_obj_valid(gvs_manual_cont)) {
        lv_obj_add_flag(gvs_manual_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(gvs_manual_mask)) {
        lv_obj_del(gvs_manual_mask);
        gvs_manual_mask = NULL;
    }
}

/**
 * @brief Обработчик событий энкодера для меню ручной режим ГВС
 */
void gvs_manual_menu_encoder_event_cb(uint8_t e) {
    // Обновляем таймер активности при любом действии пользователя
    access_control_update_activity_timer();
    
    // Если активно окно подтверждения, передаем события ему
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(gvs_manual_cont)) {
        ESP_LOGE(TAG, "Контейнер меню ручной режим ГВС не инициализирован");
        return;
    }
    
    // Если в режиме редактирования
    if (edit_mode && editing_param_index >= 0) {
        if (e & ENC_LEFT || e & ENC_RIGHT) {
            // Переключаем enum значения
            switch(editing_param_index) {
                case 0: // GVS_N1-DControl
                    editing_pump1_value = (editing_pump1_value == GVS_MANUAL_PUMP_OFF) ? 
                                         GVS_MANUAL_PUMP_ON : GVS_MANUAL_PUMP_OFF;
                    break;
                case 1: // GVS_N2-DControl
                    editing_pump2_value = (editing_pump2_value == GVS_MANUAL_PUMP_OFF) ? 
                                         GVS_MANUAL_PUMP_ON : GVS_MANUAL_PUMP_OFF;
                    break;
                case 2: // GVS_M-IControl
                    // Циклическое переключение: ЗАКР -> ОТКР -> СТОП -> ЗАКР
                    if (e & ENC_RIGHT) {
                        if (editing_valve_value == GVS_MANUAL_VALVE_STOP) {
                            editing_valve_value = GVS_MANUAL_VALVE_CLOSED;
                        } else {
                            editing_valve_value = (gvs_manual_valve_t)((int)editing_valve_value + 1);
                        }
                    } else { // ENC_LEFT
                        if (editing_valve_value == GVS_MANUAL_VALVE_CLOSED) {
                            editing_valve_value = GVS_MANUAL_VALVE_STOP;
                        } else {
                            editing_valve_value = (gvs_manual_valve_t)((int)editing_valve_value - 1);
                        }
                    }
                    break;
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_CLICK) {
            // Выходим из режима редактирования
            exit_edit_mode_with_confirmation();
        }
        return;
    }
    
    // Обычный режим навигации
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_MANUAL);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, gvs_manual_cont, menu_state, MENU_TYPE_GVS_MANUAL);
    
    if (prev_cursor != menu_state->cursor_index) {
        gvs_manual_highlight_box(gvs_manual_cont, menu_state->cursor_index);
    }
    
    // Обработка нажатия кнопки
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в меню ГВС
            ESP_LOGI(TAG, "Returning to GVS menu from manual menu");
            gvs_manual_menu_hide();
            gvs_menu_show();
            // Переключаем обработчик энкодера
            extern void gvs_menu_encoder_event_cb(uint8_t e);
            encoder_manager_register_callback(gvs_menu_encoder_event_cb);
        } else {
            // Входим в режим редактирования
            int param_index = gvs_manual_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_edit_mode(param_index);
            }
        }
    }
}

/**
 * @brief Очистка меню ручной режим ГВС
 */
void gvs_manual_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up GVS manual menu");
    
    gvs_manual_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    // Очищаем массив указателей на labels
    for (int i = 0; i < 3; i++) {
        value_labels[i] = NULL;
    }
    
    if (is_obj_valid(gvs_manual_mask)) {
        lv_obj_del(gvs_manual_mask);
        gvs_manual_mask = NULL;
    }
    
    if (is_obj_valid(gvs_manual_cont)) {
        lv_obj_del(gvs_manual_cont);
        gvs_manual_cont = NULL;
    }
    
    gvs_manual_menu_initialized = false;
}

/**
 * @brief Инициализация меню ручной режим ГВС
 */
void GVS_Manual_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню ручной режим ГВС");
    
    if (gvs_manual_menu_creation_in_progress) {
        ESP_LOGW(TAG, "GVS manual menu creation already in progress, skipping");
        return;
    }
    
    gvs_manual_menu_creation_in_progress = true;
    
    if (gvs_manual_menu_initialized && is_obj_valid(gvs_manual_cont)) {
        ESP_LOGI(TAG, "GVS manual menu already initialized, showing it");
        gvs_manual_menu_show();
        gvs_manual_menu_creation_in_progress = false;
        return;
    }
    
    gvs_manual_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    gvs_manual_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(gvs_manual_cont)) {
        ESP_LOGE(TAG, "Failed to create GVS manual menu container");
        gvs_manual_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(gvs_manual_cont, 1200, 1200);
    lv_obj_center(gvs_manual_cont);
    lv_obj_add_event_cb(gvs_manual_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(gvs_manual_cont, &style, 0);
    lv_obj_set_style_radius(gvs_manual_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(gvs_manual_cont, true, 0);
    lv_obj_set_scroll_dir(gvs_manual_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(gvs_manual_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(gvs_manual_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(gvs_manual_cont, 633, 0);
    lv_obj_set_style_bg_color(gvs_manual_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(gvs_manual_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(gvs_manual_cont, 0, 0);
    lv_obj_set_style_pad_row(gvs_manual_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(gvs_manual_menu_items) / sizeof(GvsManualMenuItem); i++) {
        create_gvs_manual_menu_item(gvs_manual_cont, &gvs_manual_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    gvs_manual_mask = radial();
    if (is_obj_valid(gvs_manual_mask)) {
        lv_obj_set_pos(gvs_manual_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_GVS_MANUAL);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_MANUAL);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(gvs_manual_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    gvs_manual_highlight_box(gvs_manual_cont, menu_state->cursor_index);
    arc_menu_update_slide(gvs_manual_cont);
    
    gvs_manual_menu_initialized = true;
    gvs_manual_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню ручной режим ГВС успешно инициализировано");
}


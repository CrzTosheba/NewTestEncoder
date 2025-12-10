#include "CO_manual_menu.h"
#include "CO_manual_params.h"
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

static const char *TAG = "CO_MANUAL_MENU";

// Forward declarations
static void update_param_display(int param_index);

// Структура элемента меню ручной режим
typedef struct {
    const char *label_text;     // Текст элемента слева
    const void *img_src;        // Иконка (только для "Назад")
    int param_index;            // Индекс параметра (-1 для "Назад")
} CoManualMenuItem;

// Элементы меню ручной режим
static const CoManualMenuItem co_manual_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Насос 1", NULL, 0},                       // N1-DControl (enum)
    {"Насос 2", NULL, 1},                       // N2-DControl (enum)
    {"Клапан", NULL, 2},                         // M-IControl (enum)
};

// Локальные переменные для меню ручной режим
lv_obj_t *co_manual_cont = NULL;
static bool co_manual_menu_initialized = false;
static bool co_manual_menu_creation_in_progress = false;
static lv_obj_t *co_manual_mask = NULL;

// Массив указателей на label для значений параметров (3 параметра)
static lv_obj_t *value_labels[3] = {NULL};

// Состояние редактирования
static bool edit_mode = false;
static int editing_param_index = -1;
static manual_pump1_t editing_pump1_value = MANUAL_PUMP1_OFF;
static manual_pump2_t editing_pump2_value = MANUAL_PUMP2_OFF;
static manual_valve_t editing_valve_value = MANUAL_VALVE_STOP;

// Временные значения для отмены изменений
static manual_pump1_t temp_N1_DControl = MANUAL_PUMP1_OFF;
static manual_pump2_t temp_N2_DControl = MANUAL_PUMP2_OFF;
static manual_valve_t temp_M_IControl = MANUAL_VALVE_STOP;

/**
 * @brief Проверяет, является ли объект валидным
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Получает строковое представление состояния насоса 1
 */
static const char* get_pump1_string(manual_pump1_t pump) {
    return (pump == MANUAL_PUMP1_OFF) ? "ВЫКЛ" : "ВКЛ";
}

/**
 * @brief Получает строковое представление состояния насоса 2
 */
static const char* get_pump2_string(manual_pump2_t pump) {
    return (pump == MANUAL_PUMP2_OFF) ? "ВЫКЛ" : "ВКЛ";
}

/**
 * @brief Получает строковое представление состояния клапана
 */
static const char* get_valve_string(manual_valve_t valve) {
    switch(valve) {
        case MANUAL_VALVE_CLOSED: return "ЗАКР";
        case MANUAL_VALVE_OPEN: return "ОТКР";
        case MANUAL_VALVE_STOP: return "СТОП";
        default: return "???";
    }
}

/**
 * @brief Подсветка выбранного элемента меню ручной режим
 */
static void co_manual_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
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
                    if (!(edit_mode && editing_param_index == co_manual_menu_items[i].param_index)) {
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
            if (!(edit_mode && editing_param_index == co_manual_menu_items[i].param_index)) {
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
    if (param_index < 0 || param_index >= 3) return;
    if (!is_obj_valid(value_labels[param_index])) return;
    
    char value_str[32];
    
    if (edit_mode && editing_param_index == param_index) {
        // В режиме редактирования показываем временное значение
        switch(param_index) {
            case 0: // N1-DControl
                snprintf(value_str, sizeof(value_str), "%s", get_pump1_string(editing_pump1_value));
                break;
            case 1: // N2-DControl
                snprintf(value_str, sizeof(value_str), "%s", get_pump2_string(editing_pump2_value));
                break;
            case 2: // M-IControl
                snprintf(value_str, sizeof(value_str), "%s", get_valve_string(editing_valve_value));
                break;
        }
    } else {
        // Обычный режим - показываем текущее значение
        switch(param_index) {
            case 0: // N1-DControl
                snprintf(value_str, sizeof(value_str), "%s", get_pump1_string(N1_DControl));
                break;
            case 1: // N2-DControl
                snprintf(value_str, sizeof(value_str), "%s", get_pump2_string(N2_DControl));
                break;
            case 2: // M-IControl
                snprintf(value_str, sizeof(value_str), "%s", get_valve_string(M_IControl));
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
        case 0: N1_DControl = editing_pump1_value; break;
        case 1: N2_DControl = editing_pump2_value; break;
        case 2: M_IControl = editing_valve_value; break;
    }
    
    // НЕ сохраняем параметры в NVS (как в ГВС)
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_MANUAL);
    if (menu_state) {
        co_manual_highlight_box(co_manual_cont, menu_state->cursor_index);
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
        case 0: editing_pump1_value = temp_N1_DControl; break;
        case 1: editing_pump2_value = temp_N2_DControl; break;
        case 2: editing_valve_value = temp_M_IControl; break;
    }
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_MANUAL);
    if (menu_state) {
        co_manual_highlight_box(co_manual_cont, menu_state->cursor_index);
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
            temp_N1_DControl = N1_DControl;
            editing_pump1_value = N1_DControl;
            break;
        case 1:
            temp_N2_DControl = N2_DControl;
            editing_pump2_value = N2_DControl;
            break;
        case 2:
            temp_M_IControl = M_IControl;
            editing_valve_value = M_IControl;
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
            value_changed = (editing_pump1_value != temp_N1_DControl);
            break;
        case 1:
            value_changed = (editing_pump2_value != temp_N2_DControl);
            break;
        case 2:
            value_changed = (editing_valve_value != temp_M_IControl);
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
 * @brief Создание элемента меню ручной режим
 */
static void create_co_manual_menu_item(lv_obj_t *cont, const CoManualMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_co_manual_menu_item");
        return;
    }
    
    // Создаем контейнер для элемента (высота 40px)
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_co_manual_menu_item");
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
 * @brief Показывает меню ручной режим
 */
void co_manual_menu_show(void) {
    ESP_LOGI(TAG, "Showing CO manual menu");
    if (is_obj_valid(co_manual_cont)) {
        lv_obj_clear_flag(co_manual_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(co_manual_mask)) {
        co_manual_mask = radial();
        if (is_obj_valid(co_manual_mask)) {
            lv_obj_set_pos(co_manual_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(co_manual_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает меню ручной режим
 */
void co_manual_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding CO manual menu");
    if (is_obj_valid(co_manual_cont)) {
        lv_obj_add_flag(co_manual_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(co_manual_mask)) {
        lv_obj_del(co_manual_mask);
        co_manual_mask = NULL;
    }
}

/**
 * @brief Обработчик событий энкодера для меню ручной режим
 */
void co_manual_menu_encoder_event_cb(uint8_t e) {
    // Если активно окно подтверждения, передаем события ему
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(co_manual_cont)) {
        ESP_LOGE(TAG, "Контейнер меню ручной режим не инициализирован");
        return;
    }
    
    // Если в режиме редактирования
    if (edit_mode && editing_param_index >= 0) {
        if (e & ENC_LEFT || e & ENC_RIGHT) {
            // Переключаем enum значения
            switch(editing_param_index) {
                case 0: // N1-DControl
                    editing_pump1_value = (editing_pump1_value == MANUAL_PUMP1_OFF) ? 
                                         MANUAL_PUMP1_ON : MANUAL_PUMP1_OFF;
                    break;
                case 1: // N2-DControl
                    editing_pump2_value = (editing_pump2_value == MANUAL_PUMP2_OFF) ? 
                                         MANUAL_PUMP2_ON : MANUAL_PUMP2_OFF;
                    break;
                case 2: // M-IControl
                    // Циклическое переключение: ЗАКР -> ОТКР -> СТОП -> ЗАКР
                    if (e & ENC_RIGHT) {
                        if (editing_valve_value == MANUAL_VALVE_STOP) {
                            editing_valve_value = MANUAL_VALVE_CLOSED;
                        } else {
                            editing_valve_value = (manual_valve_t)((int)editing_valve_value + 1);
                        }
                    } else { // ENC_LEFT
                        if (editing_valve_value == MANUAL_VALVE_CLOSED) {
                            editing_valve_value = MANUAL_VALVE_STOP;
                        } else {
                            editing_valve_value = (manual_valve_t)((int)editing_valve_value - 1);
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
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_MANUAL);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, co_manual_cont, menu_state, MENU_TYPE_CO_MANUAL);
    
    if (prev_cursor != menu_state->cursor_index) {
        co_manual_highlight_box(co_manual_cont, menu_state->cursor_index);
    }
    
    // Обработка нажатия кнопки
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в меню отопления
            ESP_LOGI(TAG, "Returning to CO menu from manual menu");
            co_manual_menu_hide();
            co_menu_show();
            // Переключаем обработчик энкодера
            extern void co_menu_encoder_event_cb(uint8_t e);
            encoder_manager_register_callback(co_menu_encoder_event_cb);
        } else {
            // Входим в режим редактирования
            int param_index = co_manual_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_edit_mode(param_index);
            }
        }
    }
}

/**
 * @brief Очистка меню ручной режим
 */
void co_manual_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up CO manual menu");
    
    co_manual_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    // Очищаем массив указателей на labels
    for (int i = 0; i < 3; i++) {
        value_labels[i] = NULL;
    }
    
    if (is_obj_valid(co_manual_mask)) {
        lv_obj_del(co_manual_mask);
        co_manual_mask = NULL;
    }
    
    if (is_obj_valid(co_manual_cont)) {
        lv_obj_del(co_manual_cont);
        co_manual_cont = NULL;
    }
    
    co_manual_menu_initialized = false;
}

/**
 * @brief Инициализация меню ручной режим
 */
void CO_Manual_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню ручной режим");
    
    if (co_manual_menu_creation_in_progress) {
        ESP_LOGW(TAG, "CO manual menu creation already in progress, skipping");
        return;
    }
    
    co_manual_menu_creation_in_progress = true;
    
    if (co_manual_menu_initialized && is_obj_valid(co_manual_cont)) {
        ESP_LOGI(TAG, "CO manual menu already initialized, showing it");
        co_manual_menu_show();
        co_manual_menu_creation_in_progress = false;
        return;
    }
    
    co_manual_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    co_manual_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(co_manual_cont)) {
        ESP_LOGE(TAG, "Failed to create CO manual menu container");
        co_manual_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(co_manual_cont, 1200, 1200);
    lv_obj_center(co_manual_cont);
    lv_obj_add_event_cb(co_manual_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(co_manual_cont, &style, 0);
    lv_obj_set_style_radius(co_manual_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(co_manual_cont, true, 0);
    lv_obj_set_scroll_dir(co_manual_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(co_manual_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(co_manual_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(co_manual_cont, 633, 0);
    lv_obj_set_style_bg_color(co_manual_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(co_manual_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(co_manual_cont, 0, 0);
    lv_obj_set_style_pad_row(co_manual_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(co_manual_menu_items) / sizeof(CoManualMenuItem); i++) {
        create_co_manual_menu_item(co_manual_cont, &co_manual_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    co_manual_mask = radial();
    if (is_obj_valid(co_manual_mask)) {
        lv_obj_set_pos(co_manual_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_CO_MANUAL);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_MANUAL);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(co_manual_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    co_manual_highlight_box(co_manual_cont, menu_state->cursor_index);
    arc_menu_update_slide(co_manual_cont);
    
    co_manual_menu_initialized = true;
    co_manual_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню ручной режим успешно инициализировано");
}


#include "CO_dry_run_menu.h"
#include "CO_dry_run_params.h"
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
#include "CO_alarms_menu.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CO_DRY_RUN_MENU";

// Forward declarations
static void update_param_display(int param_index);
static const char* get_enalarm_string(co_ps_enalarm_t value);
static const char* get_alarm_rtype_string(co_ps_alarm_rtype_t value);

// Структура элемента меню сухого хода
typedef struct {
    const char *label_text;
    const void *img_src;
    int param_index;
} CoDryRunMenuItem;

// Элементы меню сухого хода
static const CoDryRunMenuItem co_dry_run_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Активация", NULL, 0},                    // PS-EnAlarm (int)
    {"Задержка, с", NULL, 1},                   // PS-AlarmDelay (int)
    {"Сброс", NULL, 2},                         // PS-AlarmRType (int)
};

// Локальные переменные
lv_obj_t *co_dry_run_cont = NULL;
static bool co_dry_run_menu_initialized = false;
static bool co_dry_run_menu_creation_in_progress = false;
static lv_obj_t *co_dry_run_mask = NULL;

// Массив указателей на label для значений параметров (3 параметра)
static lv_obj_t *value_labels[3] = {NULL};

// Состояние редактирования
static bool edit_mode = false;
static int editing_param_index = -1;
static int editing_int_value = 0;

// Временные значения для отмены изменений
static co_ps_enalarm_t temp_PS_EnAlarm = CO_PS_ENALARM_NO;
static int temp_PS_AlarmDelay = 0;
static co_ps_alarm_rtype_t temp_PS_AlarmRType = CO_PS_ALARM_RTYPE_MANUAL_3;

static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Преобразует enum активации в строку
 */
static const char* get_enalarm_string(co_ps_enalarm_t value) {
    return (value == CO_PS_ENALARM_NO) ? "НЕТ" : "ДА";
}

/**
 * @brief Преобразует enum сброса в строку
 */
static const char* get_alarm_rtype_string(co_ps_alarm_rtype_t value) {
    switch(value) {
        case CO_PS_ALARM_RTYPE_AUTO: return "АВТО";
        case CO_PS_ALARM_RTYPE_MANUAL: return "РУЧН";
        case CO_PS_ALARM_RTYPE_MANUAL_1: return "РУЧН-1";
        case CO_PS_ALARM_RTYPE_MANUAL_2: return "РУЧН-2";
        case CO_PS_ALARM_RTYPE_MANUAL_3: return "РУЧН-3";
        case CO_PS_ALARM_RTYPE_MANUAL_4: return "РУЧН-4";
        case CO_PS_ALARM_RTYPE_MANUAL_5: return "РУЧН-5";
        case CO_PS_ALARM_RTYPE_MANUAL_6: return "РУЧН-6";
        case CO_PS_ALARM_RTYPE_MANUAL_7: return "РУЧН-7";
        case CO_PS_ALARM_RTYPE_MANUAL_8: return "РУЧН-8";
        case CO_PS_ALARM_RTYPE_MANUAL_9: return "РУЧН-9";
        case CO_PS_ALARM_RTYPE_MANUAL_10: return "РУЧН-10";
        default: return "???";
    }
}

static void co_dry_run_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
    if (!is_obj_valid(cont)) return;
    
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!is_obj_valid(child)) continue;
        
        bool is_selected = (i == cursor_index);
        bool is_editing_this = (edit_mode && editing_param_index == co_dry_run_menu_items[i].param_index);
        
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

static void update_param_display(int param_index) {
    if (param_index < 0 || param_index >= 3) return;
    if (!is_obj_valid(value_labels[param_index])) return;
    
    char value_str[32];
    
    if (edit_mode && editing_param_index == param_index) {
        // В режиме редактирования
        if (param_index == 0) {
            // Активация - enum НЕТ/ДА
            snprintf(value_str, sizeof(value_str), "%s", 
                     get_enalarm_string((co_ps_enalarm_t)editing_int_value));
        } else if (param_index == 1) {
            // Задержка - int
            snprintf(value_str, sizeof(value_str), "%d", editing_int_value);
        } else if (param_index == 2) {
            // Сброс - enum АВТО/РУЧН/РУЧН-1/.../РУЧН-10
            snprintf(value_str, sizeof(value_str), "%s", 
                     get_alarm_rtype_string((co_ps_alarm_rtype_t)editing_int_value));
        }
    } else {
        // Обычный режим
        if (param_index == 0) {
            snprintf(value_str, sizeof(value_str), "%s", get_enalarm_string(PS_EnAlarm));
        } else if (param_index == 1) {
            snprintf(value_str, sizeof(value_str), "%d", PS_AlarmDelay);
        } else if (param_index == 2) {
            snprintf(value_str, sizeof(value_str), "%s", get_alarm_rtype_string(PS_AlarmRType));
        }
    }
    
    char full_str[40];
    if (param_index == 1) {
        snprintf(full_str, sizeof(full_str), "%s", value_str);
    } else {
        snprintf(full_str, sizeof(full_str), "%s", value_str);
    }
    
    lv_label_set_text(value_labels[param_index], full_str);
    
    if (!is_obj_valid(value_labels[param_index])) return;
    
    lv_obj_set_style_text_font(value_labels[param_index], &Roboto_bold_24, LV_PART_MAIN);
    
    lv_obj_t *value_container = lv_obj_get_parent(value_labels[param_index]);
    if (!is_obj_valid(value_container)) return;
    
    if (edit_mode && editing_param_index == param_index) {
        lv_obj_set_style_bg_color(value_container, lv_color_hex(0xE9EBEB), LV_PART_MAIN);
        lv_obj_set_style_text_color(value_labels[param_index], lv_color_hex(0x101315), LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(value_container, lv_color_hex(0x2B3639), LV_PART_MAIN);
        lv_obj_set_style_text_color(value_labels[param_index], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    }
}

static void save_param_changes(void) {
    ESP_LOGI(TAG, "Saving parameter changes for index %d", editing_param_index);
    
    int saved_index = editing_param_index;
    
    switch(editing_param_index) {
        case 0: PS_EnAlarm = (co_ps_enalarm_t)editing_int_value; break;
        case 1: PS_AlarmDelay = editing_int_value; break;
        case 2: PS_AlarmRType = (co_ps_alarm_rtype_t)editing_int_value; break;
    }
    
    // НЕ сохраняем параметры в NVS (как в ГВС)
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_ALARMS_DRY_RUN);
    if (menu_state) {
        co_dry_run_highlight_box(co_dry_run_cont, menu_state->cursor_index);
    }
}

static void cancel_param_changes(void) {
    ESP_LOGI(TAG, "Canceling parameter changes");
    
    int saved_index = editing_param_index;
    
    switch(editing_param_index) {
        case 0: editing_int_value = (int)temp_PS_EnAlarm; break;
        case 1: editing_int_value = temp_PS_AlarmDelay; break;
        case 2: editing_int_value = (int)temp_PS_AlarmRType; break;
    }
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_ALARMS_DRY_RUN);
    if (menu_state) {
        co_dry_run_highlight_box(co_dry_run_cont, menu_state->cursor_index);
    }
}

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
    
    switch(param_index) {
        case 0:
            temp_PS_EnAlarm = PS_EnAlarm;
            editing_int_value = (int)PS_EnAlarm;
            break;
        case 1:
            temp_PS_AlarmDelay = PS_AlarmDelay;
            editing_int_value = PS_AlarmDelay;
            break;
        case 2:
            temp_PS_AlarmRType = PS_AlarmRType;
            editing_int_value = (int)PS_AlarmRType;
            break;
    }
    
    update_param_display(param_index);
}

static void exit_edit_mode_with_confirmation(void) {
    if (!edit_mode || editing_param_index < 0) return;
    
    bool value_changed = false;
    
    switch(editing_param_index) {
        case 0: value_changed = (editing_int_value != (int)temp_PS_EnAlarm); break;
        case 1: value_changed = (editing_int_value != temp_PS_AlarmDelay); break;
        case 2: value_changed = (editing_int_value != (int)temp_PS_AlarmRType); break;
    }
    
    if (value_changed) {
        create_yes_no_screen_with_callbacks(save_param_changes, cancel_param_changes);
    } else {
        cancel_param_changes();
    }
}

static void create_co_dry_run_menu_item(lv_obj_t *cont, const CoDryRunMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_co_dry_run_menu_item");
        return;
    }
    
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_co_dry_run_menu_item");
        return;
    }
    
    lv_obj_set_size(box, 462, 40);
    lv_obj_set_style_border_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_border_width(box, 0, 0);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_radius(box, 0, 0);
    
    lv_obj_t *label = lv_label_create(box);
    if (is_obj_valid(label)) {
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
        lv_label_set_text(label, item->label_text);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, -5, 0);
    }
    
    if (item->img_src != NULL) {
        lv_obj_t *img = lv_img_create(box);
        if (is_obj_valid(img)) {
            lv_img_set_src(img, item->img_src);
            lv_obj_set_style_img_recolor(img, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
            lv_obj_align(img, LV_ALIGN_CENTER, 90, 0);
        }
    }
    
    if (item->param_index >= 0) {
        lv_obj_t *value_container = lv_obj_create(box);
        if (is_obj_valid(value_container)) {
            lv_obj_set_size(value_container, 83, 40);
            lv_obj_set_style_bg_color(value_container, lv_color_hex(0x2B3639), LV_PART_MAIN);
            lv_obj_set_style_border_color(value_container, lv_color_hex(0x2B3639), LV_PART_MAIN);
            lv_obj_set_style_border_width(value_container, 0, 0);
            lv_obj_set_style_radius(value_container, 0, 0);
            lv_obj_set_style_pad_all(value_container, 0, 0);
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

void co_dry_run_menu_show(void) {
    ESP_LOGI(TAG, "Showing CO dry run menu");
    if (is_obj_valid(co_dry_run_cont)) {
        lv_obj_clear_flag(co_dry_run_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(co_dry_run_mask)) {
        co_dry_run_mask = radial();
        if (is_obj_valid(co_dry_run_mask)) {
            lv_obj_set_pos(co_dry_run_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(co_dry_run_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

void co_dry_run_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding CO dry run menu");
    if (is_obj_valid(co_dry_run_cont)) {
        lv_obj_add_flag(co_dry_run_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(co_dry_run_mask)) {
        lv_obj_del(co_dry_run_mask);
        co_dry_run_mask = NULL;
    }
}

void co_dry_run_menu_encoder_event_cb(uint8_t e) {
    // Обновляем таймер активности при любом действии пользователя
    access_control_update_activity_timer();
    
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(co_dry_run_cont)) {
        ESP_LOGE(TAG, "Контейнер меню сухого хода не инициализирован");
        return;
    }
    
    if (edit_mode && editing_param_index >= 0) {
        if (e & ENC_LEFT) {
            int step = co_dry_run_param_limits_int[editing_param_index].step;
            editing_int_value -= step;
            if (editing_int_value < co_dry_run_param_limits_int[editing_param_index].min) {
                // Для enum параметров делаем циклическое переключение
                if (editing_param_index == 0 || editing_param_index == 2) {
                    editing_int_value = co_dry_run_param_limits_int[editing_param_index].max;
                } else {
                    editing_int_value = co_dry_run_param_limits_int[editing_param_index].min;
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_RIGHT) {
            int step = co_dry_run_param_limits_int[editing_param_index].step;
            editing_int_value += step;
            if (editing_int_value > co_dry_run_param_limits_int[editing_param_index].max) {
                // Для enum параметров делаем циклическое переключение
                if (editing_param_index == 0 || editing_param_index == 2) {
                    editing_int_value = co_dry_run_param_limits_int[editing_param_index].min;
                } else {
                    editing_int_value = co_dry_run_param_limits_int[editing_param_index].max;
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_CLICK) {
            exit_edit_mode_with_confirmation();
        }
        return;
    }
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_ALARMS_DRY_RUN);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, co_dry_run_cont, menu_state, MENU_TYPE_CO_ALARMS_DRY_RUN);
    
    if (prev_cursor != menu_state->cursor_index) {
        co_dry_run_highlight_box(co_dry_run_cont, menu_state->cursor_index);
    }
    
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            ESP_LOGI(TAG, "Returning to alarms menu from dry run menu");
            co_dry_run_menu_hide();
            co_alarms_menu_show();
            encoder_manager_register_callback(co_alarms_menu_encoder_event_cb);
        } else {
            int param_index = co_dry_run_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_edit_mode(param_index);
            }
        }
    }
}

void co_dry_run_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up CO dry run menu");
    
    co_dry_run_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    for (int i = 0; i < 3; i++) {
        value_labels[i] = NULL;
    }
    
    if (is_obj_valid(co_dry_run_mask)) {
        lv_obj_del(co_dry_run_mask);
        co_dry_run_mask = NULL;
    }
    
    if (is_obj_valid(co_dry_run_cont)) {
        lv_obj_del(co_dry_run_cont);
        co_dry_run_cont = NULL;
    }
    
    co_dry_run_menu_initialized = false;
}

void CO_Dry_Run_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню сухого хода");
    
    if (co_dry_run_menu_creation_in_progress) {
        ESP_LOGW(TAG, "CO dry run menu creation already in progress, skipping");
        return;
    }
    
    co_dry_run_menu_creation_in_progress = true;
    
    if (co_dry_run_menu_initialized && is_obj_valid(co_dry_run_cont)) {
        ESP_LOGI(TAG, "CO dry run menu already initialized, showing it");
        co_dry_run_menu_show();
        co_dry_run_menu_creation_in_progress = false;
        return;
    }
    
    co_dry_run_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    co_dry_run_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(co_dry_run_cont)) {
        ESP_LOGE(TAG, "Failed to create CO dry run menu container");
        co_dry_run_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(co_dry_run_cont, 1200, 1200);
    lv_obj_center(co_dry_run_cont);
    lv_obj_add_event_cb(co_dry_run_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(co_dry_run_cont, &style, 0);
    lv_obj_set_style_radius(co_dry_run_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(co_dry_run_cont, true, 0);
    lv_obj_set_scroll_dir(co_dry_run_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(co_dry_run_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(co_dry_run_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(co_dry_run_cont, 633, 0);
    lv_obj_set_style_bg_color(co_dry_run_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(co_dry_run_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(co_dry_run_cont, 0, 0);
    lv_obj_set_style_pad_row(co_dry_run_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(co_dry_run_menu_items) / sizeof(CoDryRunMenuItem); i++) {
        create_co_dry_run_menu_item(co_dry_run_cont, &co_dry_run_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    co_dry_run_mask = radial();
    if (is_obj_valid(co_dry_run_mask)) {
        lv_obj_set_pos(co_dry_run_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_CO_ALARMS_DRY_RUN);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_ALARMS_DRY_RUN);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(co_dry_run_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    co_dry_run_highlight_box(co_dry_run_cont, menu_state->cursor_index);
    arc_menu_update_slide(co_dry_run_cont);
    
    co_dry_run_menu_initialized = true;
    co_dry_run_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню сухого хода успешно инициализировано");
}



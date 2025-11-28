#include "CO_dev_alarm_menu.h"
#include "CO_dev_alarm_params.h"
#include "co_params_limits.h"
#include "encoder/encoder.h"
#include "encoder/encoder_manager.h"
#include "my_widgets/w_rad_mask.h"
#include "screen_logic/arc_menu.h"
#include "screen_logic/menu_config.h"
#include "screen_logic/screen_navigation.h"
#include "screen_logic/screen_container_manager.h"
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

static const char *TAG = "CO_DEV_ALARM_MENU";

static void update_param_display(int param_index);

typedef struct {
    const char *label_text;
    const void *img_src;
    int param_index;
} CoDevAlarmMenuItem;

static const CoDevAlarmMenuItem co_dev_alarm_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Авар.откл.Тпод_СО", NULL, 0},          // T1-EnDevAlarm (int)
    {"Перегрев Тпод_СО", NULL, 1},             // T1-EnHighAlarm (int)
    {"Недогрев Тпод_СО", NULL, 2},             // T1-EnLowAlarm (int)
    {"Задержка, с", NULL, 3},                  // T1-DevAlarmDelay (int)
    {"Сброс", NULL, 4},                        // T1-DevAlarmRType (int)
    {"Макс.откл.Тпод_СО, °C", NULL, 5},       // T1-AlarmDev (float)
};

lv_obj_t *co_dev_alarm_cont = NULL;
static bool co_dev_alarm_menu_initialized = false;
static bool co_dev_alarm_menu_creation_in_progress = false;
static lv_obj_t *co_dev_alarm_mask = NULL;

static lv_obj_t *value_labels[6] = {NULL};

static bool edit_mode = false;
static int editing_param_index = -1;
static int editing_int_value = 0;
static float editing_float_value = 0.0f;

static int temp_T1_EnDevAlarm = 0;
static int temp_T1_EnHighAlarm = 0;
static int temp_T1_EnLowAlarm = 0;
static int temp_T1_DevAlarmDelay = 0;
static int temp_T1_DevAlarmRType = 0;
static float temp_T1_AlarmDev = 0.0f;

static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

static void format_float_value(char *buf, size_t buf_size, float value) {
    if (value < 0.0f) {
        snprintf(buf, buf_size, "-%.1f", -value);
    } else {
        snprintf(buf, buf_size, "%.1f", value);
    }
}

static void co_dev_alarm_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
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
                    if (!(edit_mode && editing_param_index == co_dev_alarm_menu_items[i].param_index)) {
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
        
        if (i == cursor_index) {
            if (!(edit_mode && editing_param_index == co_dev_alarm_menu_items[i].param_index)) {
                lv_obj_set_style_bg_color(child, lv_color_hex(0xFFCC00), LV_PART_MAIN);
            }
        } else {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x2B3639), LV_PART_MAIN);
        }
    }
}

static void update_param_display(int param_index) {
    if (param_index < 0 || param_index >= 6) return;
    if (!is_obj_valid(value_labels[param_index])) return;
    
    char value_str[32];
    bool is_float = (param_index == 5);
    
    bool is_enum = (param_index == 0 || param_index == 1 || param_index == 2 || param_index == 4);  // Enum параметры
    
    if (edit_mode && editing_param_index == param_index) {
        if (is_float) {
            format_float_value(value_str, sizeof(value_str), editing_float_value);
        } else if (is_enum) {
            snprintf(value_str, sizeof(value_str), "%s", 
                     editing_int_value == 0 ? "НЕТ" : "ДА");
        } else {
            snprintf(value_str, sizeof(value_str), "%d", editing_int_value);
        }
    } else {
        switch(param_index) {
            case 0: {
                int value = T1_EnDevAlarm;
                if (is_enum) {
                    snprintf(value_str, sizeof(value_str), "%s", value == 0 ? "НЕТ" : "ДА");
                } else {
                    snprintf(value_str, sizeof(value_str), "%d", value);
                }
                break;
            }
            case 1: {
                int value = T1_EnHighAlarm;
                if (is_enum) {
                    snprintf(value_str, sizeof(value_str), "%s", value == 0 ? "НЕТ" : "ДА");
                } else {
                    snprintf(value_str, sizeof(value_str), "%d", value);
                }
                break;
            }
            case 2: {
                int value = T1_EnLowAlarm;
                if (is_enum) {
                    snprintf(value_str, sizeof(value_str), "%s", value == 0 ? "НЕТ" : "ДА");
                } else {
                    snprintf(value_str, sizeof(value_str), "%d", value);
                }
                break;
            }
            case 3: snprintf(value_str, sizeof(value_str), "%d", T1_DevAlarmDelay); break;
            case 4: {
                int value = T1_DevAlarmRType;
                if (is_enum) {
                    snprintf(value_str, sizeof(value_str), "%s", value == 0 ? "НЕТ" : "ДА");
                } else {
                    snprintf(value_str, sizeof(value_str), "%d", value);
                }
                break;
            }
            case 5: format_float_value(value_str, sizeof(value_str), T1_AlarmDev); break;
        }
    }
    
    char full_str[40];
    if (param_index == 3) {
        snprintf(full_str, sizeof(full_str), "%s с", value_str);
    } else if (param_index == 5) {
        snprintf(full_str, sizeof(full_str), "%s °C", value_str);
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
        case 0: T1_EnDevAlarm = editing_int_value; break;
        case 1: T1_EnHighAlarm = editing_int_value; break;
        case 2: T1_EnLowAlarm = editing_int_value; break;
        case 3: T1_DevAlarmDelay = editing_int_value; break;
        case 4: T1_DevAlarmRType = editing_int_value; break;
        case 5: T1_AlarmDev = editing_float_value; break;
    }
    
    co_dev_alarm_params_save();
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_ALARMS_DEVIATION);
    if (menu_state) {
        co_dev_alarm_highlight_box(co_dev_alarm_cont, menu_state->cursor_index);
    }
}

static void cancel_param_changes(void) {
    ESP_LOGI(TAG, "Canceling parameter changes");
    
    int saved_index = editing_param_index;
    
    switch(editing_param_index) {
        case 0: editing_int_value = temp_T1_EnDevAlarm; break;
        case 1: editing_int_value = temp_T1_EnHighAlarm; break;
        case 2: editing_int_value = temp_T1_EnLowAlarm; break;
        case 3: editing_int_value = temp_T1_DevAlarmDelay; break;
        case 4: editing_int_value = temp_T1_DevAlarmRType; break;
        case 5: editing_float_value = temp_T1_AlarmDev; break;
    }
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_ALARMS_DEVIATION);
    if (menu_state) {
        co_dev_alarm_highlight_box(co_dev_alarm_cont, menu_state->cursor_index);
    }
}

static void enter_edit_mode(int param_index) {
    if (param_index < 0 || param_index >= 6) return;
    
    ESP_LOGI(TAG, "Entering edit mode for parameter %d", param_index);
    
    edit_mode = true;
    editing_param_index = param_index;
    
    switch(param_index) {
        case 0:
            temp_T1_EnDevAlarm = T1_EnDevAlarm;
            editing_int_value = T1_EnDevAlarm;
            break;
        case 1:
            temp_T1_EnHighAlarm = T1_EnHighAlarm;
            editing_int_value = T1_EnHighAlarm;
            break;
        case 2:
            temp_T1_EnLowAlarm = T1_EnLowAlarm;
            editing_int_value = T1_EnLowAlarm;
            break;
        case 3:
            temp_T1_DevAlarmDelay = T1_DevAlarmDelay;
            editing_int_value = T1_DevAlarmDelay;
            break;
        case 4:
            temp_T1_DevAlarmRType = T1_DevAlarmRType;
            editing_int_value = T1_DevAlarmRType;
            break;
        case 5:
            temp_T1_AlarmDev = T1_AlarmDev;
            editing_float_value = T1_AlarmDev;
            break;
    }
    
    update_param_display(param_index);
}

static void exit_edit_mode_with_confirmation(void) {
    if (!edit_mode || editing_param_index < 0) return;
    
    bool value_changed = false;
    
    if (editing_param_index == 5) {
        value_changed = (fabs(editing_float_value - temp_T1_AlarmDev) > 0.01f);
    } else {
        switch(editing_param_index) {
            case 0: value_changed = (editing_int_value != temp_T1_EnDevAlarm); break;
            case 1: value_changed = (editing_int_value != temp_T1_EnHighAlarm); break;
            case 2: value_changed = (editing_int_value != temp_T1_EnLowAlarm); break;
            case 3: value_changed = (editing_int_value != temp_T1_DevAlarmDelay); break;
            case 4: value_changed = (editing_int_value != temp_T1_DevAlarmRType); break;
        }
    }
    
    if (value_changed) {
        create_yes_no_screen_with_callbacks(save_param_changes, cancel_param_changes);
    } else {
        cancel_param_changes();
    }
}

static void create_co_dev_alarm_menu_item(lv_obj_t *cont, const CoDevAlarmMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_co_dev_alarm_menu_item");
        return;
    }
    
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_co_dev_alarm_menu_item");
        return;
    }
    
    lv_obj_set_size(box, 462, 40);
    lv_obj_set_style_border_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_radius(box, 0, 0);
    
    lv_obj_t *label = lv_label_create(box);
    if (is_obj_valid(label)) {
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
        lv_label_set_text(label, item->label_text);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);
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
            lv_obj_set_size(value_container, 150, 40);
            lv_obj_set_style_bg_color(value_container, lv_color_hex(0x2B3639), LV_PART_MAIN);
            lv_obj_set_style_border_color(value_container, lv_color_hex(0x2B3639), LV_PART_MAIN);
            lv_obj_set_style_radius(value_container, 0, 0);
            lv_obj_set_style_pad_all(value_container, 0, 0);
            lv_obj_set_pos(value_container, 200, -23);
            
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

void co_dev_alarm_menu_show(void) {
    ESP_LOGI(TAG, "Showing CO deviation alarm menu");
    if (is_obj_valid(co_dev_alarm_cont)) {
        lv_obj_clear_flag(co_dev_alarm_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(co_dev_alarm_mask)) {
        co_dev_alarm_mask = radial();
        if (is_obj_valid(co_dev_alarm_mask)) {
            lv_obj_set_pos(co_dev_alarm_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(co_dev_alarm_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

void co_dev_alarm_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding CO deviation alarm menu");
    if (is_obj_valid(co_dev_alarm_cont)) {
        lv_obj_add_flag(co_dev_alarm_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(co_dev_alarm_mask)) {
        lv_obj_del(co_dev_alarm_mask);
        co_dev_alarm_mask = NULL;
    }
}

void co_dev_alarm_menu_encoder_event_cb(uint8_t e) {
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(co_dev_alarm_cont)) {
        ESP_LOGE(TAG, "Контейнер меню аварийного отклонения не инициализирован");
        return;
    }
    
    if (edit_mode && editing_param_index >= 0) {
        if (e & ENC_LEFT) {
            if (editing_param_index == 5) {
                float step = co_dev_alarm_param_limits_float[0].step;
                editing_float_value -= step;
                if (editing_float_value < co_dev_alarm_param_limits_float[0].min) {
                    editing_float_value = co_dev_alarm_param_limits_float[0].min;
                }
            } else {
                int step = co_dev_alarm_param_limits_int[editing_param_index].step;
                editing_int_value -= step;
                if (editing_int_value < co_dev_alarm_param_limits_int[editing_param_index].min) {
                    editing_int_value = co_dev_alarm_param_limits_int[editing_param_index].min;
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_RIGHT) {
            if (editing_param_index == 5) {
                float step = co_dev_alarm_param_limits_float[0].step;
                editing_float_value += step;
                if (editing_float_value > co_dev_alarm_param_limits_float[0].max) {
                    editing_float_value = co_dev_alarm_param_limits_float[0].max;
                }
            } else {
                int step = co_dev_alarm_param_limits_int[editing_param_index].step;
                editing_int_value += step;
                if (editing_int_value > co_dev_alarm_param_limits_int[editing_param_index].max) {
                    editing_int_value = co_dev_alarm_param_limits_int[editing_param_index].max;
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_CLICK) {
            exit_edit_mode_with_confirmation();
        }
        return;
    }
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_ALARMS_DEVIATION);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, co_dev_alarm_cont, menu_state, MENU_TYPE_CO_ALARMS_DEVIATION);
    
    if (prev_cursor != menu_state->cursor_index) {
        co_dev_alarm_highlight_box(co_dev_alarm_cont, menu_state->cursor_index);
    }
    
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            ESP_LOGI(TAG, "Returning to alarms menu from deviation alarm menu");
            co_dev_alarm_menu_hide();
            co_alarms_menu_show();
            encoder_manager_register_callback(co_alarms_menu_encoder_event_cb);
        } else {
            int param_index = co_dev_alarm_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_edit_mode(param_index);
            }
        }
    }
}

void co_dev_alarm_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up CO deviation alarm menu");
    
    co_dev_alarm_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    for (int i = 0; i < 6; i++) {
        value_labels[i] = NULL;
    }
    
    if (is_obj_valid(co_dev_alarm_mask)) {
        lv_obj_del(co_dev_alarm_mask);
        co_dev_alarm_mask = NULL;
    }
    
    if (is_obj_valid(co_dev_alarm_cont)) {
        lv_obj_del(co_dev_alarm_cont);
        co_dev_alarm_cont = NULL;
    }
    
    co_dev_alarm_menu_initialized = false;
}

void CO_Dev_Alarm_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню аварийного отклонения");
    
    if (co_dev_alarm_menu_creation_in_progress) {
        ESP_LOGW(TAG, "CO deviation alarm menu creation already in progress, skipping");
        return;
    }
    
    co_dev_alarm_menu_creation_in_progress = true;
    
    if (co_dev_alarm_menu_initialized && is_obj_valid(co_dev_alarm_cont)) {
        ESP_LOGI(TAG, "CO deviation alarm menu already initialized, showing it");
        co_dev_alarm_menu_show();
        co_dev_alarm_menu_creation_in_progress = false;
        return;
    }
    
    co_dev_alarm_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    co_dev_alarm_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(co_dev_alarm_cont)) {
        ESP_LOGE(TAG, "Failed to create CO deviation alarm menu container");
        co_dev_alarm_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(co_dev_alarm_cont, 1200, 1200);
    lv_obj_center(co_dev_alarm_cont);
    lv_obj_add_event_cb(co_dev_alarm_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(co_dev_alarm_cont, &style, 0);
    lv_obj_set_style_radius(co_dev_alarm_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(co_dev_alarm_cont, true, 0);
    lv_obj_set_scroll_dir(co_dev_alarm_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(co_dev_alarm_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(co_dev_alarm_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(co_dev_alarm_cont, 633, 0);
    lv_obj_set_style_bg_color(co_dev_alarm_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(co_dev_alarm_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(co_dev_alarm_cont, 0, 0);
    lv_obj_set_style_pad_row(co_dev_alarm_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(co_dev_alarm_menu_items) / sizeof(CoDevAlarmMenuItem); i++) {
        create_co_dev_alarm_menu_item(co_dev_alarm_cont, &co_dev_alarm_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    co_dev_alarm_mask = radial();
    if (is_obj_valid(co_dev_alarm_mask)) {
        lv_obj_set_pos(co_dev_alarm_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_CO_ALARMS_DEVIATION);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_ALARMS_DEVIATION);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(co_dev_alarm_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    co_dev_alarm_highlight_box(co_dev_alarm_cont, menu_state->cursor_index);
    arc_menu_update_slide(co_dev_alarm_cont);
    
    co_dev_alarm_menu_initialized = true;
    co_dev_alarm_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню аварийного отклонения успешно инициализировано");
}


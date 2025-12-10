#include "GVS_ext_alarm_menu.h"
#include "GVS_ext_alarm_params.h"
#include "gvs_ext_alarm_params_limits.h"
#include "encoder/encoder.h"
#include "encoder/encoder_manager.h"
#include "my_widgets/w_rad_mask.h"
#include "screen_logic/arc_menu.h"
#include "screen_logic/menu_config.h"
#include "screen_logic/screen_navigation.h"
#include "screen_logic/screen_container_manager.h"
#include "screen_logic/access_control.h"
#include "dialog_screen/screen_YES_NO/yes_no_screen.h"
#include "GVS_alarms_menu.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "GVS_EXT_ALARM_MENU";

static void update_param_display(int param_index);
static const char* get_ext_alarm_rtype_string(gvs_ext_alarm_rtype_t value);

typedef struct {
    const char *label_text;
    const void *img_src;
    int param_index;
} GvsExtAlarmMenuItem;

static const GvsExtAlarmMenuItem gvs_ext_alarm_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Внеш.авария Н1", NULL, 0},              // GVS_N1_EnExtAlarm (enum)
    {"Внеш.авария Н2", NULL, 1},              // GVS_N2_EnExtAlarm (enum)
    {"Задержка, с", NULL, 2},                 // GVS_N_ExtAlarmDelay (int)
    {"Сброс", NULL, 3},                        // GVS_N_ExtAlarmRType (enum)
};

lv_obj_t *gvs_ext_alarm_cont = NULL;
static bool gvs_ext_alarm_menu_initialized = false;
static bool gvs_ext_alarm_menu_creation_in_progress = false;
static lv_obj_t *gvs_ext_alarm_mask = NULL;

static lv_obj_t *value_labels[4] = {NULL};

static bool edit_mode = false;
static int editing_param_index = -1;
static int editing_int_value = 0;

static gvs_ext_alarm_en_t temp_GVS_N1_EnExtAlarm = GVS_EXT_ALARM_NO;
static gvs_ext_alarm_en_t temp_GVS_N2_EnExtAlarm = GVS_EXT_ALARM_NO;
static int temp_GVS_N_ExtAlarmDelay = 0;
static gvs_ext_alarm_rtype_t temp_GVS_N_ExtAlarmRType = GVS_EXT_ALARM_RTYPE_MANUAL_3;

static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Преобразует enum сброса в строку
 */
static const char* get_ext_alarm_rtype_string(gvs_ext_alarm_rtype_t value) {
    switch(value) {
        case GVS_EXT_ALARM_RTYPE_AUTO: return "АВТО";
        case GVS_EXT_ALARM_RTYPE_MANUAL: return "РУЧН";
        case GVS_EXT_ALARM_RTYPE_MANUAL_1: return "РУЧН-1";
        case GVS_EXT_ALARM_RTYPE_MANUAL_2: return "РУЧН-2";
        case GVS_EXT_ALARM_RTYPE_MANUAL_3: return "РУЧН-3";
        case GVS_EXT_ALARM_RTYPE_MANUAL_4: return "РУЧН-4";
        case GVS_EXT_ALARM_RTYPE_MANUAL_5: return "РУЧН-5";
        case GVS_EXT_ALARM_RTYPE_MANUAL_6: return "РУЧН-6";
        case GVS_EXT_ALARM_RTYPE_MANUAL_7: return "РУЧН-7";
        case GVS_EXT_ALARM_RTYPE_MANUAL_8: return "РУЧН-8";
        case GVS_EXT_ALARM_RTYPE_MANUAL_9: return "РУЧН-9";
        case GVS_EXT_ALARM_RTYPE_MANUAL_10: return "РУЧН-10";
        default: return "???";
    }
}

static void gvs_ext_alarm_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
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
                    if (!(edit_mode && editing_param_index == gvs_ext_alarm_menu_items[i].param_index)) {
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
            if (!(edit_mode && editing_param_index == gvs_ext_alarm_menu_items[i].param_index)) {
                lv_obj_set_style_bg_color(child, lv_color_hex(0xFFCC00), LV_PART_MAIN);
            }
        } else {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x2B3639), LV_PART_MAIN);
        }
    }
}

static void update_param_display(int param_index) {
    if (param_index < 0 || param_index >= 4) return;
    if (!is_obj_valid(value_labels[param_index])) return;
    
    char value_str[32];
    
    if (edit_mode && editing_param_index == param_index) {
        // В режиме редактирования
        if (param_index == 0 || param_index == 1) {
            // Внеш.авария Н1/Н2 - enum НЕТ/ДА
            snprintf(value_str, sizeof(value_str), "%s", 
                     editing_int_value == 0 ? "НЕТ" : "ДА");
        } else if (param_index == 2) {
            // Задержка - int
            snprintf(value_str, sizeof(value_str), "%d", editing_int_value);
        } else if (param_index == 3) {
            // Сброс - enum АВТО/РУЧН/РУЧН-1/.../РУЧН-10
            snprintf(value_str, sizeof(value_str), "%s", 
                     get_ext_alarm_rtype_string((gvs_ext_alarm_rtype_t)editing_int_value));
        }
    } else {
        // Обычный режим
        if (param_index == 0) {
            snprintf(value_str, sizeof(value_str), "%s", 
                     GVS_N1_EnExtAlarm == GVS_EXT_ALARM_NO ? "НЕТ" : "ДА");
        } else if (param_index == 1) {
            snprintf(value_str, sizeof(value_str), "%s", 
                     GVS_N2_EnExtAlarm == GVS_EXT_ALARM_NO ? "НЕТ" : "ДА");
        } else if (param_index == 2) {
            snprintf(value_str, sizeof(value_str), "%d", GVS_N_ExtAlarmDelay);
        } else if (param_index == 3) {
            snprintf(value_str, sizeof(value_str), "%s", get_ext_alarm_rtype_string(GVS_N_ExtAlarmRType));
        }
    }
    
    char full_str[40];
    if (param_index == 2) {
        snprintf(full_str, sizeof(full_str), "%s с", value_str);
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
        case 0: GVS_N1_EnExtAlarm = (gvs_ext_alarm_en_t)editing_int_value; break;
        case 1: GVS_N2_EnExtAlarm = (gvs_ext_alarm_en_t)editing_int_value; break;
        case 2: GVS_N_ExtAlarmDelay = editing_int_value; break;
        case 3: GVS_N_ExtAlarmRType = (gvs_ext_alarm_rtype_t)editing_int_value; break;
    }
    
    // НЕ сохраняем параметры в NVS (как в ГВС)
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_ALARMS_EXTERNAL);
    if (menu_state) {
        gvs_ext_alarm_highlight_box(gvs_ext_alarm_cont, menu_state->cursor_index);
    }
}

static void cancel_param_changes(void) {
    ESP_LOGI(TAG, "Canceling parameter changes");
    
    int saved_index = editing_param_index;
    
    switch(editing_param_index) {
        case 0: editing_int_value = (int)temp_GVS_N1_EnExtAlarm; break;
        case 1: editing_int_value = (int)temp_GVS_N2_EnExtAlarm; break;
        case 2: editing_int_value = temp_GVS_N_ExtAlarmDelay; break;
        case 3: editing_int_value = (int)temp_GVS_N_ExtAlarmRType; break;
    }
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_ALARMS_EXTERNAL);
    if (menu_state) {
        gvs_ext_alarm_highlight_box(gvs_ext_alarm_cont, menu_state->cursor_index);
    }
}

static void enter_edit_mode(int param_index) {
    if (param_index < 0 || param_index >= 4) return;
    
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
            temp_GVS_N1_EnExtAlarm = GVS_N1_EnExtAlarm;
            editing_int_value = (int)GVS_N1_EnExtAlarm;
            break;
        case 1:
            temp_GVS_N2_EnExtAlarm = GVS_N2_EnExtAlarm;
            editing_int_value = (int)GVS_N2_EnExtAlarm;
            break;
        case 2:
            temp_GVS_N_ExtAlarmDelay = GVS_N_ExtAlarmDelay;
            editing_int_value = GVS_N_ExtAlarmDelay;
            break;
        case 3:
            temp_GVS_N_ExtAlarmRType = GVS_N_ExtAlarmRType;
            editing_int_value = (int)GVS_N_ExtAlarmRType;
            break;
    }
    
    update_param_display(param_index);
}

static void exit_edit_mode_with_confirmation(void) {
    if (!edit_mode || editing_param_index < 0) return;
    
    bool value_changed = false;
    
    switch(editing_param_index) {
        case 0: value_changed = (editing_int_value != (int)temp_GVS_N1_EnExtAlarm); break;
        case 1: value_changed = (editing_int_value != (int)temp_GVS_N2_EnExtAlarm); break;
        case 2: value_changed = (editing_int_value != temp_GVS_N_ExtAlarmDelay); break;
        case 3: value_changed = (editing_int_value != (int)temp_GVS_N_ExtAlarmRType); break;
    }
    
    if (value_changed) {
        create_yes_no_screen_with_callbacks(save_param_changes, cancel_param_changes);
    } else {
        cancel_param_changes();
    }
}

static void create_gvs_ext_alarm_menu_item(lv_obj_t *cont, const GvsExtAlarmMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_gvs_ext_alarm_menu_item");
        return;
    }
    
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_gvs_ext_alarm_menu_item");
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

void gvs_ext_alarm_menu_show(void) {
    ESP_LOGI(TAG, "Showing GVS external alarm menu");
    if (is_obj_valid(gvs_ext_alarm_cont)) {
        lv_obj_clear_flag(gvs_ext_alarm_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(gvs_ext_alarm_mask)) {
        gvs_ext_alarm_mask = radial();
        if (is_obj_valid(gvs_ext_alarm_mask)) {
            lv_obj_set_pos(gvs_ext_alarm_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(gvs_ext_alarm_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

void gvs_ext_alarm_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding GVS external alarm menu");
    if (is_obj_valid(gvs_ext_alarm_cont)) {
        lv_obj_add_flag(gvs_ext_alarm_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(gvs_ext_alarm_mask)) {
        lv_obj_del(gvs_ext_alarm_mask);
        gvs_ext_alarm_mask = NULL;
    }
}

void gvs_ext_alarm_menu_encoder_event_cb(uint8_t e) {
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(gvs_ext_alarm_cont)) {
        ESP_LOGE(TAG, "Контейнер меню внешней аварии ГВС не инициализирован");
        return;
    }
    
    if (edit_mode && editing_param_index >= 0) {
        if (e & ENC_LEFT) {
            int step = gvs_ext_alarm_param_limits_int[editing_param_index].step;
            editing_int_value -= step;
            if (editing_int_value < gvs_ext_alarm_param_limits_int[editing_param_index].min) {
                // Для enum параметров делаем циклическое переключение
                if (editing_param_index == 0 || editing_param_index == 1 || editing_param_index == 3) {
                    editing_int_value = gvs_ext_alarm_param_limits_int[editing_param_index].max;
                } else {
                    editing_int_value = gvs_ext_alarm_param_limits_int[editing_param_index].min;
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_RIGHT) {
            int step = gvs_ext_alarm_param_limits_int[editing_param_index].step;
            editing_int_value += step;
            if (editing_int_value > gvs_ext_alarm_param_limits_int[editing_param_index].max) {
                // Для enum параметров делаем циклическое переключение
                if (editing_param_index == 0 || editing_param_index == 1 || editing_param_index == 3) {
                    editing_int_value = gvs_ext_alarm_param_limits_int[editing_param_index].min;
                } else {
                    editing_int_value = gvs_ext_alarm_param_limits_int[editing_param_index].max;
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_CLICK) {
            exit_edit_mode_with_confirmation();
        }
        return;
    }
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_ALARMS_EXTERNAL);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, gvs_ext_alarm_cont, menu_state, MENU_TYPE_GVS_ALARMS_EXTERNAL);
    
    if (prev_cursor != menu_state->cursor_index) {
        gvs_ext_alarm_highlight_box(gvs_ext_alarm_cont, menu_state->cursor_index);
    }
    
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            ESP_LOGI(TAG, "Returning to GVS alarms menu from external alarm menu");
            gvs_ext_alarm_menu_hide();
            gvs_alarms_menu_show();
            encoder_manager_register_callback(gvs_alarms_menu_encoder_event_cb);
        } else {
            int param_index = gvs_ext_alarm_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_edit_mode(param_index);
            }
        }
    }
}

void gvs_ext_alarm_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up GVS external alarm menu");
    
    gvs_ext_alarm_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    for (int i = 0; i < 4; i++) {
        value_labels[i] = NULL;
    }
    
    if (is_obj_valid(gvs_ext_alarm_mask)) {
        lv_obj_del(gvs_ext_alarm_mask);
        gvs_ext_alarm_mask = NULL;
    }
    
    if (is_obj_valid(gvs_ext_alarm_cont)) {
        lv_obj_del(gvs_ext_alarm_cont);
        gvs_ext_alarm_cont = NULL;
    }
    
    gvs_ext_alarm_menu_initialized = false;
}

void GVS_Ext_Alarm_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню внешней аварии ГВС");
    
    if (gvs_ext_alarm_menu_creation_in_progress) {
        ESP_LOGW(TAG, "GVS external alarm menu creation already in progress, skipping");
        return;
    }
    
    gvs_ext_alarm_menu_creation_in_progress = true;
    
    if (gvs_ext_alarm_menu_initialized && is_obj_valid(gvs_ext_alarm_cont)) {
        ESP_LOGI(TAG, "GVS external alarm menu already initialized, showing it");
        gvs_ext_alarm_menu_show();
        gvs_ext_alarm_menu_creation_in_progress = false;
        return;
    }
    
    gvs_ext_alarm_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    gvs_ext_alarm_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(gvs_ext_alarm_cont)) {
        ESP_LOGE(TAG, "Failed to create GVS external alarm menu container");
        gvs_ext_alarm_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(gvs_ext_alarm_cont, 1200, 1200);
    lv_obj_center(gvs_ext_alarm_cont);
    lv_obj_add_event_cb(gvs_ext_alarm_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(gvs_ext_alarm_cont, &style, 0);
    lv_obj_set_style_radius(gvs_ext_alarm_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(gvs_ext_alarm_cont, true, 0);
    lv_obj_set_scroll_dir(gvs_ext_alarm_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(gvs_ext_alarm_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(gvs_ext_alarm_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(gvs_ext_alarm_cont, 633, 0);
    lv_obj_set_style_bg_color(gvs_ext_alarm_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(gvs_ext_alarm_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(gvs_ext_alarm_cont, 0, 0);
    lv_obj_set_style_pad_row(gvs_ext_alarm_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(gvs_ext_alarm_menu_items) / sizeof(GvsExtAlarmMenuItem); i++) {
        create_gvs_ext_alarm_menu_item(gvs_ext_alarm_cont, &gvs_ext_alarm_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    gvs_ext_alarm_mask = radial();
    if (is_obj_valid(gvs_ext_alarm_mask)) {
        lv_obj_set_pos(gvs_ext_alarm_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_GVS_ALARMS_EXTERNAL);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_ALARMS_EXTERNAL);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(gvs_ext_alarm_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    gvs_ext_alarm_highlight_box(gvs_ext_alarm_cont, menu_state->cursor_index);
    arc_menu_update_slide(gvs_ext_alarm_cont);
    
    gvs_ext_alarm_menu_initialized = true;
    gvs_ext_alarm_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню внешней аварии ГВС успешно инициализировано");
}


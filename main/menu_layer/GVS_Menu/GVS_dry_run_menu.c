#include "GVS_dry_run_menu.h"
#include "GVS_dry_run_params.h"
#include "gvs_dry_run_params_limits.h"
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

static const char *TAG = "GVS_DRY_RUN_MENU";

// Forward declarations
static void update_param_display(int param_index);
static const char* get_enalarm_string(gvs_ps_enalarm_t value);
static const char* get_alarm_rtype_string(gvs_ps_alarm_rtype_t value);

// Структура элемента меню сухого хода ГВС
typedef struct {
    const char *label_text;
    const void *img_src;
    int param_index;
} GvsDryRunMenuItem;

// Элементы меню сухого хода ГВС
static const GvsDryRunMenuItem gvs_dry_run_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Активация", NULL, 0},                    // GVS_PS_EnAlarm (enum)
    {"Задержка, с", NULL, 1},                   // GVS_PS_AlarmDelay (int)
    {"Сброс", NULL, 2},                         // GVS_PS_AlarmRType (enum)
};

// Локальные переменные
lv_obj_t *gvs_dry_run_cont = NULL;
static bool gvs_dry_run_menu_initialized = false;
static bool gvs_dry_run_menu_creation_in_progress = false;
static lv_obj_t *gvs_dry_run_mask = NULL;

// Массив указателей на label для значений параметров (3 параметра)
static lv_obj_t *value_labels[3] = {NULL};

// Состояние редактирования
static bool edit_mode = false;
static int editing_param_index = -1;
static int editing_int_value = 0;

// Временные значения для отмены изменений
static gvs_ps_enalarm_t temp_GVS_PS_EnAlarm = GVS_PS_ENALARM_NO;
static int temp_GVS_PS_AlarmDelay = 0;
static gvs_ps_alarm_rtype_t temp_GVS_PS_AlarmRType = GVS_PS_ALARM_RTYPE_MANUAL_3;

static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Преобразует enum активации в строку
 */
static const char* get_enalarm_string(gvs_ps_enalarm_t value) {
    return (value == GVS_PS_ENALARM_NO) ? "НЕТ" : "ДА";
}

/**
 * @brief Преобразует enum сброса в строку
 */
static const char* get_alarm_rtype_string(gvs_ps_alarm_rtype_t value) {
    switch(value) {
        case GVS_PS_ALARM_RTYPE_AUTO: return "АВТО";
        case GVS_PS_ALARM_RTYPE_MANUAL: return "РУЧН";
        case GVS_PS_ALARM_RTYPE_MANUAL_1: return "РУЧН-1";
        case GVS_PS_ALARM_RTYPE_MANUAL_2: return "РУЧН-2";
        case GVS_PS_ALARM_RTYPE_MANUAL_3: return "РУЧН-3";
        case GVS_PS_ALARM_RTYPE_MANUAL_4: return "РУЧН-4";
        case GVS_PS_ALARM_RTYPE_MANUAL_5: return "РУЧН-5";
        case GVS_PS_ALARM_RTYPE_MANUAL_6: return "РУЧН-6";
        case GVS_PS_ALARM_RTYPE_MANUAL_7: return "РУЧН-7";
        case GVS_PS_ALARM_RTYPE_MANUAL_8: return "РУЧН-8";
        case GVS_PS_ALARM_RTYPE_MANUAL_9: return "РУЧН-9";
        case GVS_PS_ALARM_RTYPE_MANUAL_10: return "РУЧН-10";
        default: return "???";
    }
}

static void gvs_dry_run_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
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
                    if (!(edit_mode && editing_param_index == gvs_dry_run_menu_items[i].param_index)) {
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
            if (!(edit_mode && editing_param_index == gvs_dry_run_menu_items[i].param_index)) {
                lv_obj_set_style_bg_color(child, lv_color_hex(0xFFCC00), LV_PART_MAIN);
            }
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
                     get_enalarm_string((gvs_ps_enalarm_t)editing_int_value));
        } else if (param_index == 1) {
            // Задержка - int
            snprintf(value_str, sizeof(value_str), "%d", editing_int_value);
        } else if (param_index == 2) {
            // Сброс - enum АВТО/РУЧН/РУЧН-1/.../РУЧН-10
            snprintf(value_str, sizeof(value_str), "%s", 
                     get_alarm_rtype_string((gvs_ps_alarm_rtype_t)editing_int_value));
        }
    } else {
        // Обычный режим
        if (param_index == 0) {
            snprintf(value_str, sizeof(value_str), "%s", get_enalarm_string(GVS_PS_EnAlarm));
        } else if (param_index == 1) {
            snprintf(value_str, sizeof(value_str), "%d", GVS_PS_AlarmDelay);
        } else if (param_index == 2) {
            snprintf(value_str, sizeof(value_str), "%s", get_alarm_rtype_string(GVS_PS_AlarmRType));
        }
    }
    
    char full_str[40];
    if (param_index == 1) {
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
        case 0: GVS_PS_EnAlarm = (gvs_ps_enalarm_t)editing_int_value; break;
        case 1: GVS_PS_AlarmDelay = editing_int_value; break;
        case 2: GVS_PS_AlarmRType = (gvs_ps_alarm_rtype_t)editing_int_value; break;
    }
    
    // НЕ сохраняем параметры в NVS (как в ГВС)
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_ALARMS_DRY_RUN);
    if (menu_state) {
        gvs_dry_run_highlight_box(gvs_dry_run_cont, menu_state->cursor_index);
    }
}

static void cancel_param_changes(void) {
    ESP_LOGI(TAG, "Canceling parameter changes");
    
    int saved_index = editing_param_index;
    
    switch(editing_param_index) {
        case 0: editing_int_value = (int)temp_GVS_PS_EnAlarm; break;
        case 1: editing_int_value = temp_GVS_PS_AlarmDelay; break;
        case 2: editing_int_value = (int)temp_GVS_PS_AlarmRType; break;
    }
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_ALARMS_DRY_RUN);
    if (menu_state) {
        gvs_dry_run_highlight_box(gvs_dry_run_cont, menu_state->cursor_index);
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
            temp_GVS_PS_EnAlarm = GVS_PS_EnAlarm;
            editing_int_value = (int)GVS_PS_EnAlarm;
            break;
        case 1:
            temp_GVS_PS_AlarmDelay = GVS_PS_AlarmDelay;
            editing_int_value = GVS_PS_AlarmDelay;
            break;
        case 2:
            temp_GVS_PS_AlarmRType = GVS_PS_AlarmRType;
            editing_int_value = (int)GVS_PS_AlarmRType;
            break;
    }
    
    update_param_display(param_index);
}

static void exit_edit_mode_with_confirmation(void) {
    if (!edit_mode || editing_param_index < 0) return;
    
    bool value_changed = false;
    
    switch(editing_param_index) {
        case 0: value_changed = (editing_int_value != (int)temp_GVS_PS_EnAlarm); break;
        case 1: value_changed = (editing_int_value != temp_GVS_PS_AlarmDelay); break;
        case 2: value_changed = (editing_int_value != (int)temp_GVS_PS_AlarmRType); break;
    }
    
    if (value_changed) {
        create_yes_no_screen_with_callbacks(save_param_changes, cancel_param_changes);
    } else {
        cancel_param_changes();
    }
}

static void create_gvs_dry_run_menu_item(lv_obj_t *cont, const GvsDryRunMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_gvs_dry_run_menu_item");
        return;
    }
    
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_gvs_dry_run_menu_item");
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

void gvs_dry_run_menu_show(void) {
    ESP_LOGI(TAG, "Showing GVS dry run menu");
    if (is_obj_valid(gvs_dry_run_cont)) {
        lv_obj_clear_flag(gvs_dry_run_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(gvs_dry_run_mask)) {
        gvs_dry_run_mask = radial();
        if (is_obj_valid(gvs_dry_run_mask)) {
            lv_obj_set_pos(gvs_dry_run_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(gvs_dry_run_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

void gvs_dry_run_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding GVS dry run menu");
    if (is_obj_valid(gvs_dry_run_cont)) {
        lv_obj_add_flag(gvs_dry_run_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(gvs_dry_run_mask)) {
        lv_obj_del(gvs_dry_run_mask);
        gvs_dry_run_mask = NULL;
    }
}

void gvs_dry_run_menu_encoder_event_cb(uint8_t e) {
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(gvs_dry_run_cont)) {
        ESP_LOGE(TAG, "Контейнер меню сухого хода ГВС не инициализирован");
        return;
    }
    
    if (edit_mode && editing_param_index >= 0) {
        if (e & ENC_LEFT) {
            int step = gvs_dry_run_param_limits_int[editing_param_index].step;
            editing_int_value -= step;
            if (editing_int_value < gvs_dry_run_param_limits_int[editing_param_index].min) {
                // Для enum параметров делаем циклическое переключение
                if (editing_param_index == 0 || editing_param_index == 2) {
                    editing_int_value = gvs_dry_run_param_limits_int[editing_param_index].max;
                } else {
                    editing_int_value = gvs_dry_run_param_limits_int[editing_param_index].min;
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_RIGHT) {
            int step = gvs_dry_run_param_limits_int[editing_param_index].step;
            editing_int_value += step;
            if (editing_int_value > gvs_dry_run_param_limits_int[editing_param_index].max) {
                // Для enum параметров делаем циклическое переключение
                if (editing_param_index == 0 || editing_param_index == 2) {
                    editing_int_value = gvs_dry_run_param_limits_int[editing_param_index].min;
                } else {
                    editing_int_value = gvs_dry_run_param_limits_int[editing_param_index].max;
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_CLICK) {
            exit_edit_mode_with_confirmation();
        }
        return;
    }
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_ALARMS_DRY_RUN);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, gvs_dry_run_cont, menu_state, MENU_TYPE_GVS_ALARMS_DRY_RUN);
    
    if (prev_cursor != menu_state->cursor_index) {
        gvs_dry_run_highlight_box(gvs_dry_run_cont, menu_state->cursor_index);
    }
    
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            ESP_LOGI(TAG, "Returning to GVS alarms menu from dry run menu");
            gvs_dry_run_menu_hide();
            gvs_alarms_menu_show();
            encoder_manager_register_callback(gvs_alarms_menu_encoder_event_cb);
        } else {
            int param_index = gvs_dry_run_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_edit_mode(param_index);
            }
        }
    }
}

void gvs_dry_run_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up GVS dry run menu");
    
    gvs_dry_run_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    for (int i = 0; i < 3; i++) {
        value_labels[i] = NULL;
    }
    
    if (is_obj_valid(gvs_dry_run_mask)) {
        lv_obj_del(gvs_dry_run_mask);
        gvs_dry_run_mask = NULL;
    }
    
    if (is_obj_valid(gvs_dry_run_cont)) {
        lv_obj_del(gvs_dry_run_cont);
        gvs_dry_run_cont = NULL;
    }
    
    gvs_dry_run_menu_initialized = false;
}

void GVS_Dry_Run_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню сухого хода ГВС");
    
    if (gvs_dry_run_menu_creation_in_progress) {
        ESP_LOGW(TAG, "GVS dry run menu creation already in progress, skipping");
        return;
    }
    
    gvs_dry_run_menu_creation_in_progress = true;
    
    if (gvs_dry_run_menu_initialized && is_obj_valid(gvs_dry_run_cont)) {
        ESP_LOGI(TAG, "GVS dry run menu already initialized, showing it");
        gvs_dry_run_menu_show();
        gvs_dry_run_menu_creation_in_progress = false;
        return;
    }
    
    gvs_dry_run_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    gvs_dry_run_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(gvs_dry_run_cont)) {
        ESP_LOGE(TAG, "Failed to create GVS dry run menu container");
        gvs_dry_run_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(gvs_dry_run_cont, 1200, 1200);
    lv_obj_center(gvs_dry_run_cont);
    lv_obj_add_event_cb(gvs_dry_run_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(gvs_dry_run_cont, &style, 0);
    lv_obj_set_style_radius(gvs_dry_run_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(gvs_dry_run_cont, true, 0);
    lv_obj_set_scroll_dir(gvs_dry_run_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(gvs_dry_run_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(gvs_dry_run_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(gvs_dry_run_cont, 633, 0);
    lv_obj_set_style_bg_color(gvs_dry_run_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(gvs_dry_run_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(gvs_dry_run_cont, 0, 0);
    lv_obj_set_style_pad_row(gvs_dry_run_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(gvs_dry_run_menu_items) / sizeof(GvsDryRunMenuItem); i++) {
        create_gvs_dry_run_menu_item(gvs_dry_run_cont, &gvs_dry_run_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    gvs_dry_run_mask = radial();
    if (is_obj_valid(gvs_dry_run_mask)) {
        lv_obj_set_pos(gvs_dry_run_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_GVS_ALARMS_DRY_RUN);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_ALARMS_DRY_RUN);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(gvs_dry_run_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    gvs_dry_run_highlight_box(gvs_dry_run_cont, menu_state->cursor_index);
    arc_menu_update_slide(gvs_dry_run_cont);
    
    gvs_dry_run_menu_initialized = true;
    gvs_dry_run_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню сухого хода ГВС успешно инициализировано");
}


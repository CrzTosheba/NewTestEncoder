#include "GVS_schedule_menu.h"
#include "GVS_schedule_params.h"
#include "gvs_params_limits.h"
#include "encoder/encoder.h"
#include "encoder/encoder_manager.h"
#include "my_widgets/w_rad_mask.h"
#include "screen_logic/arc_menu.h"
#include "screen_logic/menu_config.h"
#include "screen_logic/screen_navigation.h"
#include "screen_logic/screen_container_manager.h"
#include "dialog_screen/screen_YES_NO/yes_no_screen.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "GVS_SCHEDULE_MENU";

// Forward declarations
static void update_schedule_day_param_display(int param_index);
static void update_day_param_display(int param_index, int day_index);

// Структура элемента главного меню расписания (дни недели)
typedef struct {
    const char *label_text;     // Текст элемента
    const void *img_src;        // Иконка (только для "Назад")
    int day_index;              // Индекс дня (-1 для "Назад", 0-6 для дней Пн-Вс)
} GvsScheduleMenuItem;

// Элементы главного меню расписания
static const GvsScheduleMenuItem gvs_schedule_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Понедельник", NULL, 0},
    {"Вторник", NULL, 1},
    {"Среда", NULL, 2},
    {"Четверг", NULL, 3},
    {"Пятница", NULL, 4},
    {"Суббота", NULL, 5},
    {"Воскресенье", NULL, 6},
};

// Структура элемента подменю дня недели
typedef struct {
    const char *label_text;     // Текст элемента слева
    const void *img_src;        // Иконка (только для "Назад")
    int param_index;            // Индекс параметра (-1 для "Назад")
} GvsScheduleDayMenuItem;

// Элементы подменю дня недели
// param_index: 0=From1, 1=To1, 2=From2, 3=To2
static const GvsScheduleDayMenuItem gvs_schedule_day_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Комф. период 1. C", NULL, 0},        // From1 (часы + минуты)
    {"Комф. период 1. До", NULL, 1},       // To1 (часы + минуты)
    {"Комф. период 2. C", NULL, 2},        // From2 (часы + минуты)
    {"Комф. период 2. До", NULL, 3},       // To2 (часы + минуты)
};

// Локальные переменные для главного меню расписания
lv_obj_t *gvs_schedule_cont = NULL;
static bool gvs_schedule_menu_initialized = false;
static bool gvs_schedule_menu_creation_in_progress = false;
static lv_obj_t *gvs_schedule_mask = NULL;

// Локальные переменные для подменю дня недели
lv_obj_t *gvs_schedule_day_cont = NULL;
static bool gvs_schedule_day_menu_initialized = false;
static bool gvs_schedule_day_menu_creation_in_progress = false;
static lv_obj_t *gvs_schedule_day_mask = NULL;
int current_gvs_schedule_day = 0; // Текущий выбранный день (0-6: Пн-Вс)

// Массив указателей на label для значений параметров дня (4 периода)
static lv_obj_t *day_value_labels[4] = {NULL};

// Состояние редактирования для подменю дня
static bool edit_mode = false;
static int editing_param_index = -1;
static bool editing_hours = true;  // true = редактируем часы, false = редактируем минуты
static int editing_hours_value = 0;
static int editing_minutes_value = 0;

// Временные значения для отмены изменений (для текущего дня)
static int temp_hours[4] = {0, 0, 0, 0};  // Для периодов 0-3
static int temp_minutes[4] = {0, 0, 0, 0};  // Для периодов 0-3

/**
 * @brief Проверяет, является ли объект валидным
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Получает указатель на часы для периода
 * @param day_index Индекс дня (0-6)
 * @param period_index Индекс периода (0=From1, 1=To1, 2=From2, 3=To2)
 */
static int* get_day_hours_ptr(int day_index, int period_index) {
    // Массивы указателей на часы для каждого дня
    static int* mon_hours[4] = {&GVS_MonHoursFrom1, &GVS_MonHoursTo1, &GVS_MonHoursFrom2, &GVS_MonHoursTo2};
    static int* tue_hours[4] = {&GVS_TueHoursFrom1, &GVS_TueHoursTo1, &GVS_TueHoursFrom2, &GVS_TueHoursTo2};
    static int* wed_hours[4] = {&GVS_WedHoursFrom1, &GVS_WedHoursTo1, &GVS_WedHoursFrom2, &GVS_WedHoursTo2};
    static int* thu_hours[4] = {&GVS_ThuHoursFrom1, &GVS_ThuHoursTo1, &GVS_ThuHoursFrom2, &GVS_ThuHoursTo2};
    static int* fri_hours[4] = {&GVS_FriHoursFrom1, &GVS_FriHoursTo1, &GVS_FriHoursFrom2, &GVS_FriHoursTo2};
    static int* sat_hours[4] = {&GVS_SatHoursFrom1, &GVS_SatHoursTo1, &GVS_SatHoursFrom2, &GVS_SatHoursTo2};
    static int* sun_hours[4] = {&GVS_SunHoursFrom1, &GVS_SunHoursTo1, &GVS_SunHoursFrom2, &GVS_SunHoursTo2};
    
    switch(day_index) {
        case 0: return mon_hours[period_index];
        case 1: return tue_hours[period_index];
        case 2: return wed_hours[period_index];
        case 3: return thu_hours[period_index];
        case 4: return fri_hours[period_index];
        case 5: return sat_hours[period_index];
        case 6: return sun_hours[period_index];
        default: return NULL;
    }
}

/**
 * @brief Получает указатель на минуты для периода
 * @param day_index Индекс дня (0-6)
 * @param period_index Индекс периода (0=From1, 1=To1, 2=From2, 3=To2)
 */
static int* get_day_minutes_ptr(int day_index, int period_index) {
    // Массивы указателей на минуты для каждого дня
    static int* mon_minutes[4] = {&GVS_MonMinFrom1, &GVS_MonMinTo1, &GVS_MonMinFrom2, &GVS_MonMinTo2};
    static int* tue_minutes[4] = {&GVS_TueMinFrom1, &GVS_TueMinTo1, &GVS_TueMinFrom2, &GVS_TueMinTo2};
    static int* wed_minutes[4] = {&GVS_WedMinFrom1, &GVS_WedMinTo1, &GVS_WedMinFrom2, &GVS_WedMinTo2};
    static int* thu_minutes[4] = {&GVS_ThuMinFrom1, &GVS_ThuMinTo1, &GVS_ThuMinFrom2, &GVS_ThuMinTo2};
    static int* fri_minutes[4] = {&GVS_FriMinFrom1, &GVS_FriMinTo1, &GVS_FriMinFrom2, &GVS_FriMinTo2};
    static int* sat_minutes[4] = {&GVS_SatMinFrom1, &GVS_SatMinTo1, &GVS_SatMinFrom2, &GVS_SatMinTo2};
    static int* sun_minutes[4] = {&GVS_SunMinFrom1, &GVS_SunMinTo1, &GVS_SunMinFrom2, &GVS_SunMinTo2};
    
    switch(day_index) {
        case 0: return mon_minutes[period_index];
        case 1: return tue_minutes[period_index];
        case 2: return wed_minutes[period_index];
        case 3: return thu_minutes[period_index];
        case 4: return fri_minutes[period_index];
        case 5: return sat_minutes[period_index];
        case 6: return sun_minutes[period_index];
        default: return NULL;
    }
}

/**
 * @brief Форматирует время в формат ЧЧ:ММ
 */
static void format_time_value(char *buf, size_t buf_size, int hours, int minutes) {
    snprintf(buf, buf_size, "%02d:%02d", hours, minutes);
}

/**
 * @brief Обновляет отображение параметра дня
 * @param period_index Индекс периода (0=From1, 1=To1, 2=From2, 3=To2)
 */
static void update_day_param_display(int period_index, int day_index) {
    if (period_index < 0 || period_index >= 4 || !is_obj_valid(day_value_labels[period_index])) {
        return;
    }
    
    char value_str[20];
    int hours, minutes;
    
    int* hours_ptr = get_day_hours_ptr(day_index, period_index);
    int* minutes_ptr = get_day_minutes_ptr(day_index, period_index);
    
    if (edit_mode && editing_param_index == period_index) {
        // В режиме редактирования - всегда показываем текущие редактируемые значения
        hours = editing_hours_value;
        minutes = editing_minutes_value;
    } else {
        // Обычный режим - показываем значения из переменных
        hours = hours_ptr ? *hours_ptr : 0;
        minutes = minutes_ptr ? *minutes_ptr : 0;
    }
    
    format_time_value(value_str, sizeof(value_str), hours, minutes);
    lv_label_set_text(day_value_labels[period_index], value_str);
    
    // Обновляем цвет в режиме редактирования
    if (!is_obj_valid(day_value_labels[period_index])) return;
    
    lv_obj_set_style_text_font(day_value_labels[period_index], &Roboto_bold_24, LV_PART_MAIN);
    
    lv_obj_t *value_container = lv_obj_get_parent(day_value_labels[period_index]);
    if (!is_obj_valid(value_container)) return;
    
    if (edit_mode && editing_param_index == period_index) {
        lv_obj_set_style_bg_color(value_container, lv_color_hex(0xE9EBEB), LV_PART_MAIN);
        lv_obj_set_style_text_color(day_value_labels[period_index], lv_color_hex(0x101315), LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(value_container, lv_color_hex(0x2B3639), LV_PART_MAIN);
        lv_obj_set_style_text_color(day_value_labels[period_index], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    }
}

/**
 * @brief Обновляет отображение всех параметров дня
 */
static void update_schedule_day_param_display(int param_index) {
    if (param_index >= 0 && param_index < 4) {
        update_day_param_display(param_index, current_gvs_schedule_day);
    }
}

/**
 * @brief Подсветка выбранного элемента главного меню расписания
 */
static void gvs_schedule_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
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
                    lv_obj_set_style_text_color(grand_child, lv_color_hex(0x000000), LV_PART_MAIN);
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
            lv_obj_set_style_bg_color(child, lv_color_hex(0xFFCC00), LV_PART_MAIN);
        } else {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x2B3639), LV_PART_MAIN);
        }
    }
}

/**
 * @brief Создание элемента главного меню расписания
 */
static void create_gvs_schedule_menu_item(lv_obj_t *cont, const GvsScheduleMenuItem *item) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_gvs_schedule_menu_item");
        return;
    }
    
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_gvs_schedule_menu_item");
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
    
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);
    vTaskDelay(pdMS_TO_TICKS(1));
}

/**
 * @brief Показывает главное меню расписания
 */
void gvs_schedule_menu_show(void) {
    ESP_LOGI(TAG, "Showing GVS schedule menu");
    if (is_obj_valid(gvs_schedule_cont)) {
        lv_obj_clear_flag(gvs_schedule_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(gvs_schedule_mask)) {
        gvs_schedule_mask = radial();
        if (is_obj_valid(gvs_schedule_mask)) {
            lv_obj_set_pos(gvs_schedule_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(gvs_schedule_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает главное меню расписания
 */
void gvs_schedule_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding GVS schedule menu");
    if (is_obj_valid(gvs_schedule_cont)) {
        lv_obj_add_flag(gvs_schedule_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(gvs_schedule_mask)) {
        lv_obj_add_flag(gvs_schedule_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Обработчик событий энкодера для главного меню расписания
 */
void gvs_schedule_menu_encoder_event_cb(uint8_t e) {
    if (!is_obj_valid(gvs_schedule_cont)) {
        ESP_LOGE(TAG, "Контейнер меню расписания ГВС не инициализирован");
        return;
    }
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_SCHEDULE);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, gvs_schedule_cont, menu_state, MENU_TYPE_GVS_SCHEDULE);
    
    if (prev_cursor != menu_state->cursor_index) {
        gvs_schedule_highlight_box(gvs_schedule_cont, menu_state->cursor_index);
    }
    
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Назад - возвращаемся в меню ГВС
            ESP_LOGI(TAG, "Returning to GVS menu from schedule menu");
            gvs_schedule_menu_hide();
            extern void gvs_menu_show(void);
            gvs_menu_show();
            extern void gvs_menu_encoder_event_cb(uint8_t e);
            encoder_manager_register_callback(gvs_menu_encoder_event_cb);
        } else {
            // Открываем подменю дня недели
            int day_index = gvs_schedule_menu_items[menu_state->cursor_index].day_index;
            if (day_index >= 0 && day_index <= 6) {
                ESP_LOGI(TAG, "Opening GVS schedule day menu for day %d", day_index);
                current_gvs_schedule_day = day_index;
                gvs_schedule_menu_hide();
                GVS_Schedule_Day_Menu_List(day_index);
                gvs_schedule_day_menu_show();
                encoder_manager_register_callback(gvs_schedule_day_menu_encoder_event_cb);
            }
        }
    }
}

/**
 * @brief Очистка главного меню расписания
 */
void gvs_schedule_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up GVS schedule menu");
    
    gvs_schedule_menu_creation_in_progress = false;
    
    if (is_obj_valid(gvs_schedule_mask)) {
        lv_obj_del(gvs_schedule_mask);
        gvs_schedule_mask = NULL;
    }
    
    if (is_obj_valid(gvs_schedule_cont)) {
        lv_obj_del(gvs_schedule_cont);
        gvs_schedule_cont = NULL;
    }
    
    gvs_schedule_menu_initialized = false;
}

/**
 * @brief Инициализация главного меню расписания
 */
void GVS_Schedule_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация главного меню расписания ГВС");
    
    if (gvs_schedule_menu_creation_in_progress) {
        ESP_LOGW(TAG, "GVS schedule menu creation already in progress, skipping");
        return;
    }
    
    gvs_schedule_menu_creation_in_progress = true;
    
    if (gvs_schedule_menu_initialized && is_obj_valid(gvs_schedule_cont)) {
        ESP_LOGI(TAG, "GVS schedule menu already initialized, showing it");
        gvs_schedule_menu_show();
        gvs_schedule_menu_creation_in_progress = false;
        return;
    }
    
    gvs_schedule_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    gvs_schedule_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(gvs_schedule_cont)) {
        ESP_LOGE(TAG, "Failed to create GVS schedule menu container");
        gvs_schedule_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(gvs_schedule_cont, 1200, 1200);
    lv_obj_center(gvs_schedule_cont);
    lv_obj_add_event_cb(gvs_schedule_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(gvs_schedule_cont, &style, 0);
    lv_obj_set_style_radius(gvs_schedule_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(gvs_schedule_cont, true, 0);
    lv_obj_set_scroll_dir(gvs_schedule_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(gvs_schedule_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(gvs_schedule_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(gvs_schedule_cont, 633, 0);
    lv_obj_set_style_bg_color(gvs_schedule_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(gvs_schedule_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(gvs_schedule_cont, 0, 0);
    lv_obj_set_style_pad_row(gvs_schedule_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(gvs_schedule_menu_items) / sizeof(GvsScheduleMenuItem); i++) {
        create_gvs_schedule_menu_item(gvs_schedule_cont, &gvs_schedule_menu_items[i]);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    gvs_schedule_mask = radial();
    if (is_obj_valid(gvs_schedule_mask)) {
        lv_obj_set_pos(gvs_schedule_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_GVS_SCHEDULE);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_SCHEDULE);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(gvs_schedule_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    gvs_schedule_highlight_box(gvs_schedule_cont, menu_state->cursor_index);
    arc_menu_update_slide(gvs_schedule_cont);
    
    gvs_schedule_menu_initialized = true;
    gvs_schedule_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Главное меню расписания ГВС успешно инициализировано");
}

/**
 * @brief Подсветка выбранного элемента подменю дня
 */
static void gvs_schedule_day_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
    if (!is_obj_valid(cont)) return;
    
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!is_obj_valid(child)) continue;
        
        bool is_selected = (i == cursor_index);
        bool is_editing_this = (edit_mode && editing_param_index == gvs_schedule_day_menu_items[i].param_index);
        
        uint32_t grand_child_cnt = lv_obj_get_child_cnt(child);
        
        for (uint32_t j = 0; j < grand_child_cnt; j++) {
            lv_obj_t *grand_child = lv_obj_get_child(child, j);
            if (!is_obj_valid(grand_child)) continue;
            
            // Проверяем, является ли это контейнером значения параметра
            bool is_value_container = false;
            for (int k = 0; k < 4; k++) {
                if (day_value_labels[k] != NULL && lv_obj_get_parent(day_value_labels[k]) == grand_child) {
                    is_value_container = true;
                    break;
                }
            }
            
            if (lv_obj_check_type(grand_child, &lv_label_class)) {
                // Проверяем, является ли это label значения параметра
                bool is_value_label = false;
                for (int k = 0; k < 4; k++) {
                    if (day_value_labels[k] == grand_child) {
                        is_value_label = true;
                        break;
                    }
                }
                
                if (is_selected) {
                    if (is_value_label && is_editing_this) {
                        // В режиме редактирования не меняем цвет редактируемого значения
                        // (цвет устанавливается в update_day_param_display)
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
        
        if (is_selected) {
            // Всегда устанавливаем желтый фон для выбранной строки
            lv_obj_set_style_bg_color(child, lv_color_hex(0xFFCC00), LV_PART_MAIN);
        } else {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x2B3639), LV_PART_MAIN);
        }
    }
}

/**
 * @brief Создание элемента подменю дня
 */
static void create_gvs_schedule_day_menu_item(lv_obj_t *cont, const GvsScheduleDayMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_gvs_schedule_day_menu_item");
        return;
    }
    
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_gvs_schedule_day_menu_item");
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
    
    // Значение параметра справа (только для редактируемых параметров)
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
                day_value_labels[item->param_index] = value_label;
                lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_font(value_label, &Roboto_bold_24, 0);
                lv_obj_align(value_label, LV_ALIGN_BOTTOM_RIGHT, 0, -2);
                update_schedule_day_param_display(item->param_index);
            }
        }
    }
    
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);
    vTaskDelay(pdMS_TO_TICKS(1));
}

/**
 * @brief Сохраняет изменения параметра дня
 */
static void save_day_param_changes(void) {
    ESP_LOGI(TAG, "Saving day parameter changes for period %d", editing_param_index);
    
    int saved_index = editing_param_index;
    
    int* hours_ptr = get_day_hours_ptr(current_gvs_schedule_day, editing_param_index);
    int* minutes_ptr = get_day_minutes_ptr(current_gvs_schedule_day, editing_param_index);
    
    if (hours_ptr) {
        *hours_ptr = editing_hours_value;
    }
    if (minutes_ptr) {
        *minutes_ptr = editing_minutes_value;
    }
    
    // НЕ сохраняем параметры в NVS (как в ГВС)
    
    edit_mode = false;
    editing_param_index = -1;
    editing_hours = true;
    update_schedule_day_param_display(saved_index);
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_SCHEDULE_DAY);
    if (menu_state) {
        gvs_schedule_day_highlight_box(gvs_schedule_day_cont, menu_state->cursor_index);
    }
}

/**
 * @brief Отменяет изменения параметра дня
 */
static void cancel_day_param_changes(void) {
    ESP_LOGI(TAG, "Canceling day parameter changes");
    
    int saved_index = editing_param_index;
    
    // Восстанавливаем исходные значения в переменных дней из временных переменных
    if (saved_index >= 0 && saved_index < 4) {
        int* hours_ptr = get_day_hours_ptr(current_gvs_schedule_day, saved_index);
        int* minutes_ptr = get_day_minutes_ptr(current_gvs_schedule_day, saved_index);
        
        if (hours_ptr) {
            *hours_ptr = temp_hours[saved_index];
        }
        if (minutes_ptr) {
            *minutes_ptr = temp_minutes[saved_index];
        }
        
        editing_hours_value = temp_hours[saved_index];
        editing_minutes_value = temp_minutes[saved_index];
    }
    
    edit_mode = false;
    editing_param_index = -1;
    editing_hours = true;
    update_schedule_day_param_display(saved_index);
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_SCHEDULE_DAY);
    if (menu_state) {
        gvs_schedule_day_highlight_box(gvs_schedule_day_cont, menu_state->cursor_index);
    }
}

/**
 * @brief Входит в режим редактирования параметра дня
 * @param period_index Индекс периода (0=From1, 1=To1, 2=From2, 3=To2)
 */
static void enter_day_edit_mode(int period_index) {
    if (period_index < 0 || period_index >= 4) return;
    
    ESP_LOGI(TAG, "Entering edit mode for day period %d", period_index);
    
    edit_mode = true;
    editing_param_index = period_index;
    editing_hours = true;  // Начинаем с редактирования часов
    
    int* hours_ptr = get_day_hours_ptr(current_gvs_schedule_day, period_index);
    int* minutes_ptr = get_day_minutes_ptr(current_gvs_schedule_day, period_index);
    
    if (hours_ptr) {
        editing_hours_value = *hours_ptr;
        temp_hours[period_index] = *hours_ptr;
    }
    if (minutes_ptr) {
        editing_minutes_value = *minutes_ptr;
        temp_minutes[period_index] = *minutes_ptr;
    }
    
    update_schedule_day_param_display(period_index);
}

/**
 * @brief Переключает режим редактирования (часы/минуты) или выходит из режима редактирования
 */
static void toggle_day_edit_mode_or_exit(void) {
    if (!edit_mode || editing_param_index < 0) return;
    
    if (editing_hours) {
        // Переключаемся на редактирование минут
        // Сохраняем измененное значение часов обратно в переменную, чтобы оно не терялось
        int* hours_ptr = get_day_hours_ptr(current_gvs_schedule_day, editing_param_index);
        if (hours_ptr) {
            *hours_ptr = editing_hours_value;
        }
        
        editing_hours = false;
        ESP_LOGI(TAG, "Switching to minutes editing for period %d", editing_param_index);
        update_schedule_day_param_display(editing_param_index);
    } else {
        // Выходим из режима редактирования и проверяем изменения
        // Сохраняем измененное значение минут обратно в переменную
        int* minutes_ptr = get_day_minutes_ptr(current_gvs_schedule_day, editing_param_index);
        if (minutes_ptr) {
            *minutes_ptr = editing_minutes_value;
        }
        
        bool value_changed = false;
        
        if (editing_param_index >= 0 && editing_param_index < 4) {
            value_changed = (editing_hours_value != temp_hours[editing_param_index]) ||
                          (editing_minutes_value != temp_minutes[editing_param_index]);
        }
        
        if (value_changed) {
            create_yes_no_screen_with_callbacks(save_day_param_changes, cancel_day_param_changes);
        } else {
            cancel_day_param_changes();
        }
    }
}

/**
 * @brief Показывает подменю дня
 */
void gvs_schedule_day_menu_show(void) {
    ESP_LOGI(TAG, "Showing GVS schedule day menu");
    if (is_obj_valid(gvs_schedule_day_cont)) {
        lv_obj_clear_flag(gvs_schedule_day_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(gvs_schedule_day_mask)) {
        gvs_schedule_day_mask = radial();
        if (is_obj_valid(gvs_schedule_day_mask)) {
            lv_obj_set_pos(gvs_schedule_day_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(gvs_schedule_day_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает подменю дня
 */
void gvs_schedule_day_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding GVS schedule day menu");
    if (is_obj_valid(gvs_schedule_day_cont)) {
        lv_obj_add_flag(gvs_schedule_day_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(gvs_schedule_day_mask)) {
        lv_obj_add_flag(gvs_schedule_day_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Обработчик событий энкодера для подменю дня
 */
void gvs_schedule_day_menu_encoder_event_cb(uint8_t e) {
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(gvs_schedule_day_cont)) {
        ESP_LOGE(TAG, "Контейнер подменю дня ГВС не инициализирован");
        return;
    }
    
    // Если в режиме редактирования
    if (edit_mode && editing_param_index >= 0) {
        if (e & ENC_LEFT) {
            // Уменьшаем значение
            if (editing_hours) {
                // Редактируем часы
                int step = gvs_schedule_param_limits_int[0].step;  // Используем первый элемент для часов
                editing_hours_value -= step;
                if (editing_hours_value < gvs_schedule_param_limits_int[0].min) {
                    editing_hours_value = gvs_schedule_param_limits_int[0].min;
                }
            } else {
                // Редактируем минуты
                int step = gvs_schedule_param_limits_int[2].step;  // Используем третий элемент для минут
                editing_minutes_value -= step;
                if (editing_minutes_value < gvs_schedule_param_limits_int[2].min) {
                    editing_minutes_value = gvs_schedule_param_limits_int[2].min;
                }
            }
            update_schedule_day_param_display(editing_param_index);
        } else if (e & ENC_RIGHT) {
            // Увеличиваем значение
            if (editing_hours) {
                // Редактируем часы
                int step = gvs_schedule_param_limits_int[0].step;
                editing_hours_value += step;
                if (editing_hours_value > gvs_schedule_param_limits_int[0].max) {
                    editing_hours_value = gvs_schedule_param_limits_int[0].max;
                }
            } else {
                // Редактируем минуты
                int step = gvs_schedule_param_limits_int[2].step;
                editing_minutes_value += step;
                if (editing_minutes_value > gvs_schedule_param_limits_int[2].max) {
                    editing_minutes_value = gvs_schedule_param_limits_int[2].max;
                }
            }
            update_schedule_day_param_display(editing_param_index);
        } else if (e & ENC_CLICK) {
            toggle_day_edit_mode_or_exit();
        }
        return;
    }
    
    // Обычный режим навигации
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_SCHEDULE_DAY);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, gvs_schedule_day_cont, menu_state, MENU_TYPE_GVS_SCHEDULE_DAY);
    
    if (prev_cursor != menu_state->cursor_index) {
        gvs_schedule_day_highlight_box(gvs_schedule_day_cont, menu_state->cursor_index);
    }
    
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Назад - возвращаемся в меню расписания
            ESP_LOGI(TAG, "Returning to GVS schedule menu from day menu");
            gvs_schedule_day_menu_hide();
            gvs_schedule_menu_show();
            encoder_manager_register_callback(gvs_schedule_menu_encoder_event_cb);
        } else {
            // Входим в режим редактирования
            int param_index = gvs_schedule_day_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_day_edit_mode(param_index);
            }
        }
    }
}

/**
 * @brief Очистка подменю дня
 */
void gvs_schedule_day_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up GVS schedule day menu");
    
    gvs_schedule_day_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    for (int i = 0; i < 4; i++) {
        day_value_labels[i] = NULL;
    }
    
    if (is_obj_valid(gvs_schedule_day_mask)) {
        lv_obj_del(gvs_schedule_day_mask);
        gvs_schedule_day_mask = NULL;
    }
    
    if (is_obj_valid(gvs_schedule_day_cont)) {
        lv_obj_del(gvs_schedule_day_cont);
        gvs_schedule_day_cont = NULL;
    }
    
    gvs_schedule_day_menu_initialized = false;
}

/**
 * @brief Инициализация подменю дня
 */
void GVS_Schedule_Day_Menu_List(int day_index) {
    ESP_LOGI(TAG, "Инициализация подменю дня %d ГВС", day_index);
    
    if (gvs_schedule_day_menu_creation_in_progress) {
        ESP_LOGW(TAG, "GVS schedule day menu creation already in progress, skipping");
        return;
    }
    
    gvs_schedule_day_menu_creation_in_progress = true;
    
    // Выходим из режима редактирования, если он активен
    if (edit_mode) {
        ESP_LOGI(TAG, "Exiting edit mode before switching to day %d", day_index);
        edit_mode = false;
        editing_param_index = -1;
        editing_hours = true;
    }
    
    current_gvs_schedule_day = day_index;
    
    if (gvs_schedule_day_menu_initialized && is_obj_valid(gvs_schedule_day_cont)) {
        ESP_LOGI(TAG, "GVS schedule day menu already initialized, updating for day %d", day_index);
        // Обновляем отображение всех параметров для нового дня
        for (int i = 0; i < 4; i++) {
            update_schedule_day_param_display(i);
        }
        // Обновляем подсветку
        menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_SCHEDULE_DAY);
        if (menu_state) {
            gvs_schedule_day_highlight_box(gvs_schedule_day_cont, menu_state->cursor_index);
        }
        gvs_schedule_day_menu_show();
        gvs_schedule_day_menu_creation_in_progress = false;
        return;
    }
    
    gvs_schedule_day_menu_cleanup();
    current_gvs_schedule_day = day_index;
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    gvs_schedule_day_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(gvs_schedule_day_cont)) {
        ESP_LOGE(TAG, "Failed to create GVS schedule day menu container");
        gvs_schedule_day_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(gvs_schedule_day_cont, 1200, 1200);
    lv_obj_center(gvs_schedule_day_cont);
    lv_obj_add_event_cb(gvs_schedule_day_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(gvs_schedule_day_cont, &style, 0);
    lv_obj_set_style_radius(gvs_schedule_day_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(gvs_schedule_day_cont, true, 0);
    lv_obj_set_scroll_dir(gvs_schedule_day_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(gvs_schedule_day_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(gvs_schedule_day_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(gvs_schedule_day_cont, 633, 0);
    lv_obj_set_style_bg_color(gvs_schedule_day_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(gvs_schedule_day_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(gvs_schedule_day_cont, 0, 0);
    lv_obj_set_style_pad_row(gvs_schedule_day_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(gvs_schedule_day_menu_items) / sizeof(GvsScheduleDayMenuItem); i++) {
        create_gvs_schedule_day_menu_item(gvs_schedule_day_cont, &gvs_schedule_day_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    gvs_schedule_day_mask = radial();
    if (is_obj_valid(gvs_schedule_day_mask)) {
        lv_obj_set_pos(gvs_schedule_day_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_GVS_SCHEDULE_DAY);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS_SCHEDULE_DAY);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(gvs_schedule_day_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    // Обновляем отображение всех параметров
    for (int i = 0; i < 4; i++) {
        update_schedule_day_param_display(i);
    }
    
    gvs_schedule_day_highlight_box(gvs_schedule_day_cont, menu_state->cursor_index);
    arc_menu_update_slide(gvs_schedule_day_cont);
    
    gvs_schedule_day_menu_initialized = true;
    gvs_schedule_day_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Подменю дня ГВС успешно инициализировано");
}


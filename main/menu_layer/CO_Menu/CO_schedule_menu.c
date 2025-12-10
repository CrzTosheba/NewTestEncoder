#include "CO_schedule_menu.h"
#include "CO_schedule_params.h"
#include "co_params_limits.h"
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

static const char *TAG = "CO_SCHEDULE_MENU";

// Forward declarations
static void update_schedule_day_param_display(int param_index);
static void update_day_param_display(int param_index, int day_index);

// Структура элемента главного меню расписания (дни недели)
typedef struct {
    const char *label_text;     // Текст элемента
    const void *img_src;        // Иконка (только для "Назад")
    int day_index;              // Индекс дня (-1 для "Назад", 0-5 для дней Пн-Сб)
} CoScheduleMenuItem;

// Элементы главного меню расписания
static const CoScheduleMenuItem co_schedule_menu_items[] = {
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
} CoScheduleDayMenuItem;

// Элементы подменю дня недели
// param_index: 0=From1, 1=To1, 2=From2, 3=To2
static const CoScheduleDayMenuItem co_schedule_day_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Комф. период 1. C", NULL, 0},        // From1 (часы + минуты)
    {"Комф. период 1. До", NULL, 1},       // To1 (часы + минуты)
    {"Комф. период 2. C", NULL, 2},        // From2 (часы + минуты)
    {"Комф. период 2. До", NULL, 3},       // To2 (часы + минуты)
};

// Локальные переменные для главного меню расписания
lv_obj_t *co_schedule_cont = NULL;
static bool co_schedule_menu_initialized = false;
static bool co_schedule_menu_creation_in_progress = false;
static lv_obj_t *co_schedule_mask = NULL;

// Локальные переменные для подменю дня недели
lv_obj_t *co_schedule_day_cont = NULL;
static bool co_schedule_day_menu_initialized = false;
static bool co_schedule_day_menu_creation_in_progress = false;
static lv_obj_t *co_schedule_day_mask = NULL;
int current_schedule_day = 0; // Текущий выбранный день (0-6: Пн-Вс)

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
    static int* mon_hours[4] = {&MonHoursFrom1, &MonHoursTo1, &MonHoursFrom2, &MonHoursTo2};
    static int* tue_hours[4] = {&TueHoursFrom1, &TueHoursTo1, &TueHoursFrom2, &TueHoursTo2};
    static int* wed_hours[4] = {&WedHoursFrom1, &WedHoursTo1, &WedHoursFrom2, &WedHoursTo2};
    static int* thu_hours[4] = {&ThuHoursFrom1, &ThuHoursTo1, &ThuHoursFrom2, &ThuHoursTo2};
    static int* fri_hours[4] = {&FriHoursFrom1, &FriHoursTo1, &FriHoursFrom2, &FriHoursTo2};
    static int* sat_hours[4] = {&SatHoursFrom1, &SatHoursTo1, &SatHoursFrom2, &SatHoursTo2};
    static int* sun_hours[4] = {&SunHoursFrom1, &SunHoursTo1, &SunHoursFrom2, &SunHoursTo2};
    
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
    static int* mon_minutes[4] = {&MonMinFrom1, &MonMinTo1, &MonMinFrom2, &MonMinTo2};
    static int* tue_minutes[4] = {&TueMinFrom1, &TueMinTo1, &TueMinFrom2, &TueMinTo2};
    static int* wed_minutes[4] = {&WedMinFrom1, &WedMinTo1, &WedMinFrom2, &WedMinTo2};
    static int* thu_minutes[4] = {&ThuMinFrom1, &ThuMinTo1, &ThuMinFrom2, &ThuMinTo2};
    static int* fri_minutes[4] = {&FriMinFrom1, &FriMinTo1, &FriMinFrom2, &FriMinTo2};
    static int* sat_minutes[4] = {&SatMinFrom1, &SatMinTo1, &SatMinFrom2, &SatMinTo2};
    static int* sun_minutes[4] = {&SunMinFrom1, &SunMinTo1, &SunMinFrom2, &SunMinTo2};
    
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
        update_day_param_display(param_index, current_schedule_day);
    }
}

/**
 * @brief Подсветка выбранного элемента главного меню расписания
 */
static void co_schedule_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
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
static void create_co_schedule_menu_item(lv_obj_t *cont, const CoScheduleMenuItem *item) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_co_schedule_menu_item");
        return;
    }
    
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_co_schedule_menu_item");
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
    
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);
    vTaskDelay(pdMS_TO_TICKS(1));
}

/**
 * @brief Показывает главное меню расписания
 */
void co_schedule_menu_show(void) {
    ESP_LOGI(TAG, "Showing CO schedule menu");
    if (is_obj_valid(co_schedule_cont)) {
        lv_obj_clear_flag(co_schedule_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(co_schedule_mask)) {
        co_schedule_mask = radial();
        if (is_obj_valid(co_schedule_mask)) {
            lv_obj_set_pos(co_schedule_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(co_schedule_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает главное меню расписания
 */
void co_schedule_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding CO schedule menu");
    if (is_obj_valid(co_schedule_cont)) {
        lv_obj_add_flag(co_schedule_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(co_schedule_mask)) {
        lv_obj_add_flag(co_schedule_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Обработчик событий энкодера для главного меню расписания
 */
void co_schedule_menu_encoder_event_cb(uint8_t e) {
    if (!is_obj_valid(co_schedule_cont)) {
        ESP_LOGE(TAG, "Контейнер меню расписания не инициализирован");
        return;
    }
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_SCHEDULE);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, co_schedule_cont, menu_state, MENU_TYPE_CO_SCHEDULE);
    
    if (prev_cursor != menu_state->cursor_index) {
        co_schedule_highlight_box(co_schedule_cont, menu_state->cursor_index);
    }
    
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Назад - возвращаемся в меню отопления
            ESP_LOGI(TAG, "Returning to CO menu from schedule menu");
            co_schedule_menu_hide();
            extern void co_menu_show(void);
            co_menu_show();
            extern void co_menu_encoder_event_cb(uint8_t e);
            encoder_manager_register_callback(co_menu_encoder_event_cb);
        } else {
            // Открываем подменю дня недели
            int day_index = co_schedule_menu_items[menu_state->cursor_index].day_index;
            if (day_index >= 0 && day_index <= 6) {
                ESP_LOGI(TAG, "Opening schedule day menu for day %d", day_index);
                current_schedule_day = day_index;
                co_schedule_menu_hide();
                CO_Schedule_Day_Menu_List(day_index);
                co_schedule_day_menu_show();
                encoder_manager_register_callback(co_schedule_day_menu_encoder_event_cb);
            }
        }
    }
}

/**
 * @brief Очистка главного меню расписания
 */
void co_schedule_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up CO schedule menu");
    
    co_schedule_menu_creation_in_progress = false;
    
    if (is_obj_valid(co_schedule_mask)) {
        lv_obj_del(co_schedule_mask);
        co_schedule_mask = NULL;
    }
    
    if (is_obj_valid(co_schedule_cont)) {
        lv_obj_del(co_schedule_cont);
        co_schedule_cont = NULL;
    }
    
    co_schedule_menu_initialized = false;
}

/**
 * @brief Инициализация главного меню расписания
 */
void CO_Schedule_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация главного меню расписания");
    
    if (co_schedule_menu_creation_in_progress) {
        ESP_LOGW(TAG, "CO schedule menu creation already in progress, skipping");
        return;
    }
    
    co_schedule_menu_creation_in_progress = true;
    
    if (co_schedule_menu_initialized && is_obj_valid(co_schedule_cont)) {
        ESP_LOGI(TAG, "CO schedule menu already initialized, showing it");
        co_schedule_menu_show();
        co_schedule_menu_creation_in_progress = false;
        return;
    }
    
    co_schedule_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    co_schedule_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(co_schedule_cont)) {
        ESP_LOGE(TAG, "Failed to create CO schedule menu container");
        co_schedule_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(co_schedule_cont, 1200, 1200);
    lv_obj_center(co_schedule_cont);
    lv_obj_add_event_cb(co_schedule_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(co_schedule_cont, &style, 0);
    lv_obj_set_style_radius(co_schedule_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(co_schedule_cont, true, 0);
    lv_obj_set_scroll_dir(co_schedule_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(co_schedule_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(co_schedule_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(co_schedule_cont, 633, 0);
    lv_obj_set_style_bg_color(co_schedule_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(co_schedule_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(co_schedule_cont, 0, 0);
    lv_obj_set_style_pad_row(co_schedule_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(co_schedule_menu_items) / sizeof(CoScheduleMenuItem); i++) {
        create_co_schedule_menu_item(co_schedule_cont, &co_schedule_menu_items[i]);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    co_schedule_mask = radial();
    if (is_obj_valid(co_schedule_mask)) {
        lv_obj_set_pos(co_schedule_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_CO_SCHEDULE);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_SCHEDULE);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(co_schedule_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    co_schedule_highlight_box(co_schedule_cont, menu_state->cursor_index);
    arc_menu_update_slide(co_schedule_cont);
    
    co_schedule_menu_initialized = true;
    co_schedule_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Главное меню расписания успешно инициализировано");
}

/**
 * @brief Подсветка выбранного элемента подменю дня
 */
static void co_schedule_day_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
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
                    if (!(edit_mode && editing_param_index == co_schedule_day_menu_items[i].param_index)) {
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
            if (!(edit_mode && editing_param_index == co_schedule_day_menu_items[i].param_index)) {
                lv_obj_set_style_bg_color(child, lv_color_hex(0xFFCC00), LV_PART_MAIN);
            }
        } else {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x2B3639), LV_PART_MAIN);
        }
    }
}

/**
 * @brief Создание элемента подменю дня
 */
static void create_co_schedule_day_menu_item(lv_obj_t *cont, const CoScheduleDayMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_co_schedule_day_menu_item");
        return;
    }
    
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_co_schedule_day_menu_item");
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
    
    // Значение параметра справа (только для редактируемых параметров)
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
                day_value_labels[item->param_index] = value_label;
                lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_font(value_label, &Roboto_bold_24, 0);
                lv_obj_align(value_label, LV_ALIGN_CENTER, 0, 0);
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
    
    int* hours_ptr = get_day_hours_ptr(current_schedule_day, editing_param_index);
    int* minutes_ptr = get_day_minutes_ptr(current_schedule_day, editing_param_index);
    
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
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_SCHEDULE_DAY);
    if (menu_state) {
        co_schedule_day_highlight_box(co_schedule_day_cont, menu_state->cursor_index);
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
        int* hours_ptr = get_day_hours_ptr(current_schedule_day, saved_index);
        int* minutes_ptr = get_day_minutes_ptr(current_schedule_day, saved_index);
        
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
    
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_SCHEDULE_DAY);
    if (menu_state) {
        co_schedule_day_highlight_box(co_schedule_day_cont, menu_state->cursor_index);
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
    
    int* hours_ptr = get_day_hours_ptr(current_schedule_day, period_index);
    int* minutes_ptr = get_day_minutes_ptr(current_schedule_day, period_index);
    
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
        int* hours_ptr = get_day_hours_ptr(current_schedule_day, editing_param_index);
        if (hours_ptr) {
            *hours_ptr = editing_hours_value;
        }
        
        editing_hours = false;
        ESP_LOGI(TAG, "Switching to minutes editing for period %d", editing_param_index);
        update_schedule_day_param_display(editing_param_index);
    } else {
        // Выходим из режима редактирования и проверяем изменения
        // Сохраняем измененное значение минут обратно в переменную
        int* minutes_ptr = get_day_minutes_ptr(current_schedule_day, editing_param_index);
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
void co_schedule_day_menu_show(void) {
    ESP_LOGI(TAG, "Showing CO schedule day menu");
    if (is_obj_valid(co_schedule_day_cont)) {
        lv_obj_clear_flag(co_schedule_day_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(co_schedule_day_mask)) {
        co_schedule_day_mask = radial();
        if (is_obj_valid(co_schedule_day_mask)) {
            lv_obj_set_pos(co_schedule_day_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(co_schedule_day_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает подменю дня
 */
void co_schedule_day_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding CO schedule day menu");
    if (is_obj_valid(co_schedule_day_cont)) {
        lv_obj_add_flag(co_schedule_day_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(co_schedule_day_mask)) {
        lv_obj_add_flag(co_schedule_day_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Обработчик событий энкодера для подменю дня
 */
void co_schedule_day_menu_encoder_event_cb(uint8_t e) {
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(co_schedule_day_cont)) {
        ESP_LOGE(TAG, "Контейнер подменю дня не инициализирован");
        return;
    }
    
    // Если в режиме редактирования
    if (edit_mode && editing_param_index >= 0) {
        if (e & ENC_LEFT) {
            // Уменьшаем значение
            if (editing_hours) {
                // Редактируем часы
                int step = co_schedule_param_limits_int[0].step;  // Используем первый элемент для часов
                editing_hours_value -= step;
                if (editing_hours_value < co_schedule_param_limits_int[0].min) {
                    editing_hours_value = co_schedule_param_limits_int[0].min;
                }
            } else {
                // Редактируем минуты
                int step = co_schedule_param_limits_int[2].step;  // Используем третий элемент для минут
                editing_minutes_value -= step;
                if (editing_minutes_value < co_schedule_param_limits_int[2].min) {
                    editing_minutes_value = co_schedule_param_limits_int[2].min;
                }
            }
            update_schedule_day_param_display(editing_param_index);
        } else if (e & ENC_RIGHT) {
            // Увеличиваем значение
            if (editing_hours) {
                // Редактируем часы
                int step = co_schedule_param_limits_int[0].step;
                editing_hours_value += step;
                if (editing_hours_value > co_schedule_param_limits_int[0].max) {
                    editing_hours_value = co_schedule_param_limits_int[0].max;
                }
            } else {
                // Редактируем минуты
                int step = co_schedule_param_limits_int[2].step;
                editing_minutes_value += step;
                if (editing_minutes_value > co_schedule_param_limits_int[2].max) {
                    editing_minutes_value = co_schedule_param_limits_int[2].max;
                }
            }
            update_schedule_day_param_display(editing_param_index);
        } else if (e & ENC_CLICK) {
            toggle_day_edit_mode_or_exit();
        }
        return;
    }
    
    // Обычный режим навигации
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_SCHEDULE_DAY);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, co_schedule_day_cont, menu_state, MENU_TYPE_CO_SCHEDULE_DAY);
    
    if (prev_cursor != menu_state->cursor_index) {
        co_schedule_day_highlight_box(co_schedule_day_cont, menu_state->cursor_index);
    }
    
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Назад - возвращаемся в меню расписания
            ESP_LOGI(TAG, "Returning to schedule menu from day menu");
            co_schedule_day_menu_hide();
            co_schedule_menu_show();
            encoder_manager_register_callback(co_schedule_menu_encoder_event_cb);
        } else {
            // Входим в режим редактирования
            int param_index = co_schedule_day_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_day_edit_mode(param_index);
            }
        }
    }
}

/**
 * @brief Очистка подменю дня
 */
void co_schedule_day_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up CO schedule day menu");
    
    co_schedule_day_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    for (int i = 0; i < 4; i++) {
        day_value_labels[i] = NULL;
    }
    
    if (is_obj_valid(co_schedule_day_mask)) {
        lv_obj_del(co_schedule_day_mask);
        co_schedule_day_mask = NULL;
    }
    
    if (is_obj_valid(co_schedule_day_cont)) {
        lv_obj_del(co_schedule_day_cont);
        co_schedule_day_cont = NULL;
    }
    
    co_schedule_day_menu_initialized = false;
}

/**
 * @brief Инициализация подменю дня
 */
void CO_Schedule_Day_Menu_List(int day_index) {
    ESP_LOGI(TAG, "Инициализация подменю дня %d", day_index);
    
    if (co_schedule_day_menu_creation_in_progress) {
        ESP_LOGW(TAG, "CO schedule day menu creation already in progress, skipping");
        return;
    }
    
    co_schedule_day_menu_creation_in_progress = true;
    
    // Выходим из режима редактирования, если он активен
    if (edit_mode) {
        ESP_LOGI(TAG, "Exiting edit mode before switching to day %d", day_index);
        edit_mode = false;
        editing_param_index = -1;
        editing_hours = true;
    }
    
    current_schedule_day = day_index;
    
    if (co_schedule_day_menu_initialized && is_obj_valid(co_schedule_day_cont)) {
        ESP_LOGI(TAG, "CO schedule day menu already initialized, updating for day %d", day_index);
        // Обновляем отображение всех параметров для нового дня
        for (int i = 0; i < 4; i++) {
            update_schedule_day_param_display(i);
        }
        // Обновляем подсветку
        menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_SCHEDULE_DAY);
        if (menu_state) {
            co_schedule_day_highlight_box(co_schedule_day_cont, menu_state->cursor_index);
        }
        co_schedule_day_menu_show();
        co_schedule_day_menu_creation_in_progress = false;
        return;
    }
    
    co_schedule_day_menu_cleanup();
    current_schedule_day = day_index;
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    co_schedule_day_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(co_schedule_day_cont)) {
        ESP_LOGE(TAG, "Failed to create CO schedule day menu container");
        co_schedule_day_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(co_schedule_day_cont, 1200, 1200);
    lv_obj_center(co_schedule_day_cont);
    lv_obj_add_event_cb(co_schedule_day_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(co_schedule_day_cont, &style, 0);
    lv_obj_set_style_radius(co_schedule_day_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(co_schedule_day_cont, true, 0);
    lv_obj_set_scroll_dir(co_schedule_day_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(co_schedule_day_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(co_schedule_day_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(co_schedule_day_cont, 633, 0);
    lv_obj_set_style_bg_color(co_schedule_day_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(co_schedule_day_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(co_schedule_day_cont, 0, 0);
    lv_obj_set_style_pad_row(co_schedule_day_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(co_schedule_day_menu_items) / sizeof(CoScheduleDayMenuItem); i++) {
        create_co_schedule_day_menu_item(co_schedule_day_cont, &co_schedule_day_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    co_schedule_day_mask = radial();
    if (is_obj_valid(co_schedule_day_mask)) {
        lv_obj_set_pos(co_schedule_day_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_CO_SCHEDULE_DAY);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_SCHEDULE_DAY);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(co_schedule_day_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    // Обновляем отображение всех параметров
    for (int i = 0; i < 4; i++) {
        update_schedule_day_param_display(i);
    }
    
    co_schedule_day_highlight_box(co_schedule_day_cont, menu_state->cursor_index);
    arc_menu_update_slide(co_schedule_day_cont);
    
    co_schedule_day_menu_initialized = true;
    co_schedule_day_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Подменю дня успешно инициализировано");
}


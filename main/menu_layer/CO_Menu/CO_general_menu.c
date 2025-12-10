#include "CO_general_menu.h"
#include "CO_general_params.h"
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

static const char *TAG = "CO_GENERAL_MENU";

// Forward declarations
static void update_param_display(int param_index);

// Структура элемента меню общие
typedef struct {
    const char *label_text;     // Текст элемента слева
    const void *img_src;        // Иконка (только для "Назад")
    int param_index;            // Индекс параметра (-1 для "Назад")
} CoGeneralMenuItem;

// Элементы меню общие
static const CoGeneralMenuItem co_general_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right, -1},
    {"Режим", NULL, 0},                    // Mode
    {"Тэконом", NULL, 1},                 // T1-Econom
    {"Ткомф", NULL, 2},                   // T1-Comfort
    {"Тожид", NULL, 3},                   // T1-Standby
    {"Макс.Тпод_СО", NULL, 4},            // T1-DesiredMax
    {"Мин.Тпод_СО", NULL, 5},             // T1-DesiredMin
};

// Локальные переменные для меню общие
lv_obj_t *co_general_cont = NULL;
static bool co_general_menu_initialized = false;
static bool co_general_menu_creation_in_progress = false;
static lv_obj_t *co_general_mask = NULL;

// Массив указателей на label для значений параметров
static lv_obj_t *value_labels[6] = {NULL};

// Состояние редактирования
static bool edit_mode = false;
static int editing_param_index = -1;
static float editing_float_value = 0.0f;
static heating_mode_t editing_mode_value = MODE_COMF;  // По умолчанию КОМФ

// Используем пределы из co_params_limits.h
#define param_limits co_general_param_limits

// Временные значения для отмены изменений
static float temp_T1_Econom = 0.0f;
static float temp_T1_Comfort = 0.0f;
static float temp_T1_Standby = 0.0f;
static float temp_T1_DesiredMax = 0.0f;
static float temp_T1_DesiredMin = 0.0f;
static heating_mode_t temp_Mode = MODE_COMF;

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
 * @brief Получает строковое представление режима
 */
static const char* get_mode_string(heating_mode_t mode) {
    switch(mode) {
        case MODE_MANUAL:   return "РУЧН";
        case MODE_SCHEDULE: return "РАСП";
        case MODE_ECON:     return "ЭКОН";
        case MODE_COMF:     return "КОМФ";
        case MODE_ALARM:    return "АВАР";
        default:            return "???";
    }
}

/**
 * @brief Подсветка выбранного элемента меню общие
 */
static void co_general_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
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
                    if (!(edit_mode && editing_param_index == co_general_menu_items[i].param_index)) {
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
            if (!(edit_mode && editing_param_index == co_general_menu_items[i].param_index)) {
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
    if (param_index < 0 || param_index >= 6) return;
    if (!is_obj_valid(value_labels[param_index])) return;
    
    char value_str[32];
    
    if (edit_mode && editing_param_index == param_index) {
        // В режиме редактирования показываем временное значение
        if (param_index == 0) {
            // Режим
            snprintf(value_str, sizeof(value_str), "%s", get_mode_string(editing_mode_value));
        } else {
            // Float параметры
            format_float_value(value_str, sizeof(value_str), editing_float_value);
        }
    } else {
        // Обычный режим - показываем текущее значение
        switch(param_index) {
            case 0: // Mode
                snprintf(value_str, sizeof(value_str), "%s", get_mode_string(Mode));
                break;
            case 1: // T1-Econom
                format_float_value(value_str, sizeof(value_str), T1_Econom);
                break;
            case 2: // T1-Comfort
                format_float_value(value_str, sizeof(value_str), T1_Comfort);
                break;
            case 3: // T1-Standby
                format_float_value(value_str, sizeof(value_str), T1_Standby);
                break;
            case 4: // T1-DesiredMax
                format_float_value(value_str, sizeof(value_str), T1_DesiredMax);
                break;
            case 5: // T1-DesiredMin
                format_float_value(value_str, sizeof(value_str), T1_DesiredMin);
                break;
        }
    }
    
    // Добавляем единицы измерения
    if (param_index > 0) {
        char full_str[40];
        snprintf(full_str, sizeof(full_str), "%s °C", value_str);
        lv_label_set_text(value_labels[param_index], full_str);
    } else {
        lv_label_set_text(value_labels[param_index], value_str);
    }
    
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
    ESP_LOGI(TAG, "Saving parameter changes");
    
    int saved_index = editing_param_index;
    
    if (editing_param_index == 0) {
        // Режим
        Mode = editing_mode_value;
    } else {
        // Float параметры
        switch(editing_param_index) {
            case 1: T1_Econom = editing_float_value; break;
            case 2: T1_Comfort = editing_float_value; break;
            case 3: T1_Standby = editing_float_value; break;
            case 4: T1_DesiredMax = editing_float_value; break;
            case 5: T1_DesiredMin = editing_float_value; break;
        }
    }
    
    // НЕ сохраняем параметры в NVS (как в ГВС)
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_GENERAL);
    if (menu_state) {
        co_general_highlight_box(co_general_cont, menu_state->cursor_index);
    }
}

/**
 * @brief Отменяет изменения параметра
 */
static void cancel_param_changes(void) {
    ESP_LOGI(TAG, "Canceling parameter changes");
    
    int saved_index = editing_param_index;
    
    // Восстанавливаем временные значения
    if (editing_param_index == 0) {
        editing_mode_value = temp_Mode;
    } else {
        switch(editing_param_index) {
            case 1: editing_float_value = temp_T1_Econom; break;
            case 2: editing_float_value = temp_T1_Comfort; break;
            case 3: editing_float_value = temp_T1_Standby; break;
            case 4: editing_float_value = temp_T1_DesiredMax; break;
            case 5: editing_float_value = temp_T1_DesiredMin; break;
        }
    }
    
    edit_mode = false;
    editing_param_index = -1;
    update_param_display(saved_index);
    
    // Восстанавливаем подсветку текущего элемента
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_GENERAL);
    if (menu_state) {
        co_general_highlight_box(co_general_cont, menu_state->cursor_index);
    }
}

/**
 * @brief Входит в режим редактирования параметра
 */
static void enter_edit_mode(int param_index) {
    if (param_index < 0 || param_index >= 6) return;
    
    // Проверяем доступ перед редактированием
    if (!access_control_is_unlocked()) {
        ESP_LOGW(TAG, "Access denied: cannot edit parameters when access is locked");
        return;
    }
    
    ESP_LOGI(TAG, "Entering edit mode for parameter %d", param_index);
    
    edit_mode = true;
    editing_param_index = param_index;
    
    // Сохраняем текущие значения как временные
    if (param_index == 0) {
        temp_Mode = Mode;
        editing_mode_value = Mode;
    } else {
        switch(param_index) {
            case 1: 
                temp_T1_Econom = T1_Econom;
                editing_float_value = T1_Econom;
                break;
            case 2:
                temp_T1_Comfort = T1_Comfort;
                editing_float_value = T1_Comfort;
                break;
            case 3:
                temp_T1_Standby = T1_Standby;
                editing_float_value = T1_Standby;
                break;
            case 4:
                temp_T1_DesiredMax = T1_DesiredMax;
                editing_float_value = T1_DesiredMax;
                break;
            case 5:
                temp_T1_DesiredMin = T1_DesiredMin;
                editing_float_value = T1_DesiredMin;
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
    
    // Проверяем, изменилось ли значение
    if (editing_param_index == 0) {
        value_changed = (editing_mode_value != temp_Mode);
    } else {
        switch(editing_param_index) {
            case 1: value_changed = (fabs(editing_float_value - temp_T1_Econom) > 0.01f); break;
            case 2: value_changed = (fabs(editing_float_value - temp_T1_Comfort) > 0.01f); break;
            case 3: value_changed = (fabs(editing_float_value - temp_T1_Standby) > 0.01f); break;
            case 4: value_changed = (fabs(editing_float_value - temp_T1_DesiredMax) > 0.01f); break;
            case 5: value_changed = (fabs(editing_float_value - temp_T1_DesiredMin) > 0.01f); break;
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
 * @brief Создание элемента меню общие
 */
static void create_co_general_menu_item(lv_obj_t *cont, const CoGeneralMenuItem *item, int index) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_co_general_menu_item");
        return;
    }
    
    // Создаем контейнер для элемента (высота 40px)
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_co_general_menu_item");
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
 * @brief Показывает меню общие
 */
void co_general_menu_show(void) {
    ESP_LOGI(TAG, "Showing CO general menu");
    if (is_obj_valid(co_general_cont)) {
        lv_obj_clear_flag(co_general_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (!is_obj_valid(co_general_mask)) {
        co_general_mask = radial();
        if (is_obj_valid(co_general_mask)) {
            lv_obj_set_pos(co_general_mask, 433, 70);
        }
    } else {
        lv_obj_clear_flag(co_general_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает меню общие
 */
void co_general_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding CO general menu");
    if (is_obj_valid(co_general_cont)) {
        lv_obj_add_flag(co_general_cont, LV_OBJ_FLAG_HIDDEN);
    }
    if (is_obj_valid(co_general_mask)) {
        lv_obj_del(co_general_mask);
        co_general_mask = NULL;
    }
}

/**
 * @brief Обработчик событий энкодера для меню общие
 */
void co_general_menu_encoder_event_cb(uint8_t e) {
    // Если активно окно подтверждения, передаем события ему
    extern bool confirmation_active;
    if (confirmation_active) {
        yes_no_menu_encoder_event_cb(e);
        return;
    }
    
    if (!is_obj_valid(co_general_cont)) {
        ESP_LOGE(TAG, "Контейнер меню общие не инициализирован");
        return;
    }
    
    // Если в режиме редактирования
    if (edit_mode && editing_param_index >= 0) {
        if (e & ENC_LEFT) {
            if (editing_param_index == 0) {
                // Режим - переключаем циклически: РУЧН -> РАСП -> ЭКОН -> КОМФ -> АВАР -> РУЧН
                if (editing_mode_value == MODE_MANUAL) {
                    editing_mode_value = MODE_ALARM;
                } else {
                    editing_mode_value = (heating_mode_t)((int)editing_mode_value - 1);
                }
            } else {
                // Float параметры - уменьшаем на step
                float step = param_limits[editing_param_index].step;
                editing_float_value -= step;
                if (editing_float_value < param_limits[editing_param_index].min) {
                    editing_float_value = param_limits[editing_param_index].min;
                }
            }
            update_param_display(editing_param_index);
        } else if (e & ENC_RIGHT) {
            if (editing_param_index == 0) {
                // Режим - переключаем циклически: РУЧН -> РАСП -> ЭКОН -> КОМФ -> АВАР -> РУЧН
                if (editing_mode_value == MODE_ALARM) {
                    editing_mode_value = MODE_MANUAL;
                } else {
                    editing_mode_value = (heating_mode_t)((int)editing_mode_value + 1);
                }
            } else {
                // Float параметры - увеличиваем на step
                float step = param_limits[editing_param_index].step;
                editing_float_value += step;
                if (editing_float_value > param_limits[editing_param_index].max) {
                    editing_float_value = param_limits[editing_param_index].max;
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
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_GENERAL);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    arc_menu_handle_encoder(e, co_general_cont, menu_state, MENU_TYPE_CO_GENERAL);
    
    if (prev_cursor != menu_state->cursor_index) {
        co_general_highlight_box(co_general_cont, menu_state->cursor_index);
    }
    
    // Обработка нажатия кнопки
    if (e & ENC_CLICK) {
        if (menu_state->cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в меню отопления
            ESP_LOGI(TAG, "Returning to CO menu from general menu");
            co_general_menu_hide();
            co_menu_show();
            // Переключаем обработчик энкодера
            extern void co_menu_encoder_event_cb(uint8_t e);
            encoder_manager_register_callback(co_menu_encoder_event_cb);
        } else {
            // Входим в режим редактирования
            int param_index = co_general_menu_items[menu_state->cursor_index].param_index;
            if (param_index >= 0) {
                enter_edit_mode(param_index);
            }
        }
    }
}

/**
 * @brief Очистка меню общие
 */
void co_general_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up CO general menu");
    
    co_general_menu_creation_in_progress = false;
    edit_mode = false;
    editing_param_index = -1;
    
    // Очищаем массив указателей на labels
    for (int i = 0; i < 6; i++) {
        value_labels[i] = NULL;
    }
    
    if (is_obj_valid(co_general_mask)) {
        lv_obj_del(co_general_mask);
        co_general_mask = NULL;
    }
    
    if (is_obj_valid(co_general_cont)) {
        lv_obj_del(co_general_cont);
        co_general_cont = NULL;
    }
    
    co_general_menu_initialized = false;
}

/**
 * @brief Инициализация меню общие
 */
void CO_General_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню общие настройки отопления");
    
    if (co_general_menu_creation_in_progress) {
        ESP_LOGW(TAG, "CO general menu creation already in progress, skipping");
        return;
    }
    
    co_general_menu_creation_in_progress = true;
    
    if (co_general_menu_initialized && is_obj_valid(co_general_cont)) {
        ESP_LOGI(TAG, "CO general menu already initialized, showing it");
        co_general_menu_show();
        co_general_menu_creation_in_progress = false;
        return;
    }
    
    co_general_menu_cleanup();
    
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    co_general_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(co_general_cont)) {
        ESP_LOGE(TAG, "Failed to create CO general menu container");
        co_general_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(co_general_cont, 1200, 1200);
    lv_obj_center(co_general_cont);
    lv_obj_add_event_cb(co_general_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(co_general_cont, &style, 0);
    lv_obj_set_style_radius(co_general_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(co_general_cont, true, 0);
    lv_obj_set_scroll_dir(co_general_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(co_general_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(co_general_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(co_general_cont, 633, 0);
    lv_obj_set_style_bg_color(co_general_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(co_general_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(co_general_cont, 0, 0);
    lv_obj_set_style_pad_row(co_general_cont, 1, 0);
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    for (uint32_t i = 0; i < sizeof(co_general_menu_items) / sizeof(CoGeneralMenuItem); i++) {
        create_co_general_menu_item(co_general_cont, &co_general_menu_items[i], i);
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
    
    co_general_mask = radial();
    if (is_obj_valid(co_general_mask)) {
        lv_obj_set_pos(co_general_mask, 433, 70);
    }
    
    const menu_config_t* config = get_menu_config(MENU_TYPE_CO_GENERAL);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO_GENERAL);
    
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_t *target_child = lv_obj_get_child(co_general_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    co_general_highlight_box(co_general_cont, menu_state->cursor_index);
    arc_menu_update_slide(co_general_cont);
    
    co_general_menu_initialized = true;
    co_general_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню общие настройки отопления успешно инициализировано");
}


#include "In_Out_main_menu.h"
#include "encoder/encoder.h"
#include "my_widgets/w_rad_mask.h"
#include "screen_logic/arc_menu.h"
#include "screen_logic/screen_navigation.h"
#include "screen_logic/access_control.h"
#include "screens/S_In_Out/1_layer/screen_In_Out.h"
#include <stdint.h>
#include <inttypes.h>
#include "esp_log.h"

static const char *TAG = "IO_MENU";

// Структура элемента меню входов/выходов
typedef struct {
    const char *label_text;     // Текст элемента
    const void *img_src;        // Иконка (только для "Назад")
} IoMenuItem;

// Элементы меню входов/выходов
static const IoMenuItem io_menu_items[] = {
   {"                                Назад", &lv_im_arrow_right},
    {"Все", NULL},
    {"Универсальные входы", NULL},
    {"Аналоговые выходы", NULL},
    {"Дискретные выходы", NULL},
};

// Локальные переменные для меню входов/выходов
static lv_obj_t *io_cont = NULL;
static lv_obj_t *io_mask = NULL;
static bool io_menu_initialized = false;
static bool io_menu_creation_in_progress = false;

/**
 * @brief Проверяет, является ли объект валидным
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Создание элемента меню входов/выходов
 */
static void create_io_menu_item(lv_obj_t *cont, const IoMenuItem *item) {
    // Создаем контейнер для элемента (такой же как в главном меню)
    lv_obj_t *box = lv_obj_create(cont);
    lv_obj_set_size(box, 462, 40);
    lv_obj_set_style_border_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x2B3639), 0);
    
    // Основная надпись
    lv_obj_t *label = lv_label_create(box);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
    lv_label_set_text(label, item->label_text);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);
    
    // Иконка (только для "Назад")
    if (item->img_src != NULL) {
        lv_obj_t *img = lv_img_create(box);
        lv_img_set_src(img, item->img_src);
        lv_obj_set_style_img_recolor(img, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
        lv_obj_align(img, LV_ALIGN_CENTER, 90, 0);
    }
    
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);
}

/**
 * @brief Подсветка выбранного элемента меню входов/выходов
 */
static void io_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
    if (!is_obj_valid(cont)) return;
    
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        if (!is_obj_valid(child)) continue;
        
        uint32_t grand_child_cnt = lv_obj_get_child_cnt(child);
        
        // Обрабатываем все дочерние элементы контейнера
        for (uint32_t j = 0; j < grand_child_cnt; j++) {
            lv_obj_t *grand_child = lv_obj_get_child(child, j);
            if (!is_obj_valid(grand_child)) continue;
            
            if (lv_obj_check_type(grand_child, &lv_label_class)) {
                // Это метка - меняем цвет текста
                if (i == cursor_index) {
                    lv_obj_set_style_text_color(grand_child, lv_color_hex(0x000000), LV_PART_MAIN);
                } else {
                    lv_obj_set_style_text_color(grand_child, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                }
            } else if (lv_obj_check_type(grand_child, &lv_image_class)) {
                // Это изображение - меняем цвет стрелки
                if (i == cursor_index) {
                    lv_obj_set_style_img_recolor(grand_child, lv_color_hex(0x000000), 0);
                } else {
                    lv_obj_set_style_img_recolor(grand_child, lv_color_hex(0xFFFFFF), 0);
                }
            }
        }
        
        // Меняем фон контейнера
        if (i == cursor_index) {
            lv_obj_set_style_bg_color(child, lv_color_hex(0xFFCC00), LV_PART_MAIN);
        } else {
            lv_obj_set_style_bg_color(child, lv_color_hex(0x2B3639), LV_PART_MAIN);
        }
    }
}

/**
 * @brief Показывает меню входов/выходов
 */
void input_output_menu_show(void) {
    ESP_LOGI(TAG, "Showing IO menu");
    if (is_obj_valid(io_cont)) {
        lv_obj_clear_flag(io_cont, LV_OBJ_FLAG_HIDDEN);
    }
    // Пересоздаем маску, если она была удалена
    if (!is_obj_valid(io_mask)) {
        io_mask = radial();
        if (is_obj_valid(io_mask)) {
            lv_obj_set_pos(io_mask, 433, 70);
            ESP_LOGI(TAG, "IO menu mask recreated");
        }
    } else {
        lv_obj_clear_flag(io_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает меню входов/выходов
 */
void input_output_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding IO menu");
    if (is_obj_valid(io_cont)) {
        lv_obj_add_flag(io_cont, LV_OBJ_FLAG_HIDDEN);
    }
    // ВАЖНО: Удаляем маску при скрытии, чтобы она не перекрывала главный экран
    if (is_obj_valid(io_mask)) {
        lv_obj_del(io_mask);
        io_mask = NULL;
        ESP_LOGI(TAG, "IO menu mask deleted");
    }
}

/**
 * @brief Обработчик событий энкодера для меню входов/выходов
 */
void input_output_encoder_event_cb(uint8_t e) {
    // Обновляем таймер активности при любом действии пользователя
    access_control_update_activity_timer();
    
    if (!is_obj_valid(io_cont)) {
        ESP_LOGE(TAG, "Контейнер меню входов/выходов не инициализирован");
        return;
    }
    
    // Получаем состояние меню входов/выходов
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_IN_OUT);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    ESP_LOGI(TAG, "IO menu encoder event: 0x%02x, current_cursor_index: %" PRIu32, e, menu_state->cursor_index);
    
    // Используем конфигурацию для меню входов/выходов
    arc_menu_handle_encoder(e, io_cont, menu_state, MENU_TYPE_IN_OUT);
    
    ESP_LOGI(TAG, "After arc_menu_handle_encoder - current_cursor_index: %" PRIu32, menu_state->cursor_index);
    
    // Если позиция курсора изменилась, обновляем подсветку
    if (prev_cursor != menu_state->cursor_index) {
        ESP_LOGI(TAG, "Cursor changed from %" PRIu32 " to %" PRIu32, prev_cursor, menu_state->cursor_index);
        io_highlight_box(io_cont, menu_state->cursor_index);
        
        // Управляем подсветкой областей на схеме в зависимости от выбранного пункта меню
        switch(menu_state->cursor_index) {
            case 0: // "Назад"
                ESP_LOGI(TAG, "Highlighting back button");
                screen_In_Out_hide_all_highlights();
                break;
                
            case 1: // "Все"
                ESP_LOGI(TAG, "Highlighting all inputs/outputs");
                screen_In_Out_show_all_highlights();
                break;
                
            case 2: // "Универсальные входы"
                ESP_LOGI(TAG, "Highlighting universal inputs");
                screen_In_Out_show_universal_inputs();
                break;
                
            case 3: // "Аналоговые выходы"
                ESP_LOGI(TAG, "Highlighting analog outputs");
                screen_In_Out_show_analog_outputs();
                break;
                
            case 4: // "Дискретные выходы"
                ESP_LOGI(TAG, "Highlighting discrete outputs");
                screen_In_Out_show_discrete_outputs();
                break;
                
            default:
                ESP_LOGW(TAG, "Unknown menu item: %" PRIu32, menu_state->cursor_index);
                screen_In_Out_hide_all_highlights();
                break;
        }
    }
    
    // Обработка нажатия кнопки
    if (e & ENC_CLICK) {
        ESP_LOGI(TAG, "Click in IO menu on item: %" PRIu32, menu_state->cursor_index);
        
        if (menu_state->cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в главное меню
            ESP_LOGI(TAG, "Returning to main menu from IO menu");
            screen_navigation_go_to(SCREEN_MAIN_MENU);
        }
        // TODO: Добавить обработку нажатий для других пунктов меню при необходимости
    }
}

/**
 * @brief Очистка меню входов/выходов
 */
void input_output_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up IO menu");
    
    io_menu_creation_in_progress = false;
    io_menu_initialized = false;
    
    // Удаляем маску
    if (is_obj_valid(io_mask)) {
        lv_obj_del(io_mask);
        io_mask = NULL;
    }
    
    if (is_obj_valid(io_cont)) {
        lv_obj_del(io_cont);
        io_cont = NULL;
    }
    
    ESP_LOGI(TAG, "IO menu cleanup completed");
}

/**
 * @brief Инициализация меню входов/выходов
 */
void Input_Output_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню входов/выходов");
    
    // ЗАЩИТА ОТ ПОВТОРНОЙ ИНИЦИАЛИЗАЦИИ ВО ВРЕМЯ СОЗДАНИЯ
    if (io_menu_creation_in_progress) {
        ESP_LOGW(TAG, "IO menu creation already in progress, skipping");
        return;
    }
    
    io_menu_creation_in_progress = true;
    
    // ЕСЛИ МЕНЮ УЖЕ ИНИЦИАЛИЗИРОВАНО, ПРОСТО ПОКАЗЫВАЕМ ЕГО
    if (io_menu_initialized && is_obj_valid(io_cont)) {
        ESP_LOGI(TAG, "IO menu already initialized, showing it");
        
        // Показываем контейнер и маску
        if (is_obj_valid(io_cont)) {
            lv_obj_clear_flag(io_cont, LV_OBJ_FLAG_HIDDEN);
        }
        if (is_obj_valid(io_mask)) {
            lv_obj_clear_flag(io_mask, LV_OBJ_FLAG_HIDDEN);
        }
        
        io_menu_creation_in_progress = false;
        return;
    }
    
    // Очищаем предыдущее меню, если было
    input_output_menu_cleanup();
    
    // Инициализируем стиль только один раз (при первом вызове функции)
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    // Создаем контейнер меню (такой же как в главном меню)
    io_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(io_cont)) {
        ESP_LOGE(TAG, "Failed to create IO menu container");
        io_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(io_cont, 1200, 1200);
    lv_obj_center(io_cont);
    lv_obj_add_event_cb(io_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(io_cont, &style, 0);
    lv_obj_set_style_radius(io_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(io_cont, true, 0);
    lv_obj_set_scroll_dir(io_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(io_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(io_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(io_cont, 633, 0);
    lv_obj_set_style_bg_color(io_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(io_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(io_cont, 0, 0); // убираем тени
    lv_obj_set_style_pad_row(io_cont, 0, 0);      // отсутп между боксами
    
    // Создаем элементы меню
    for (uint32_t i = 0; i < sizeof(io_menu_items) / sizeof(IoMenuItem); i++) {
        create_io_menu_item(io_cont, &io_menu_items[i]);
    }
    
    // Создание радиальной маски
    io_mask = radial();
    if (is_obj_valid(io_mask)) {
        lv_obj_set_pos(io_mask, 433, 70);
    }
    
    uint32_t child_count = lv_obj_get_child_cnt(io_cont);
    
    // ИСПОЛЬЗУЕМ КОНФИГУРАЦИЮ И СОСТОЯНИЕ ДЛЯ МЕНЮ ВХОДОВ/ВЫХОДОВ
    const menu_config_t* config = get_menu_config(MENU_TYPE_IN_OUT);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_IN_OUT);
    
    // Инициализируем состояние меню
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    lv_obj_scroll_to_view(lv_obj_get_child(io_cont, menu_state->list_index), LV_ANIM_OFF);
    io_highlight_box(io_cont, menu_state->cursor_index);
    
    // УСТАНАВЛИВАЕМ ФЛАГИ ПОСЛЕ УСПЕШНОЙ ИНИЦИАЛИЗАЦИИ
    io_menu_initialized = true;
    io_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню входов/выходов успешно инициализировано");
    ESP_LOGI(TAG, "Используется конфигурация: initial_index=%lu, scroll_boundary=%lu", 
             config->initial_index, config->scroll_boundary);
    ESP_LOGI(TAG, "Состояние меню: list_index=%lu, cursor_index=%lu", 
             menu_state->list_index, menu_state->cursor_index);
    ESP_LOGI(TAG, "Количество элементов меню: %" PRIu32, child_count);
    
    fflush(NULL);
}
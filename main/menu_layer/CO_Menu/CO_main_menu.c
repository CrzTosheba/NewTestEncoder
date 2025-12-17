#include "CO_main_menu.h"
#include "CO_general_menu.h"
#include "CO_heating_graph_menu.h"
#include "CO_pumps_menu.h"
#include "CO_valve_menu.h"
#include "CO_manual_menu.h"
#include "CO_schedule_menu.h"
#include "CO_alarms_menu.h"
#include "encoder/encoder.h"
#include "my_widgets/w_rad_mask.h"
#include "screen_logic/arc_menu.h"
#include "screen_logic/screen_navigation.h"
#include "screen_logic/screen_container_manager.h"
#include "screen_logic/access_control.h"
#include "encoder/encoder_manager.h"
#include <stdint.h>
#include <inttypes.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CO_MENU";



// Структура элемента меню отопления
typedef struct {
    const char *label_text;     // Текст элемента
    const void *img_src;        // Иконка (только для "Назад")
} CoMenuItem;

// Элементы меню отопления
static const CoMenuItem co_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right},
    {"Общие", NULL},
    {"График отопления", NULL},
    {"Насосы", NULL},
    {"Клапан", NULL},
    {"Ручной режим", NULL},
    {"Расписание", NULL},
    {"Аварии", NULL},
};

// Локальные переменные для меню отопления
lv_obj_t *co_cont = NULL;  // УБИРАЕМ static, делаем глобальной
static bool co_menu_initialized = false;
static bool co_menu_creation_in_progress = false;
static lv_obj_t *co_mask = NULL;  // Добавляем указатель на маску

/**
 * @brief Проверяет, является ли объект валидным
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Создание элемента меню отопления
 */
static void create_co_menu_item(lv_obj_t *cont, const CoMenuItem *item) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_co_menu_item");
        return;
    }
    
    // Создаем контейнер для элемента (высота 40px как в In_Out_menu)
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_co_menu_item");
        return;
    }
    
    lv_obj_set_size(box, 462, 40);
    lv_obj_set_style_border_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_radius(box, 0, 0); // Убираем скругление углов
    
    // Основная надпись
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
    
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);
    
    // Даем время на обработку LVGL
    vTaskDelay(pdMS_TO_TICKS(1));
}

/**
 * @brief Подсветка выбранного элемента меню отопления
 */
static void co_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
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
 * @brief Показывает меню отопления
 */
void co_menu_show(void) {
    ESP_LOGI(TAG, "Showing CO menu");
    if (is_obj_valid(co_cont)) {
        lv_obj_clear_flag(co_cont, LV_OBJ_FLAG_HIDDEN);
    }
    // Пересоздаем маску, если она была удалена
    if (!is_obj_valid(co_mask)) {
        co_mask = radial();
        if (is_obj_valid(co_mask)) {
            lv_obj_set_pos(co_mask, 433, 70);
            ESP_LOGI(TAG, "CO menu mask recreated");
        }
    } else {
        lv_obj_clear_flag(co_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает меню отопления
 */
void co_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding CO menu");
    if (is_obj_valid(co_cont)) {
        lv_obj_add_flag(co_cont, LV_OBJ_FLAG_HIDDEN);
    }
    // ВАЖНО: Удаляем маску при скрытии, чтобы она не перекрывала главный экран
    if (is_obj_valid(co_mask)) {
        lv_obj_del(co_mask);
        co_mask = NULL;
        ESP_LOGI(TAG, "CO menu mask deleted");
    }
}

/**
 * @brief Обработчик событий энкодера для меню отопления
 */
void co_menu_encoder_event_cb(uint8_t e) {
    // Обновляем таймер активности при любом действии пользователя
    access_control_update_activity_timer();
    
    if (!is_obj_valid(co_cont)) {
        ESP_LOGE(TAG, "Контейнер меню отопления не инициализирован");
        return;
    }
    
    // Получаем состояние меню отопления
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    ESP_LOGI(TAG, "CO menu encoder event: 0x%02x, current_cursor_index: %" PRIu32, e, menu_state->cursor_index);
    
    // Используем конфигурацию для меню отопления
    arc_menu_handle_encoder(e, co_cont, menu_state, MENU_TYPE_CO);
    
    ESP_LOGI(TAG, "After arc_menu_handle_encoder - current_cursor_index: %" PRIu32, menu_state->cursor_index);
    
    // Если позиция курсора изменилась, обновляем подсветку
    if (prev_cursor != menu_state->cursor_index) {
        ESP_LOGI(TAG, "Cursor changed from %" PRIu32 " to %" PRIu32, prev_cursor, menu_state->cursor_index);
        co_highlight_box(co_cont, menu_state->cursor_index);
        
        // TODO: Здесь можно добавить логику подсветки соответствующих элементов на схеме отопления
        switch(menu_state->cursor_index) {
            case 0: // "Назад"
                ESP_LOGI(TAG, "Highlighting back button");
                break;
                
            case 1: // "Общие"
                ESP_LOGI(TAG, "Highlighting general settings");
                break;
                
            default:
                ESP_LOGI(TAG, "Selected menu item: %" PRIu32, menu_state->cursor_index);
                break;
        }
    }
    
    // Обработка нажатия кнопки
    if (e & ENC_CLICK) {
        ESP_LOGI(TAG, "Click in CO menu on item: %" PRIu32, menu_state->cursor_index);
        
        if (menu_state->cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в главное меню
            ESP_LOGI(TAG, "Returning to main menu from CO menu");
            screen_navigation_go_to(SCREEN_MAIN_MENU);
        } else if (menu_state->cursor_index == 1) {
            // Нажали на "Общие" - открываем подменю общие
            ESP_LOGI(TAG, "Opening general settings menu");
            co_menu_hide();
            CO_General_Menu_List();
            co_general_menu_show();
            // Переключаем обработчик энкодера на меню общие
            encoder_manager_register_callback(co_general_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 2) {
            // Нажали на "График отопления" - открываем подменю графика отопления
            ESP_LOGI(TAG, "Opening heating graph menu");
            co_menu_hide();
            CO_Heating_Graph_Menu_List();
            co_heating_graph_menu_show();
            // Переключаем обработчик энкодера на меню графика отопления
            encoder_manager_register_callback(co_heating_graph_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 3) {
            // Нажали на "Насосы" - открываем подменю насосов
            ESP_LOGI(TAG, "Opening pumps menu");
            co_menu_hide();
            CO_Pumps_Menu_List();
            co_pumps_menu_show();
            // Переключаем обработчик энкодера на меню насосов
            encoder_manager_register_callback(co_pumps_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 4) {
            // Нажали на "Клапан" - открываем подменю клапан
            ESP_LOGI(TAG, "Opening valve menu");
            co_menu_hide();
            CO_Valve_Menu_List();
            co_valve_menu_show();
            // Переключаем обработчик энкодера на меню клапан
            encoder_manager_register_callback(co_valve_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 5) {
            // Нажали на "Ручной режим" - открываем подменю ручной режим
            ESP_LOGI(TAG, "Opening manual menu");
            co_menu_hide();
            CO_Manual_Menu_List();
            co_manual_menu_show();
            // Переключаем обработчик энкодера на меню ручной режим
            encoder_manager_register_callback(co_manual_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 6) {
            // Нажали на "Расписание" - открываем подменю расписания
            ESP_LOGI(TAG, "Opening schedule menu");
            co_menu_hide();
            CO_Schedule_Menu_List();
            co_schedule_menu_show();
            // Переключаем обработчик энкодера на меню расписания
            encoder_manager_register_callback(co_schedule_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 7) {
            // Нажали на "Аварии" - открываем подменю аварий
            ESP_LOGI(TAG, "Opening alarms menu");
            co_menu_hide();
            CO_Alarms_Menu_List();
            co_alarms_menu_show();
            // Переключаем обработчик энкодера на меню аварий
            encoder_manager_register_callback(co_alarms_menu_encoder_event_cb);
        }
    }
}

/**
 * @brief Очистка меню отопления
 */
void co_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up CO menu");
    
    co_menu_creation_in_progress = false;
    
    // Удаляем маску
    if (is_obj_valid(co_mask)) {
        lv_obj_del(co_mask);
        co_mask = NULL;
    }
    
    if (is_obj_valid(co_cont)) {
        lv_obj_del(co_cont);
        co_cont = NULL;
    }
    
    co_menu_initialized = false;
}

/**
 * @brief Инициализация меню отопления
 */
void CO_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню отопления");
    
    // Защита от повторной инициализации во время создания
    if (co_menu_creation_in_progress) {
        ESP_LOGW(TAG, "CO menu creation already in progress, skipping");
        return;
    }
    
    co_menu_creation_in_progress = true;
    
    // Если меню уже инициализировано, просто показываем его
    if (co_menu_initialized && is_obj_valid(co_cont)) {
        ESP_LOGI(TAG, "CO menu already initialized, showing it");
        co_menu_show();
        co_menu_creation_in_progress = false;
        return;
    }
    
    // Очищаем предыдущее меню, если было
    co_menu_cleanup();
    
    // Инициализируем стиль только один раз (при первом вызове функции)
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    // Создаем контейнер меню
    co_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(co_cont)) {
        ESP_LOGE(TAG, "Failed to create CO menu container");
        co_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(co_cont, 1200, 1200);
    lv_obj_center(co_cont);
    lv_obj_add_event_cb(co_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(co_cont, &style, 0);
    lv_obj_set_style_radius(co_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(co_cont, true, 0);
    lv_obj_set_scroll_dir(co_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(co_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(co_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(co_cont, 633, 0);
    lv_obj_set_style_bg_color(co_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(co_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(co_cont, 0, 0);
    lv_obj_set_style_pad_row(co_cont, 1, 0);
    
    // Даем время на обработку LVGL после создания контейнера
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Создаем элементы меню с задержками между ними
    for (uint32_t i = 0; i < sizeof(co_menu_items) / sizeof(CoMenuItem); i++) {
        create_co_menu_item(co_cont, &co_menu_items[i]);
        
        // Даем больше времени на обработку после каждых нескольких элементов
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    // Даем время на обработку LVGL после создания всех элементов
    vTaskDelay(pdMS_TO_TICKS(20));
    
    // Создание радиальной маски
        co_mask = radial();
    if (is_obj_valid(co_mask)) {
        lv_obj_set_pos(co_mask, 433, 70);
    }
    
    uint32_t child_count = lv_obj_get_child_cnt(co_cont);
    
    // ИСПОЛЬЗУЕМ КОНФИГУРАЦИЮ И СОСТОЯНИЕ ДЛЯ МЕНЮ ОТОПЛЕНИЯ
    const menu_config_t* config = get_menu_config(MENU_TYPE_CO);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_CO);
    
    // Инициализируем состояние меню
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    // Прокручиваем к нужному элементу из конфигурации
    lv_obj_t *target_child = lv_obj_get_child(co_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    // Применяем подсветку к начальному элементу
    co_highlight_box(co_cont, menu_state->cursor_index);
    
    // Обновляем дуговое меню
    arc_menu_update_slide(co_cont);
    
    co_menu_initialized = true;
    co_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню отопления успешно инициализировано");
    ESP_LOGI(TAG, "Используется конфигурация: initial_index=%lu, scroll_boundary=%lu", 
             config->initial_index, config->scroll_boundary);
    ESP_LOGI(TAG, "Состояние меню: list_index=%lu, cursor_index=%lu", 
             menu_state->list_index, menu_state->cursor_index);
    ESP_LOGI(TAG, "Количество элементов меню: %" PRIu32, child_count);
    
    fflush(NULL);
}
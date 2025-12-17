#include "GVS_main_menu.h"
#include "GVS_general_menu.h"
#include "GVS_pumps_menu.h"
#include "GVS_valve_menu.h"
#include "GVS_manual_menu.h"
#include "GVS_schedule_menu.h"
#include "GVS_alarms_menu.h"
#include "encoder/encoder.h"
#include "my_widgets/w_rad_mask.h"
#include "screen_logic/arc_menu.h"
#include "screen_logic/screen_navigation.h"
#include "screen_logic/screen_container_manager.h"
#include "screen_logic/access_control.h"
#include "screen_logic/menu_config.h"
#include "encoder/encoder_manager.h"
#include <stdint.h>
#include <inttypes.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "GVS_MENU";

// Структура элемента меню ГВС
typedef struct {
    const char *label_text;     // Текст элемента
    const void *img_src;        // Иконка (только для "Назад")
} GvsMenuItem;

// Элементы меню ГВС
static const GvsMenuItem gvs_menu_items[] = {
    {"                                Назад", &lv_im_arrow_right},
    {"Общие", NULL},
    {"Насосы", NULL},
    {"Клапан", NULL},
    {"Ручной режим", NULL},
    {"Расписание", NULL},
    {"Аварии", NULL},
};

// Локальные переменные для меню ГВС
lv_obj_t *gvs_cont = NULL;  // Глобальная переменная для доступа из других файлов
static bool gvs_menu_initialized = false;
static bool gvs_menu_creation_in_progress = false;
static lv_obj_t *gvs_mask = NULL;  // Добавляем указатель на маску

/**
 * @brief Проверяет, является ли объект валидным
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Создание элемента меню ГВС
 */
static void create_gvs_menu_item(lv_obj_t *cont, const GvsMenuItem *item) {
    if (!is_obj_valid(cont)) {
        ESP_LOGE(TAG, "Invalid container in create_gvs_menu_item");
        return;
    }
    
    // Создаем контейнер для элемента (высота 40px как в In_Out_menu)
    lv_obj_t *box = lv_obj_create(cont);
    if (!is_obj_valid(box)) {
        ESP_LOGE(TAG, "Failed to create box in create_gvs_menu_item");
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
 * @brief Подсветка выбранного элемента меню ГВС
 */
static void gvs_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
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
 * @brief Показывает меню ГВС
 */
void gvs_menu_show(void) {
    ESP_LOGI(TAG, "Showing GVS menu");
    if (is_obj_valid(gvs_cont)) {
        lv_obj_clear_flag(gvs_cont, LV_OBJ_FLAG_HIDDEN);
    }
    // Пересоздаем маску, если она была удалена
    if (!is_obj_valid(gvs_mask)) {
        gvs_mask = radial();
        if (is_obj_valid(gvs_mask)) {
            lv_obj_set_pos(gvs_mask, 433, 70);
            ESP_LOGI(TAG, "GVS menu mask recreated");
        }
    } else {
        lv_obj_clear_flag(gvs_mask, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Скрывает меню ГВС
 */
void gvs_menu_hide(void) {
    ESP_LOGI(TAG, "Hiding GVS menu");
    if (is_obj_valid(gvs_cont)) {
        lv_obj_add_flag(gvs_cont, LV_OBJ_FLAG_HIDDEN);
    }
    // ВАЖНО: Удаляем маску при скрытии, чтобы она не перекрывала главный экран
    if (is_obj_valid(gvs_mask)) {
        lv_obj_del(gvs_mask);
        gvs_mask = NULL;
        ESP_LOGI(TAG, "GVS menu mask deleted");
    }
}

/**
 * @brief Обработчик событий энкодера для меню ГВС
 */
void gvs_menu_encoder_event_cb(uint8_t e) {
    // Обновляем таймер активности при любом действии пользователя
    access_control_update_activity_timer();
    
    if (!is_obj_valid(gvs_cont)) {
        ESP_LOGE(TAG, "Контейнер меню ГВС не инициализирован");
        return;
    }
    
    // Получаем состояние меню ГВС
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS);
    uint32_t prev_cursor = menu_state->cursor_index;
    
    ESP_LOGI(TAG, "GVS menu encoder event: 0x%02x, current_cursor_index: %" PRIu32, e, menu_state->cursor_index);
    
    // Используем конфигурацию для меню ГВС
    arc_menu_handle_encoder(e, gvs_cont, menu_state, MENU_TYPE_GVS);
    
    ESP_LOGI(TAG, "After arc_menu_handle_encoder - current_cursor_index: %" PRIu32, menu_state->cursor_index);
    
    // Если позиция курсора изменилась, обновляем подсветку
    if (prev_cursor != menu_state->cursor_index) {
        ESP_LOGI(TAG, "Cursor changed from %" PRIu32 " to %" PRIu32, prev_cursor, menu_state->cursor_index);
        gvs_highlight_box(gvs_cont, menu_state->cursor_index);
        
        // TODO: Здесь можно добавить логику подсветки соответствующих элементов на схеме ГВС
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
        ESP_LOGI(TAG, "Click in GVS menu on item: %" PRIu32, menu_state->cursor_index);
        
        if (menu_state->cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в главное меню
            ESP_LOGI(TAG, "Returning to main menu from GVS menu");
            screen_navigation_go_to(SCREEN_MAIN_MENU);
        } else if (menu_state->cursor_index == 1) {
            // Нажали на "Общие" - открываем подменю общие
            ESP_LOGI(TAG, "Opening general settings menu");
            gvs_menu_hide();
            GVS_General_Menu_List();
            gvs_general_menu_show();
            // Переключаем обработчик энкодера на меню общие
            encoder_manager_register_callback(gvs_general_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 2) {
            // Нажали на "Насосы" - открываем подменю насосов
            ESP_LOGI(TAG, "Opening GVS pumps menu");
            gvs_menu_hide();
            GVS_Pumps_Menu_List();
            gvs_pumps_menu_show();
            // Переключаем обработчик энкодера на меню насосов
            encoder_manager_register_callback(gvs_pumps_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 3) {
            // Нажали на "Клапан" - открываем подменю клапан
            ESP_LOGI(TAG, "Opening GVS valve menu");
            gvs_menu_hide();
            GVS_Valve_Menu_List();
            gvs_valve_menu_show();
            // Переключаем обработчик энкодера на меню клапан
            encoder_manager_register_callback(gvs_valve_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 4) {
            // Нажали на "Ручной режим" - открываем подменю ручной режим
            ESP_LOGI(TAG, "Opening GVS manual menu");
            gvs_menu_hide();
            GVS_Manual_Menu_List();
            gvs_manual_menu_show();
            // Переключаем обработчик энкодера на меню ручной режим
            encoder_manager_register_callback(gvs_manual_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 5) {
            // Нажали на "Расписание" - открываем подменю расписания
            ESP_LOGI(TAG, "Opening GVS schedule menu");
            gvs_menu_hide();
            GVS_Schedule_Menu_List();
            gvs_schedule_menu_show();
            // Переключаем обработчик энкодера на меню расписания
            encoder_manager_register_callback(gvs_schedule_menu_encoder_event_cb);
        } else if (menu_state->cursor_index == 6) {
            // Нажали на "Аварии" - открываем подменю аварий
            ESP_LOGI(TAG, "Opening GVS alarms menu");
            gvs_menu_hide();
            GVS_Alarms_Menu_List();
            gvs_alarms_menu_show();
            // Переключаем обработчик энкодера на меню аварий
            encoder_manager_register_callback(gvs_alarms_menu_encoder_event_cb);
        }
    }
}

/**
 * @brief Очистка меню ГВС
 */
void gvs_menu_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up GVS menu");
    
    gvs_menu_creation_in_progress = false;
    
    // Удаляем маску
    if (is_obj_valid(gvs_mask)) {
        lv_obj_del(gvs_mask);
        gvs_mask = NULL;
    }
    
    if (is_obj_valid(gvs_cont)) {
        lv_obj_del(gvs_cont);
        gvs_cont = NULL;
    }
    
    gvs_menu_initialized = false;
}

/**
 * @brief Инициализация меню ГВС
 */
void GVS_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню ГВС");
    
    // Защита от повторной инициализации во время создания
    if (gvs_menu_creation_in_progress) {
        ESP_LOGW(TAG, "GVS menu creation already in progress, skipping");
        return;
    }
    
    gvs_menu_creation_in_progress = true;
    
    // Если меню уже инициализировано, просто показываем его
    if (gvs_menu_initialized && is_obj_valid(gvs_cont)) {
        ESP_LOGI(TAG, "GVS menu already initialized, showing it");
        gvs_menu_show();
        gvs_menu_creation_in_progress = false;
        return;
    }
    
    // Очищаем предыдущее меню, если было
    gvs_menu_cleanup();
    
    // Инициализируем стиль только один раз (при первом вызове функции)
    static lv_style_t style;
    static bool style_inited = false;
    if (!style_inited) {
        lv_style_init(&style);
        style_inited = true;
    }

    // Создаем контейнер меню
    gvs_cont = lv_obj_create(lv_scr_act());
    if (!is_obj_valid(gvs_cont)) {
        ESP_LOGE(TAG, "Failed to create GVS menu container");
        gvs_menu_creation_in_progress = false;
        return;
    }
    
    lv_obj_set_size(gvs_cont, 1200, 1200);
    lv_obj_center(gvs_cont);
    lv_obj_add_event_cb(gvs_cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(gvs_cont, &style, 0);
    lv_obj_set_style_radius(gvs_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(gvs_cont, true, 0);
    lv_obj_set_scroll_dir(gvs_cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(gvs_cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(gvs_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(gvs_cont, 633, 0);
    lv_obj_set_style_bg_color(gvs_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(gvs_cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_shadow_width(gvs_cont, 0, 0);
    lv_obj_set_style_pad_row(gvs_cont, 1, 0);
    
    // Даем время на обработку LVGL после создания контейнера
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Создаем элементы меню с задержками между ними
    for (uint32_t i = 0; i < sizeof(gvs_menu_items) / sizeof(GvsMenuItem); i++) {
        create_gvs_menu_item(gvs_cont, &gvs_menu_items[i]);
        
        // Даем больше времени на обработку после каждых нескольких элементов
        if (i % 3 == 0) {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    
    // Даем время на обработку LVGL после создания всех элементов
    vTaskDelay(pdMS_TO_TICKS(20));
    
    // Создание радиальной маски
    gvs_mask = radial();
    if (is_obj_valid(gvs_mask)) {
        lv_obj_set_pos(gvs_mask, 433, 70);
    }
    
    uint32_t child_count = lv_obj_get_child_cnt(gvs_cont);
    
    // ИСПОЛЬЗУЕМ КОНФИГУРАЦИЮ И СОСТОЯНИЕ ДЛЯ МЕНЮ ГВС
    const menu_config_t* config = get_menu_config(MENU_TYPE_GVS);
    menu_state_t *menu_state = get_menu_state(MENU_TYPE_GVS);
    
    // Инициализируем состояние меню
    menu_state->list_index = config->initial_index;
    menu_state->cursor_index = 0;
    
    // Прокручиваем к нужному элементу из конфигурации
    lv_obj_t *target_child = lv_obj_get_child(gvs_cont, menu_state->list_index);
    if (target_child) {
        lv_obj_scroll_to_view(target_child, LV_ANIM_OFF);
    }
    
    // Применяем подсветку к начальному элементу
    gvs_highlight_box(gvs_cont, menu_state->cursor_index);
    
    // Обновляем дуговое меню
    arc_menu_update_slide(gvs_cont);
    
    gvs_menu_initialized = true;
    gvs_menu_creation_in_progress = false;

    ESP_LOGI(TAG, "Меню ГВС успешно инициализировано");
    ESP_LOGI(TAG, "Используется конфигурация: initial_index=%lu, scroll_boundary=%lu", 
             config->initial_index, config->scroll_boundary);
    ESP_LOGI(TAG, "Состояние меню: list_index=%lu, cursor_index=%lu", 
             menu_state->list_index, menu_state->cursor_index);
    ESP_LOGI(TAG, "Количество элементов меню: %" PRIu32, child_count);
    
    fflush(NULL);
}


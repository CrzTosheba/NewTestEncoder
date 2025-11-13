#include "In_Out_main_menu.h"
#include "encoder/encoder.h"
#include "my_widgets/w_rad_mask.h"
#include "screen_logic/arc_menu.h"
#include "encoder/encoder_manager.h"
#include "screen_logic/screen_navigation.h"
#include <stdint.h>
#include <inttypes.h>
#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "IO_MENU";

// Используем те же глобальные переменные, что и в главном меню
extern lv_obj_t *_cont;
extern uint32_t current_index;
extern uint32_t current_cursor_index;
extern void (*current_function)(void);

// Структура элемента меню входов/выходов
typedef struct {
    const char *label_text;     // Текст элемента
    const void *img_src;        // Иконка (только для "Назад")
} IoMenuItem;

// Элементы меню входов/выходов
static const IoMenuItem io_menu_items[] = {
    {"                                  Назад", &lv_im_arrow_right},
    {"Все", NULL},
    {"Универсальные входы", NULL},
    {"Аналоговые выходы", NULL},
    {"Дискретные входы", NULL},
};

// Локальные переменные для меню входов/выходов
static lv_obj_t *io_cont = NULL;
static uint32_t io_current_index = 0;
static uint32_t io_current_cursor_index = 0;

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
      lv_obj_align(img, LV_ALIGN_CENTER, 100, 0);
    }
    
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);
}

/**
 * @brief Подсветка выбранного элемента меню входов/выходов
 */
static void io_highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        uint32_t grand_child_cnt = lv_obj_get_child_cnt(child);
        
        // Обрабатываем все дочерние элементы контейнера
        for (uint32_t j = 0; j < grand_child_cnt; j++) {
            lv_obj_t *grand_child = lv_obj_get_child(child, j);
            
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
 * @brief Обработчик движений энкодера для меню входов/выходов
 */
static void io_handle_encoder_movement(uint8_t e, lv_obj_t *cont, uint32_t *current_idx) {
    static uint32_t last_event_time = 0;
    static uint8_t last_event = 0;
    const uint32_t DEBOUNCE_TIME_MS = 50;
    
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t child_count = lv_obj_get_child_cnt(cont);
    
    // Фильтр дребезга
    if ((e == last_event) && (now - last_event_time < DEBOUNCE_TIME_MS)) {
        return;
    }
    
    // Обработка движения ВВЕРХ (против часовой стрелки)
    if (e & ENC_RIGHT) {
        if (io_current_cursor_index > 0) {
            io_current_cursor_index--;  // Уменьшаем индекс курсора
            
            // Сдвигаем список, если курсор вышел за верхнюю границу видимой области
            if (io_current_cursor_index < *current_idx - 2) {
                if (*current_idx > 2) {
                    (*current_idx)--;
                }
            }
        }
        last_event = ENC_RIGHT;
        last_event_time = now;
    } 
    // Обработка движения ВНИЗ (по часовой стрелке)
    else if (e & ENC_LEFT) {
        if (io_current_cursor_index < child_count - 1) {
            io_current_cursor_index++;  // Увеличиваем индекс курсора
            
            // Сдвигаем список, когда курсор достигает последнего видимого элемента
            if (io_current_cursor_index > *current_idx + 2) {
                if (*current_idx < child_count - 3) {
                    (*current_idx)++;
                }
            }
        }
        last_event = ENC_LEFT;
        last_event_time = now;
    }
    
    // Обновление позиции после обработки движения
    if (e & (ENC_LEFT | ENC_RIGHT)) {
        // Прокручиваем к текущему элементу списка
        lv_obj_t *target = lv_obj_get_child(cont, *current_idx);
        if (target) {
            lv_obj_scroll_to_view(target, LV_ANIM_ON);
        }
        
        // Обновляем дуговое меню
        arc_menu_update_slide(cont);
        
        // Синхронизируем глобальную переменную курсора
        current_cursor_index = io_current_cursor_index;
    }
}

/**
 * @brief Обработчик событий энкодера для меню входов/выходов
 */
void input_output_encoder_event_cb(uint8_t e) {
    if (io_cont == NULL) {
        ESP_LOGE(TAG, "Контейнер меню входов/выходов не инициализирован");
        return;
    }
    
    uint32_t prev_cursor = io_current_cursor_index;
    
    // Используем нашу собственную функцию для обработки движений
    io_handle_encoder_movement(e, io_cont, &io_current_index);
    
    if (prev_cursor != io_current_cursor_index) {
        io_highlight_box(io_cont, io_current_cursor_index);
    }
    
    // Обработка нажатия кнопки
    if (e & ENC_CLICK) {
        ESP_LOGI(TAG, "Click in IO menu on item: %" PRIu32, io_current_cursor_index);
        
        if (io_current_cursor_index == 0) {
            // Нажали на "Назад" - возвращаемся в главное меню
            ESP_LOGI(TAG, "Returning to main menu from IO menu");
            input_output_menu_cleanup();
            screen_navigation_go_to(SCREEN_MAIN_MENU);
        }
    }
}

/**
 * @brief Очистка меню входов/выходов
 */
void input_output_menu_cleanup(void) {
    if (io_cont && lv_obj_is_valid(io_cont)) {
        lv_obj_del(io_cont);
        io_cont = NULL;
    }
    io_current_index = 0;
    io_current_cursor_index = 0;
}

/**
 * @brief Инициализация меню входов/выходов
 */
void Input_Output_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация меню входов/выходов");
    
    // Очищаем предыдущее меню, если было
    input_output_menu_cleanup();
    
    static lv_style_t style;
    lv_style_init(&style);

    // Создаем контейнер меню (такой же как в главном меню)
    io_cont = lv_obj_create(lv_scr_act());
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
    lv_obj_set_style_pad_row(io_cont, 0, 0);      //отсутп между боксами
    
    
    // Создаем элементы меню
    for (uint32_t i = 0; i < sizeof(io_menu_items) / sizeof(IoMenuItem); i++) {
        create_io_menu_item(io_cont, &io_menu_items[i]);
    }
    
    // Создание радиальной маски (как в главном меню)
    lv_obj_t *mask = radial();
    lv_obj_set_pos(mask, 433, 70);
    
    uint32_t child_count = lv_obj_get_child_cnt(io_cont);
    io_current_index = (child_count > 3) ? 2 : 0;
    io_current_cursor_index = 0;
    
    // Синхронизируем глобальные переменные
    current_index = io_current_index;
    current_cursor_index = io_current_cursor_index;
    
    lv_obj_scroll_to_view(lv_obj_get_child(io_cont, io_current_index), LV_ANIM_OFF);
    io_highlight_box(io_cont, io_current_cursor_index);

    ESP_LOGI(TAG, "Меню входов/выходов успешно инициализировано");
    fflush(NULL);
}
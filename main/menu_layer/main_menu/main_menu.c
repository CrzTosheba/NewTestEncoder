// Файл: main_menu.c
#include "main_menu.h"
#include "encoder/encoder.h"
#include "my_widgets/rad_mask.h"
#include "screen_logic/arc_menu.h"
#include <stdint.h>
#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "Main_Menu_main";

// Структура элемента меню
typedef struct {
    const char *label_text;       // Основной текст
    const char *sensor_text;      // Дополнительный текст
    const void *img_src;          // Основная иконка
    const void *status_img_src;   // Иконка статуса
} MenuItem;

// Глобальные переменные
lv_obj_t *_cont = NULL;          // Контейнер меню
uint32_t current_index = 0;       // Текущий индекс списка
uint32_t current_selected_point = 0; // Текущая выбранная точка
void (*current_function)(void) = NULL; // Текущая функция меню

// Используем глобальную переменную курсора из arc_menu
extern uint32_t current_cursor_index;

// Функция подсветки выбранного элемента
static void highlight_box(lv_obj_t *cont, uint32_t cursor_index) {
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        
        // Находим дочерние элементы (предполагаем фиксированный порядок)
        lv_obj_t *label = lv_obj_get_child(child, 0);        // Основной текст
        lv_obj_t *sensor = lv_obj_get_child(child, 1);       // Дополнительный текст
        lv_obj_t *img = lv_obj_get_child(child, 2);          // Основная иконка
        lv_obj_t *status_img = NULL;
        
        // Проверяем наличие статусной иконки (4-й элемент, если есть)
        if (lv_obj_get_child_cnt(child) > 3) {
            status_img = lv_obj_get_child(child, 3);
        }
        
        if (i == cursor_index) {
            // Подсветка выбранного элемента
            lv_obj_set_style_bg_color(child, lv_color_hex(0xFFCC00), LV_PART_MAIN); // Желтый фон
            if (label) lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN); // Черный текст
            if (sensor) lv_obj_set_style_text_color(sensor, lv_color_hex(0x000000), LV_PART_MAIN); // Черный текст
            
            // Основную иконку окрашиваем в черный
            if (img) {
                lv_obj_set_style_img_recolor(img, lv_color_black(), 0);
                lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
            }
        } else {
            // Стиль для невыбранных элементов
            lv_obj_set_style_bg_color(child, lv_color_hex(0x2B3639), LV_PART_MAIN); // Темный фон
            if (label) lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // Белый текст
            if (sensor) lv_obj_set_style_text_color(sensor, lv_color_hex(0xFFFFFF), LV_PART_MAIN); // Белый текст
            
            // Сбрасываем окрашивание иконки
            if (img) lv_obj_set_style_img_recolor_opa(img, LV_OPA_TRANSP, 0);
        }
        
        // Статусная иконка никогда не меняет цвет
        if (status_img) {
            lv_obj_set_style_img_recolor_opa(status_img, LV_OPA_TRANSP, 0);
        }
    }
}

// Обработчик событий энкодера
void main_menu_encoder_event_cb(uint8_t e) {
    if (_cont == NULL) {
        ESP_LOGE(TAG, "Контейнер меню не инициализирован");
        return;
    }
    
    // Сохраняем предыдущее положение курсора
    uint32_t prev_cursor = current_cursor_index;
    
    // Обрабатываем движение энкодера
    arc_menu_handle_encoder(e, _cont, &current_index);
    
    // Обновляем подсветку, если позиция курсора изменилась
    if (prev_cursor != current_cursor_index) {
        highlight_box(_cont, current_cursor_index);
    }
}

// Элементы меню
static const MenuItem menu_items[] = {
    {"Открыть доступ", "", &lv_im_module_lock, ""},                // Элемент 0
    {"ГВС", "40.6(56.5)°C", &lv_im_module_hotwater, &lv_im_module_on},  // Элемент 1
    {"Отопление", "40.6(56.5)°C", &lv_im_module_heat, &lv_im_module_on}, // Элемент 2
    {"Подпитка", "Н1", &lv_im_module_podp, &lv_im_module_off},     // Элемент 3
    {"Узел ввода", "71°C|-13°C", &lv_im_module_input_output, &lv_im_module_on}, // Элемент 4
    {"Входы/выходы", "", &lv_im_module_inout, ""},                 // Элемент 5
};

// Создание элемента меню
static void create_menu_item(lv_obj_t *cont, const MenuItem *item) {
    // Создаем контейнер для элемента
    lv_obj_t *box = lv_obj_create(cont);
    lv_obj_set_size(box, 480, 70);
    lv_obj_set_style_border_color(box, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_color(box, lv_color_hex(0x2B3639), 0);
    
    // Основная надпись
    lv_obj_t *label = lv_label_create(box);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
    lv_label_set_text(label, item->label_text);
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 75, -10);
    
    // Дополнительная надпись (сенсор)
    lv_obj_t *sensor = lv_label_create(box);
    lv_obj_set_style_text_color(sensor, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(sensor, &Roboto_bold_18, 0);
    lv_label_set_text(sensor, item->sensor_text);
    lv_obj_align(sensor, LV_ALIGN_BOTTOM_LEFT, 75, 10);
    
    // Основная иконка
    lv_obj_t *img = lv_img_create(box);
    lv_img_set_src(img, item->img_src);
    lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 10, 15);
    
    // Статусная иконка (если есть)
    if (item->status_img_src != NULL && ((const char*)item->status_img_src)[0] != '\0') {
        lv_obj_t *status_img = lv_img_create(box);
        lv_img_set_src(status_img, item->status_img_src);
        lv_obj_align(status_img, LV_ALIGN_BOTTOM_MID, 83, 20);
    }
    
    lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);
}

// Инициализация главного меню
void Main_Menu_List(void) {
    ESP_LOGI(TAG, "Инициализация главного меню");
    
    // Инициализация стилей
    static lv_style_t style;
    lv_style_init(&style);
    
    // Регистрация обработчика энкодера
    enc_register_event(main_menu_encoder_event_cb);
    
    // Создание контейнера меню
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 1200, 1200);
    lv_obj_center(cont);
    lv_obj_add_event_cb(cont, arc_menu_event_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_style(cont, &style, 0);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(cont, true, 0);
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_pos(cont, 640, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x1E2528), 0);
    lv_obj_set_style_border_color(cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_pad_row(cont, 1, 0);
    
    // Создание элементов меню
    for (uint32_t i = 0; i < sizeof(menu_items) / sizeof(MenuItem); i++) {
        create_menu_item(cont, &menu_items[i]);
    }
    
    // Создание радиальной маски
    lv_obj_t *mask = radial();
    lv_obj_set_pos(mask, 440, 70);
    
    // Сохранение глобальных ссылок
    _cont = cont;
   // Рассчитываем начальный индекс списка
    uint32_t child_count = lv_obj_get_child_cnt(cont);
    current_index = (child_count > 3) ? 2 : 0; // Стартуем с индекса 2, если элементов достаточно
    
    // Курсор начинаем с 0
    current_cursor_index = 0;
    
    // Прокрутка к начальной позиции и подсветка
    lv_obj_scroll_to_view(lv_obj_get_child(cont, current_index), LV_ANIM_OFF);
    highlight_box(cont, current_cursor_index);
    current_function = Main_Menu_List;
    
    ESP_LOGI(TAG, "Главное меню успешно инициализировано");
    fflush(NULL);
}
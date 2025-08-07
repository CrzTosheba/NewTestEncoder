#include "yes_no_screen.h"
#include "esp_log.h"
#include "my_widgets/rad_mask.h"
//#include "roulette_but/menu_common.h"
#include "driver/gpio.h"

// Теги для логирования
static const char *TAG = "YES_NO_SCREEN";
LV_FONT_DECLARE(Roboto_bold_24);

// Глобальные объекты LVGL
lv_obj_t *yes_btn = NULL;
lv_obj_t *no_btn = NULL;
lv_obj_t *confirm_win = NULL;
lv_obj_t *dim_layer = NULL;  // Для затемняющего слоя

// Текстовые метки
static lv_obj_t *yes_label = NULL;
static lv_obj_t *no_label = NULL;

// Состояния
bool confirmation_active = false;
bool selected_yes = true;

// Обновление стилей кнопок при изменении выбора
void update_buttons_style() {
    // Стиль для активной кнопки ДА
    lv_obj_set_style_bg_color(yes_btn, selected_yes ? lv_color_hex(0xFFCC00) : lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_text_color(yes_label, selected_yes ? lv_color_hex(0x000000) : lv_color_hex(0xFFFFFF), 0);
    
    // Стиль для активной кнопки НЕТ
    lv_obj_set_style_bg_color(no_btn, !selected_yes ? lv_color_hex(0xFFCC00) : lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_text_color(no_label, !selected_yes ? lv_color_hex(0x000000) : lv_color_hex(0xFFFFFF), 0);
    
    // Принудительное обновление виджетов
    lv_obj_invalidate(yes_btn);
    lv_obj_invalidate(no_btn);
    ESP_LOGI(TAG, "Кнопки обновлены: %s", selected_yes ? "ДА" : "НЕТ");
}

// Закрытие окна подтверждения
void close_yes_no_screen() {
    if (confirm_win) {
        lv_obj_del(confirm_win);
        confirm_win = NULL;
        yes_btn = NULL;     
        no_btn = NULL;      
        yes_label = NULL;  
        no_label = NULL;
         // Удаляем слой затемнения    
        if(dim_layer) {
            lv_obj_del(dim_layer);
            dim_layer = NULL;
        }
        confirmation_active = false;
    }
}

// Создание окна подтверждения
void create_yes_no_screen() {
    if (confirmation_active) {
        ESP_LOGW(TAG, "Окно уже активно!");
        return;
    }
    
    confirmation_active = true;
    selected_yes = true;  // Сброс выбора к ДА
    
        // Создаем слой затемнения
    dim_layer = lv_obj_create(lv_layer_sys());
    lv_obj_set_size(dim_layer, LV_PCT(120), LV_PCT(100));
    lv_obj_align(dim_layer, LV_ALIGN_CENTER, -5, 0);
    lv_obj_set_style_bg_color(dim_layer, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(dim_layer, LV_OPA_50, 0);
    lv_obj_add_flag(dim_layer, LV_OBJ_FLAG_CLICKABLE); // Блокируем клики на фоне
    // Создание основного контейнера
    confirm_win = lv_obj_create(lv_layer_sys());
    lv_obj_set_size(confirm_win, 400, 160);
    lv_obj_align(confirm_win, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(confirm_win, lv_color_hex(0xE9EBEB), LV_PART_MAIN);
    lv_obj_set_style_radius(confirm_win, 0, LV_PART_MAIN);
    
    // Заголовок окна
    lv_obj_t *header_label = lv_label_create(confirm_win);
    lv_label_set_text(header_label, "Сохранить изменения?");
    lv_obj_align(header_label, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_font(header_label, &Roboto_bold_24, 0);
    
    // Футер с кнопками
    lv_obj_t *footer = lv_obj_create(confirm_win);
    lv_obj_set_size(footer, 400, 60);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, 20);
    lv_obj_set_style_bg_color(footer, lv_color_hex(0x2B3639), 0);
    lv_obj_set_scrollbar_mode(confirm_win, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_color(footer,lv_color_hex(0x2B3639), 0);
    lv_obj_set_style_radius(footer, 0, LV_PART_MAIN);
    
    // Кнопка ДА
    yes_btn = lv_btn_create(footer);
    lv_obj_set_size(yes_btn, 180, 40); 
    lv_obj_align(yes_btn, LV_ALIGN_BOTTOM_LEFT, -10, 10);
    lv_obj_set_style_radius(yes_btn, 6, LV_PART_MAIN);
     //надпись
    yes_label = lv_label_create(yes_btn);
    lv_label_set_text(yes_label, "ДА");
    lv_obj_set_style_text_font(yes_label, &Roboto_bold_24, 0);
    lv_obj_center(yes_label);
    
    // Кнопка НЕТ
    no_btn = lv_btn_create(footer);
    lv_obj_set_size(no_btn, 180, 40);
    lv_obj_align(no_btn, LV_ALIGN_BOTTOM_RIGHT, 10, 10);
    lv_obj_set_style_radius(no_btn, 6, LV_PART_MAIN);
    // тоже надпись
    no_label = lv_label_create(no_btn);
    lv_label_set_text(no_label, "НЕТ");
    lv_obj_set_style_text_font(no_label, &Roboto_bold_24, 0);
    lv_obj_center(no_label);
    lv_obj_set_scrollbar_mode(footer, LV_SCROLLBAR_MODE_OFF);
        // убираем тени и размер бордеров, что бы жить не мешали
        lv_obj_set_style_shadow_width(yes_btn, 0, 0);
        lv_obj_set_style_shadow_width(no_btn, 0, 0);
        lv_obj_set_style_shadow_width(footer, 0, 0);
        lv_obj_set_style_border_width(footer, 0, 0);
        lv_obj_set_style_border_width(confirm_win, 0, 0);
    
    // Начальное обновление стилей
    update_buttons_style();
    // Убедимся что окно поверх затемнения
    lv_obj_move_foreground(confirm_win);
    ESP_LOGI(TAG, "Окно подтверждения создано");
}

// Обработчик событий энкодера (основная логика)
void yes_no_menu_encoder_event_cb(uint8_t e) {
    // Если окно активно - обрабатываем выбор
    if (confirmation_active) {
        // Вращение энкодера
        if (e & ENC_LEFT) {
            selected_yes = true;
            update_buttons_style();
        } else if (e & ENC_RIGHT) {
            selected_yes = false;
            update_buttons_style();
        }
        
        // Нажатие энкодера
        if (e & ENC_CLICK) {
            if (selected_yes) {
                ESP_LOGI(TAG, "Пользователь подтвердил действие");
                // Здесь можно добавить логику сохранения
            } else {
                ESP_LOGI(TAG, "Действие отменено");
            }
            close_yes_no_screen();
        }
    } 
    // Если окно неактивно - открываем по клику
    else {
        if (e & ENC_CLICK) {
            ESP_LOGI(TAG, "Запрос на открытие окна подтверждения");
            create_yes_no_screen();
        }
    }
}
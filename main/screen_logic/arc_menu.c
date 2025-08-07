
#include "arc_menu.h"
#include <stdint.h>
#include "esp_log.h"

// Тег для логирования (оставлен для возможной отладки)
static const char *TAG_ARC = "ARC_MENU";

// Глобальные переменные
uint32_t current_cursor_index = 0; // Текущая позиция курсора в меню
const uint8_t VISIBLE_ITEMS = 5;   // Количество одновременно видимых элементов

// Обновление позиции элементов в дуговом меню
void arc_menu_update_slide(lv_obj_t *cont) {
    // Получаем координаты контейнера
    lv_area_t cont_a;
    lv_obj_get_coords(cont, &cont_a);
    
    // Рассчитываем центр контейнера по Y
    int32_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;
    
    // Вычисляем радиус дуги (70% высоты контейнера)
    int32_t r = lv_obj_get_height(cont) * 7 / 10;
    
    // Обрабатываем все дочерние элементы
    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        lv_area_t child_a;
        lv_obj_get_coords(child, &child_a);
        
        // Вычисляем центр текущего элемента
        int32_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;
        
        // Рассчитываем расстояние от центра контейнера до центра элемента
        int32_t diff_y = child_y_center - cont_y_center;
        diff_y = LV_ABS(diff_y);  // Берем абсолютное значение

        // Вычисляем смещение по X для создания дугового эффекта
        int32_t x;
        if (diff_y >= r) {
            x = r;  // Максимальное смещение для элементов за пределами радиуса
        } else {
            // Вычисляем смещение по теореме Пифагора
            uint32_t x_sqr = r * r - diff_y * diff_y;
            lv_sqrt_res_t res;
            lv_sqrt(x_sqr, &res, 0x8000);  // Аппроксимация квадратного корня
            x = r - res.i;  // Смещение для текущей позиции
        }
        
        // Применяем вычисленное смещение к основному контейнеру
        lv_obj_set_style_translate_x(child, x, 0);
        
        // Применяем обратное смещение к дочерним элементам (компенсация)
        lv_obj_t *st = lv_obj_get_child(child, -1);
        if (st) {
            lv_obj_set_style_translate_x(st, -x, 0);
        }
        
        // Отключаем скроллбар для элемента
        lv_obj_set_scrollbar_mode(child, LV_SCROLLBAR_MODE_OFF);
    }
}

// Обработчик событий скролла
void arc_menu_event_cb(lv_event_t *e) {
    // Обновляем позиции элементов при скролле
    lv_obj_t *cont = lv_event_get_target(e);
    arc_menu_update_slide(cont);
}

// Обработчик событий энкодера
void arc_menu_handle_encoder(uint8_t e, lv_obj_t *cont, uint32_t *current_index) {
    static uint32_t last_event_time = 0;   // Время последнего события
    static uint8_t last_event = 0;         // Тип последнего события
    const uint32_t DEBOUNCE_TIME_MS = 50;  // Время антидребезга
    
    // Получаем текущее время и количество элементов
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t child_count = lv_obj_get_child_cnt(cont);
    
    // Фильтр дребезга - игнорируем повторные события в течение DEBOUNCE_TIME_MS
    if ((e == last_event) && (now - last_event_time < DEBOUNCE_TIME_MS)) {
        return;
    }
    
    // Обработка движения ВЛЕВО
    if (e & ENC_LEFT) {
        if (current_cursor_index > 0) {
            current_cursor_index--;  // Уменьшаем индекс курсора
            
            // Сдвигаем список, если курсор вышел за верхнюю границу видимой области
            if (current_cursor_index < *current_index - 2) {
                // Минимальное значение индекса списка = 2
                if (*current_index > 2) {
                    (*current_index)--;
                }
            }
        }
        last_event = ENC_LEFT;
        last_event_time = now;
    } 
    // Обработка движения ВПРАВО
    else if (e & ENC_RIGHT) {
        if (current_cursor_index < child_count - 1) {
            current_cursor_index++;  // Увеличиваем индекс курсора
            
            // Сдвигаем список, когда курсор достигает последнего видимого элемента
             if (current_cursor_index > *current_index + 2) {
                if (*current_index < child_count - 3) {
                    (*current_index)++;
                    ESP_LOGI(TAG_ARC, "Список сдвинут вниз: новый idx %"PRIu32, *current_index);
                }
            }
            
        }
        last_event = ENC_RIGHT;
        last_event_time = now;
    }
    
    // Обновление позиции после обработки движения
    if (e & (ENC_LEFT | ENC_RIGHT)) {
        // Прокручиваем к текущему элементу списка
        lv_obj_t *target = lv_obj_get_child(cont, *current_index);
        if (target) {
            lv_obj_scroll_to_view(target, LV_ANIM_ON);
        }
        
        // Обновляем дуговое меню
        arc_menu_update_slide(cont);
    }
}
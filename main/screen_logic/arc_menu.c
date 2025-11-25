#include "arc_menu.h"
#include <stdint.h>
#include "esp_log.h"
#include <stdlib.h>

// Тег для логирования
static const char *TAG_ARC = "ARC_MENU";

/**
 * @brief Устанавливает объект как статусную иконку
 * @param obj Указатель на объект LVGL
 */
void set_as_status_icon(lv_obj_t *obj) {
    if (obj == NULL) {
        ESP_LOGE(TAG_ARC, "Attempt to set NULL object as status icon");
        return;
    }
    
    // Выделяем память для пользовательских данных с помощью LVGL
    custom_obj_data_t *data = (custom_obj_data_t*)lv_malloc(sizeof(custom_obj_data_t));
    if (data == NULL) {
        ESP_LOGE(TAG_ARC, "Failed to allocate memory for custom_obj_data_t");
        return;
    }
    
    // Инициализируем структуру
    data->is_status_icon = true;
    
    // Устанавливаем пользовательские данные для объекта
    lv_obj_set_user_data(obj, data);
    
    // Добавляем обработчик события DELETE для автоматической очистки памяти
    lv_obj_add_event_cb(obj, free_custom_data, LV_EVENT_DELETE, NULL);
    
    ESP_LOGD(TAG_ARC, "Object marked as status icon");
}

/**
 * @brief Проверяет, является ли объект статусной иконкой
 * @param obj Указатель на объект LVGL
 * @return true если это статусная иконка, иначе false
 */
bool is_status_icon(lv_obj_t *obj) {
    if (obj == NULL) {
        return false;
    }
    
    custom_obj_data_t *data = (custom_obj_data_t*)lv_obj_get_user_data(obj);
    return (data != NULL && data->is_status_icon);
}

/**
 * @brief Обработчик события DELETE для освобождения пользовательских данных
 * @param e Событие LVGL
 */
void free_custom_data(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    if (obj == NULL) return;
    
    custom_obj_data_t *data = (custom_obj_data_t*)lv_obj_get_user_data(obj);
    if (data != NULL) {
        lv_free(data);
        lv_obj_set_user_data(obj, NULL);  // Обнуляем указатель
        ESP_LOGD(TAG_ARC, "Freed custom data for object");
    }
}

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
        
        // ПРИМЕНЯЕМ КОМПЕНСАЦИЮ ТОЛЬКО К СТАТУСНЫМ ИКОНКАМ
        uint32_t grand_child_cnt = lv_obj_get_child_cnt(child);
        for (uint32_t j = 0; j < grand_child_cnt; j++) {
            lv_obj_t *grand_child = lv_obj_get_child(child, j);
            
            // Проверяем, является ли элемент статусной иконкой
            if (is_status_icon(grand_child)) {
                // Применяем обратное смещение только к статусным иконкам
                lv_obj_set_style_translate_x(grand_child, -x, 0);
                ESP_LOGD(TAG_ARC, "Applied compensation to status icon, offset: %" PRId32, -x);
            }
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
void arc_menu_handle_encoder(uint8_t e, lv_obj_t *cont, menu_state_t *menu_state, menu_type_t menu_type) {
    // Проверяем валидность параметров
    if (!cont || !lv_obj_is_valid(cont) || !menu_state) {
        ESP_LOGE(TAG_ARC, "Invalid parameters in arc_menu_handle_encoder");
        return;
    }
    
    static uint32_t last_event_time = 0;   // Время последнего события
    static uint8_t last_event = 0;         // Тип последнего события
    const uint32_t DEBOUNCE_TIME_MS = 50;  // Время антидребезга
    
    // Получаем конфигурацию для текущего типа меню
    const menu_config_t* config = get_menu_config(menu_type);
    if (!config) {
        ESP_LOGE(TAG_ARC, "Failed to get menu config for type: %d", menu_type);
        return;
    }
    
    uint32_t scroll_boundary = config->scroll_boundary;
    
    // Получаем текущее время и количество элементов
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t child_count = lv_obj_get_child_cnt(cont);
    
    if (child_count == 0) {
        ESP_LOGE(TAG_ARC, "Menu container has no children");
        return;
    }
    
    ESP_LOGI(TAG_ARC, "Encoder event: 0x%02x, child_count: %" PRIu32 ", menu_type: %d", 
             e, child_count, menu_type);
    ESP_LOGI(TAG_ARC, "Before - current_cursor_index: %" PRIu32 ", current_index: %" PRIu32, 
             menu_state->cursor_index, menu_state->list_index);
    
    // Фильтр дребезга - игнорируем повторные события в течение DEBOUNCE_TIME_MS
    if ((e == last_event) && (now - last_event_time < DEBOUNCE_TIME_MS)) {
        ESP_LOGI(TAG_ARC, "Debounce filter applied");
        return;
    }
    
    // Обработка движения ВЛЕВО (против часовой стрелки - ВВЕРХ в списке)
    if (e & ENC_RIGHT) {
        if (menu_state->cursor_index > 0) {
            menu_state->cursor_index--;  // Уменьшаем индекс курсора
            
            // Сдвигаем список, если курсор вышел за верхнюю границу видимой области
            // Используем настраиваемую границу из конфигурации
            if (menu_state->cursor_index < menu_state->list_index - scroll_boundary) {
                // Минимальное значение индекса списка = scroll_boundary
                if (menu_state->list_index > scroll_boundary) {
                    menu_state->list_index--;
                    ESP_LOGI(TAG_ARC, "Список сдвинут вверх: новый idx %"PRIu32, menu_state->list_index);
                }
            }
        }
        last_event = ENC_RIGHT;
        last_event_time = now;
    } 
    // Обработка движения ВПРАВО (по часовой стрелке - ВНИЗ в списке)
    else if (e & ENC_LEFT) {
        if (menu_state->cursor_index < child_count - 1) {
            menu_state->cursor_index++;  // Увеличиваем индекс курсора
            
            // Сдвигаем список, когда курсор достигает последнего видимого элемента
            // Используем настраиваемую границу из конфигурации
            if (menu_state->cursor_index > menu_state->list_index + scroll_boundary) {
                if (menu_state->list_index < child_count - scroll_boundary - 1) {
                    menu_state->list_index++;
                    ESP_LOGI(TAG_ARC, "Список сдвинут вниз: новый idx %"PRIu32, menu_state->list_index);
                }
            }
        }
        last_event = ENC_LEFT;
        last_event_time = now;
    }
    
    // Обновление позиции после обработки движения
    if (e & (ENC_LEFT | ENC_RIGHT)) {
        // Прокручиваем к текущему элементу списка
        lv_obj_t *target = lv_obj_get_child(cont, menu_state->list_index);
        if (target && lv_obj_is_valid(target)) {
            lv_obj_scroll_to_view(target, LV_ANIM_ON);
        }
        
        // Обновляем дуговое меню
        arc_menu_update_slide(cont);
        
        ESP_LOGI(TAG_ARC, "After - current_cursor_index: %" PRIu32 ", current_index: %" PRIu32, 
                 menu_state->cursor_index, menu_state->list_index);
    }
}
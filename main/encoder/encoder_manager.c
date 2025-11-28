/**
 * @file encoder_manager.c
 * @brief Менеджер событий энкодера
 * 
 * Этот модуль обеспечивает централизованную обработку событий от поворотного энкодера.
 * Использует очередь FreeRTOS для передачи событий между задачами и обеспечивает
 * потокобезопасный доступ к LVGL при вызове callback'ов.
 */

#include "encoder_manager.h"
#include "encoder/encoder.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_task_wdt.h"
#include "bsp/esp-bsp.h"

static const char *TAG = "EncoderManager";

// ========== Глобальные переменные ==========
// Очередь событий энкодера (размер: 20 событий)
static QueueHandle_t encoder_queue = NULL;
// Счетчик событий для отладки
static uint32_t event_counter = 0;
// Текущий зарегистрированный callback для обработки событий
static encoder_event_callback_t current_callback = NULL;

/**
 * @brief Обработчик событий от драйвера энкодера
 * 
 * Эта функция вызывается из задачи enc_loop (НЕ из ISR!).
 * Преобразует сырые события энкодера в структурированные события
 * и помещает их в очередь для обработки в encoder_manager_task.
 * 
 * @param event Сырое событие от драйвера (ENC_LEFT, ENC_RIGHT, ENC_CLICK)
 */
static void encoder_event_handler(uint8_t event) {
    encoder_event_t encoder_event;
    
    // Преобразуем событие драйвера в нашу структуру
    switch (event) {
        case ENC_LEFT:
            encoder_event.type = ENCODER_EVENT_LEFT;
            break;
        case ENC_RIGHT:
            encoder_event.type = ENCODER_EVENT_RIGHT;
            break;
        case ENC_CLICK:
            encoder_event.type = ENCODER_EVENT_CLICK;
            break;
        default:
            encoder_event.type = ENCODER_EVENT_UNKNOWN;
            return; // Не отправляем неизвестные события
    }
    
    encoder_event.counter = ++event_counter;
    
    if (encoder_queue != NULL) {
        // Отправляем в очередь без прерываний (это не ISR!)
        xQueueSend(encoder_queue, &encoder_event, pdMS_TO_TICKS(10));
    }
}

/**
 * @brief Инициализация менеджера энкодера
 * 
 * Создает очередь событий и регистрирует обработчик в драйвере энкодера.
 * Должна быть вызвана один раз при старте приложения.
 */
void encoder_manager_init(void) {
    if (encoder_queue == NULL) {
        encoder_queue = xQueueCreate(20, sizeof(encoder_event_t));
        if (encoder_queue == NULL) {
            ESP_LOGE(TAG, "Failed to create encoder queue");
            return;
        }
    }
    
    // Регистрируем обработчик в драйвере энкодера
    enc_register_event(encoder_event_handler);
    ESP_LOGI(TAG, "Encoder manager initialized");
}

/**
 * @brief Получение указателя на очередь событий
 * 
 * @return Указатель на очередь событий или NULL, если очередь не создана
 */
QueueHandle_t encoder_manager_get_queue(void) {
    return encoder_queue;
}

/**
 * @brief Регистрация callback-функции для обработки событий энкодера
 * 
 * Регистрирует функцию, которая будет вызываться при получении событий
 * от энкодера. Только одна callback-функция может быть зарегистрирована
 * в любой момент времени.
 * 
 * @param callback Указатель на функцию-обработчик событий
 */
void encoder_manager_register_callback(encoder_event_callback_t callback) {
    current_callback = callback;
    ESP_LOGI(TAG, "Encoder callback registered: %p", callback);
}

/**
 * @brief Отмена регистрации callback-функции
 * 
 * Удаляет текущий зарегистрированный callback. После вызова этой функции
 * события энкодера не будут обрабатываться до регистрации нового callback.
 */
void encoder_manager_unregister_callback(void) {
    current_callback = NULL;
    ESP_LOGI(TAG, "Encoder callback unregistered");
}

/**
 * @brief Деинициализация менеджера энкодера
 * 
 * Освобождает все ресурсы, выделенные менеджером энкодера:
 * - Отменяет регистрацию обработчика в драйвере
 * - Удаляет очередь событий
 * - Сбрасывает внутренние переменные
 */
void encoder_manager_deinit(void) {
    ESP_LOGI(TAG, "Deinitializing encoder manager");
    
    // Отменяем регистрацию обработчика
    enc_unregister_event();
    current_callback = NULL;
    
    // Удаляем очередь
    if (encoder_queue != NULL) {
        vQueueDelete(encoder_queue);
        encoder_queue = NULL;
        ESP_LOGI(TAG, "Encoder queue deleted");
    }
    
    // Сбрасываем счетчик событий
    event_counter = 0;
    
    ESP_LOGI(TAG, "Encoder manager deinitialized");
}

/**
 * @brief Задача обработки событий энкодера
 * 
 * Основная задача менеджера энкодера. Читает события из очереди и вызывает
 * зарегистрированный callback с блокировкой LVGL для обеспечения потокобезопасности.
 * 
 * @param arg Параметры задачи (не используется)
 * 
 * @note Выполняется в бесконечном цикле
 * @note Все callback'и вызываются с заблокированным LVGL мьютексом
 */
void encoder_manager_task(void* arg) {
    encoder_event_t event;
    uint8_t raw_event;
    
    esp_task_wdt_add(NULL);
    
    ESP_LOGI(TAG, "Encoder manager task started");
    
    while (1) {
        esp_task_wdt_reset();
        
        // Увеличиваем таймаут для уменьшения нагрузки
        if (xQueueReceive(encoder_queue, &event, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Преобразуем обратно в raw event
            switch (event.type) {
                case ENCODER_EVENT_LEFT:
                    raw_event = ENC_LEFT;
                    break;
                case ENCODER_EVENT_RIGHT:
                    raw_event = ENC_RIGHT;
                    break;
                case ENCODER_EVENT_CLICK:
                    raw_event = ENC_CLICK;
                    break;
                default:
                    raw_event = 0;
                    continue;
            }
            
            // Вызываем зарегистрированный callback с блокировкой LVGL
            // ВАЖНО: LVGL не потокобезопасен, поэтому все вызовы LVGL функций
            // должны быть защищены блокировкой
            if (current_callback != NULL) {
                if (bsp_display_lock(100)) {  // Таймаут 100мс
                    current_callback(raw_event);
                    bsp_display_unlock();
                } else {
                    ESP_LOGW(TAG, "Failed to acquire LVGL lock for encoder callback");
                }
            }
        }
        
        // Увеличиваем задержку для уменьшения нагрузки на CPU
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
#include "encoder_manager.h"
#include "encoder/encoder.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static const char *TAG = "EncoderManager";

// Глобальная очередь событий
static QueueHandle_t encoder_queue = NULL;
static uint32_t event_counter = 0;
static encoder_event_callback_t current_callback = NULL;

// Обработчик прерываний от энкодера
static void IRAM_ATTR encoder_event_handler(uint8_t event) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
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
        if (xQueueSendFromISR(encoder_queue, &encoder_event, &xHigherPriorityTaskWoken) == pdTRUE) {
            // Корректный вызов с аргументом
            if (xHigherPriorityTaskWoken == pdTRUE) {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
    }
}

// Инициализация менеджера энкодера
void encoder_manager_init(void) {
    if (encoder_queue == NULL) {
        encoder_queue = xQueueCreate(40, sizeof(encoder_event_t));
        if (encoder_queue == NULL) {
            ESP_LOGE(TAG, "Failed to create encoder queue");
            return;
        }
    }
    
    // Регистрируем обработчик в драйвере энкодера
    enc_register_event(encoder_event_handler);
    ESP_LOGI(TAG, "Encoder manager initialized");
}

// Получение очереди событий
QueueHandle_t encoder_manager_get_queue(void) {
    return encoder_queue;
}

// Регистрация callback-функции для обработки событий
void encoder_manager_register_callback(encoder_event_callback_t callback) {
    current_callback = callback;
    ESP_LOGI(TAG, "Encoder callback registered");
}

// Отмена регистрации callback-функции
void encoder_manager_unregister_callback(void) {
    current_callback = NULL;
    ESP_LOGI(TAG, "Encoder callback unregistered");
}

// Задача обработки событий энкодера
void encoder_manager_task(void* arg) {
    encoder_event_t event;
    uint8_t raw_event;
    
    while (1) {
        if (xQueueReceive(encoder_queue, &event, portMAX_DELAY) == pdTRUE) {
            // Преобразуем обратно в raw event для совместимости
            switch (event.type) {
                case ENCODER_EVENT_LEFT:
                    raw_event = ENC_LEFT;
                    ESP_LOGI(TAG, "Encoder (%lu): LEFT rotation", event.counter);
                    break;
                case ENCODER_EVENT_RIGHT:
                    raw_event = ENC_RIGHT;
                    ESP_LOGI(TAG, "Encoder (%lu): RIGHT rotation", event.counter);
                    break;
                case ENCODER_EVENT_CLICK:
                    raw_event = ENC_CLICK;
                    ESP_LOGI(TAG, "Encoder: BUTTON click");
                    break;
                default:
                    raw_event = 0;
                    ESP_LOGI(TAG, "Unknown encoder event: %d", event.type);
                    continue; // Пропускаем неизвестные события
            }
            
            // Вызываем зарегистрированный callback, если он есть
            if (current_callback != NULL) {
                current_callback(raw_event);
            }
        }
    }
}
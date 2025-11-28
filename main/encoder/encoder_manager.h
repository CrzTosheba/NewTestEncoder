#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Типы событий энкодера
typedef enum {
    ENCODER_EVENT_LEFT,
    ENCODER_EVENT_RIGHT, 
    ENCODER_EVENT_CLICK,
    ENCODER_EVENT_UNKNOWN
} encoder_event_type_t;

// Структура события
typedef struct {
    encoder_event_type_t type;
    uint32_t counter;
} encoder_event_t;

// Тип callback-функции для обработки событий
typedef void (*encoder_event_callback_t)(uint8_t event);

// Функции для работы с менеджером энкодера
void encoder_manager_init(void);
void encoder_manager_deinit(void);
QueueHandle_t encoder_manager_get_queue(void);
void encoder_manager_task(void* arg);
void encoder_manager_register_callback(encoder_event_callback_t callback);
void encoder_manager_unregister_callback(void);

#ifdef __cplusplus
}
#endif

#endif // ENCODER_MANAGER_H
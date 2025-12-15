#include "access_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "esp_log.h"

// Объявления функций обновления отображения (избегаем циклических зависимостей)
extern void main_menu_update_access_display(void);
extern void screen_Pass_update_display(void);

static const char *TAG = "ACCESS_CTRL";

// Состояние доступа
static bool access_unlocked = false;

// Таймер для автоматического закрытия доступа
static TimerHandle_t inactivity_timer = NULL;

// Очередь для обработки событий таймера (чтобы не выполнять тяжелые операции в callback)
static QueueHandle_t timer_event_queue = NULL;

// Тип события таймера
typedef enum {
    TIMER_EVENT_TIMEOUT
} timer_event_type_t;

// Задача для обновления отображения при закрытии доступа
static void update_display_task(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(100));  // Небольшая задержка
    main_menu_update_access_display();
    screen_Pass_update_display();
    vTaskDelete(NULL);
}

// Задача для обработки событий таймера (выполняется в отдельной задаче с достаточным стеком)
static void timer_event_handler_task(void *pvParameters) {
    timer_event_type_t event;
    
    while (1) {
        if (xQueueReceive(timer_event_queue, &event, portMAX_DELAY) == pdTRUE) {
            if (event == TIMER_EVENT_TIMEOUT) {
                ESP_LOGI(TAG, "Inactivity timeout reached, locking access");
                // Вызываем блокировку доступа (она сама проверит состояние таймера)
                access_control_lock();
            }
        }
    }
}

// Callback для таймера неактивности
// ВАЖНО: Этот callback выполняется в задаче "Tmr Svc" с очень маленьким стеком!
// Поэтому мы НЕ вызываем здесь тяжелые операции, а только отправляем событие в очередь
static void inactivity_timer_callback(TimerHandle_t xTimer) {
    // Минимальный код - только отправка события в очередь
    // НЕ используем ESP_LOGI или другие тяжелые операции здесь!
    timer_event_type_t event = TIMER_EVENT_TIMEOUT;
    
    // Отправляем событие в очередь (неблокирующий вызов, таймаут 0)
    // Callback таймера выполняется в задаче, поэтому используем xQueueSend
    if (timer_event_queue != NULL) {
        xQueueSend(timer_event_queue, &event, 0);
    }
}

/**
 * @brief Инициализация модуля управления доступом
 */
void access_control_init(void) {
    // Создаем очередь для событий таймера (размер 1, так как события обрабатываются быстро)
    timer_event_queue = xQueueCreate(1, sizeof(timer_event_type_t));
    if (timer_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create timer event queue");
        return;
    }
    
    // Создаем задачу для обработки событий таймера
    xTaskCreate(timer_event_handler_task, "timer_handler", 4096, NULL, 5, NULL);
    
    // Вычисляем период таймера в миллисекундах с защитой от переполнения
    // Используем uint32_t для избежания переполнения при умножении
    uint32_t timeout_ms = (uint32_t)ACCESS_TIMEOUT_MINUTES * 60U * 1000U;
    
    // Проверяем, что значение не превышает максимальный период таймера FreeRTOS
    // Максимальный период для TickType_t (uint32_t) при 1000 Hz = ~49.7 дней
    // Но для безопасности ограничим до разумного значения (24 часа = 86,400,000 мс)
    if (timeout_ms > 86400000U) {
        ESP_LOGW(TAG, "Timeout too large (%lu ms), limiting to 24 hours", timeout_ms);
        timeout_ms = 86400000U;
    }
    
    // Создаем таймер для автоматического закрытия доступа
    inactivity_timer = xTimerCreate(
        "AccessTimeout",
        pdMS_TO_TICKS(timeout_ms),
        pdFALSE,  // Одноразовый таймер
        NULL,
        inactivity_timer_callback
    );
    
    if (inactivity_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create inactivity timer");
    } else {
        ESP_LOGI(TAG, "Access control initialized (timeout: %d minutes, %lu ms, %lu ticks)", 
                 ACCESS_TIMEOUT_MINUTES, timeout_ms, pdMS_TO_TICKS(timeout_ms));
    }
}

/**
 * @brief Проверка, открыт ли доступ
 */
bool access_control_is_unlocked(void) {
    return access_unlocked;
}

/**
 * @brief Открытие доступа
 */
void access_control_unlock(void) {
    if (!access_unlocked) {
        access_unlocked = true;
        ESP_LOGI(TAG, "Access unlocked");
        
        // Запускаем таймер неактивности
        if (inactivity_timer != NULL) {
            xTimerReset(inactivity_timer, 0);
        }
    }
}

/**
 * @brief Закрытие доступа
 */
void access_control_lock(void) {
    if (access_unlocked) {
        access_unlocked = false;
        ESP_LOGI(TAG, "Access locked");
        
        // Останавливаем таймер неактивности (если он еще активен)
        // Для одноразового таймера, который уже истек, это безопасно
        if (inactivity_timer != NULL) {
            // Проверяем, активен ли таймер, перед остановкой
            BaseType_t timer_active = xTimerIsTimerActive(inactivity_timer);
            if (timer_active != pdFALSE) {
                xTimerStop(inactivity_timer, 0);
            }
        }
        
        // Обновляем отображение (вызываем через задачу, так как это может быть вызвано из таймера)
        // Используем небольшую задержку, чтобы избежать проблем с блокировками
        // Вызываем обновление асинхронно через задачу
        xTaskCreate(update_display_task, "access_update", 2048, NULL, 1, NULL);
    }
}

/**
 * @brief Сброс таймера активности (вызывать при любой активности пользователя)
 */
void access_control_reset_activity_timer(void) {
    if (access_unlocked && inactivity_timer != NULL) {
        xTimerReset(inactivity_timer, 0);
    }
}

/**
 * @brief Обновление таймера активности (алиас для reset)
 */
void access_control_update_activity_timer(void) {
    access_control_reset_activity_timer();
}


#include "access_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"

// Объявления функций обновления отображения (избегаем циклических зависимостей)
extern void main_menu_update_access_display(void);
extern void screen_Pass_update_display(void);

static const char *TAG = "ACCESS_CTRL";

// Состояние доступа
static bool access_unlocked = false;

// Таймер для автоматического закрытия доступа
static TimerHandle_t inactivity_timer = NULL;

// Задача для обновления отображения при закрытии доступа
static void update_display_task(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(100));  // Небольшая задержка
    main_menu_update_access_display();
    screen_Pass_update_display();
    vTaskDelete(NULL);
}

// Callback для таймера неактивности
static void inactivity_timer_callback(TimerHandle_t xTimer) {
    ESP_LOGI(TAG, "Inactivity timeout reached, locking access");
    access_control_lock();
}

/**
 * @brief Инициализация модуля управления доступом
 */
void access_control_init(void) {
    // Создаем таймер для автоматического закрытия доступа
    inactivity_timer = xTimerCreate(
        "AccessTimeout",
        pdMS_TO_TICKS(ACCESS_TIMEOUT_MINUTES * 60 * 1000),  // 10 минут в миллисекундах
        pdFALSE,  // Одноразовый таймер
        NULL,
        inactivity_timer_callback
    );
    
    if (inactivity_timer == NULL) {
        ESP_LOGE(TAG, "Failed to create inactivity timer");
    } else {
        ESP_LOGI(TAG, "Access control initialized (timeout: %d minutes)", ACCESS_TIMEOUT_MINUTES);
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
        
        // Останавливаем таймер неактивности
        if (inactivity_timer != NULL) {
            xTimerStop(inactivity_timer, 0);
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


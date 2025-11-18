#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "bsp/esp-bsp.h"
#include "lvgl/lvgl.h"
#include "lv_bg_color/lv_bg_main_screen.h"
#include "encoder/encoder.h"
#include "driver/gpio.h"
#include "menu_layer/main_menu/main_menu.h"
#include "esp_heap_caps.h"
#include "encoder/encoder_manager.h"
#include "screen_logic/screen_navigation.h"

// Тег для логирования
static const char *TAG = "app_main";

// Флаги и константы конфигурации
#define LVGL_TASK_DELAY_MS 5 // Задержка для задачи обработки LVGL таймеров (мс)

// Инициализация GPIO для энкодера
static void rotary_encoder_gpio_init(void) {
    // Конфигурация GPIO для пинов энкодера
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_ROT_ENC_SW) | // Битовая маска для SW (кнопка)
                        (1ULL << GPIO_ROT_ENC_A) |  // Битовая маска для канала A
                        (1ULL << GPIO_ROT_ENC_B),   // Битовая маска для канала B
        .mode = GPIO_MODE_INPUT,      // Режим ввода
        .pull_up_en = GPIO_PULLUP_ENABLE, // Включить внутреннюю подтяжку к VCC
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // Отключить подтяжку к GND
        .intr_type = GPIO_INTR_DISABLE // Без прерываний
    };
    gpio_config(&io_conf); // Применение конфигурации
}

void main_screen_bg(void)
{
    lv_obj_t* scr_bg = lv_scr_act();
    lv_obj_set_style_bg_color(scr_bg, lv_color_hex(0x1e2528), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(scr_bg, LV_SCROLLBAR_MODE_OFF); // дизеблим скрол бар на главном экране
    fflush(NULL);
}

// Задача обработки таймеров LVGL
void lvgl_timer_task(void* arg) {
    esp_task_wdt_add(NULL);
    
    while (1) {
        lv_timer_handler();
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(LVGL_TASK_DELAY_MS));
    }
}

// Главная функция приложения
void app_main(void)
{
    // Инициализация watchdog с увеличенным таймаутом
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = 15000, // Увеличиваем до 15 секунд
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };
    ESP_ERROR_CHECK(esp_task_wdt_reconfigure(&wdt_config));
    
    // Регистрируем главную задачу в watchdog
    esp_task_wdt_add(NULL);
    
    // Инициализация GPIO энкодера
    rotary_encoder_gpio_init();
    ESP_LOGI(TAG, "Encoder GPIO initialized");

    // Инициализация дисплея
    bsp_display_start();
    bsp_display_backlight_on();
    ESP_LOGI(TAG, "++Display LVGL demo");
    
    // Инициализация менеджера энкодера
    encoder_manager_init();
    ESP_LOGI(TAG, "Encoder manager initialized");

    // Блокировка дисплея перед созданием UI
    bsp_display_lock(0);
    main_screen_bg();
    
    // Инициализация системы навигации
    screen_navigation_init();

    // Разблокировка дисплея
    bsp_display_unlock();
    
    // Инициализация драйвера энкодера
    enc_init(10, GPIO_ROT_ENC_SW, GPIO_ROT_ENC_A, GPIO_ROT_ENC_B);
    ESP_LOGI(TAG, "Encoder driver initialized");
    
    // Создание задач с увеличенными размерами стеков:
    
    // Задача обработки энкодера
    xTaskCreatePinnedToCore(
        enc_loop,
        "rotary_encoder_task",
        6144,  // Увеличиваем стек
        NULL,
        5,
        NULL,
        APP_CPU_NUM
    );        
    
    // Задача обработки событий энкодера
    xTaskCreatePinnedToCore(
        encoder_manager_task,
        "encoder_manager",
        8192,  // Увеличиваем стек
        NULL,
        3,
        NULL,
        PRO_CPU_NUM
    );
    
    // Задача обработки таймеров LVGL
    xTaskCreatePinnedToCore(
        lvgl_timer_task,
        "lvgl_timers",
        16 * 1024,  // Увеличиваем стек до 16KB
        NULL,
        4,
        NULL,
        PRO_CPU_NUM
    );
    
    ESP_LOGI(TAG, "All tasks created successfully");
    
    // Основной цикл приложения
    while (1) {
        esp_task_wdt_reset();
        // Увеличиваем задержку для уменьшения нагрузки
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
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
#include "screen_logic/screen_navigation.h"  // Добавляем навигацию


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
    // Инициализация GPIO энкодера
    rotary_encoder_gpio_init();
    ESP_LOGI(TAG, "Encoder GPIO initialized");

    // Инициализация дисплея
    bsp_display_start();
    // Включение подсветки дисплея
    bsp_display_backlight_on();
    ESP_LOGI(TAG, "++Display LVGL demo");
    
    // Инициализация менеджера энкодера
    encoder_manager_init();
    ESP_LOGI(TAG, "Encoder manager initialized");

    // Блокировка дисплея перед созданием UI
    bsp_display_lock(0);
    main_screen_bg();
    
    // Инициализация системы навигации (вместо прямого вызова Main_Menu_List)
    screen_navigation_init();

    // Разблокировка дисплея
    bsp_display_unlock();
    
    // Инициализация драйвера энкодера
    enc_init(10, GPIO_ROT_ENC_SW, GPIO_ROT_ENC_A, GPIO_ROT_ENC_B);
    ESP_LOGI(TAG, "Encoder driver initialized");
    
    // Создание задач:
    
    // Задача обработки энкодера на ядре APP_CPU (обычно CPU1)
    xTaskCreatePinnedToCore(
        enc_loop,                   // Функция задачи
        "rotary_encoder_task",      // Имя задачи
        4096,                       // Размер стека
        NULL,                       // Параметры
        5,                          // Приоритет (выше среднего)
        NULL,                       // Дескриптор задачи
        APP_CPU_NUM                 // Ядро процессора
    );        
    
    // Задача обработки событий энкодера на ядре PRO_CPU
    xTaskCreatePinnedToCore(
        encoder_manager_task,       // Функция задачи
        "encoder_manager",          // Имя задачи
        4096,                       // Размер стека
        NULL,                       // Параметры
        3,                          // Приоритет (средний)
        NULL,                       // Дескриптор задачи
        PRO_CPU_NUM                 // Ядро процессора
    );
    
    // Задача обработки таймеров LVGL на ядре PRO_CPU
    xTaskCreatePinnedToCore(
        lvgl_timer_task,            // Функция задачи
        "lvgl_timers",              // Имя задачи
        12 * 1024,                  // Размер стека (12KB)
        NULL,                       // Параметры
        4,                          // Приоритет (выше среднего)
        NULL,                       // Дескриптор задачи
        PRO_CPU_NUM                 // Ядро процессора
    );
    
    ESP_LOGI(TAG, "All tasks created successfully");
    
    // Основной цикл приложения
    while (1) {
        // Периодическая задержка
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Сообщение о запуске приложения (не достижимо из-за бесконечного цикла)
    ESP_LOGI(TAG, "Application started");
}
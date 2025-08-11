#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "lv_demos.h"
#include "bsp/esp-bsp.h"
#include "lvgl/lvgl.h"
#include "lv_bg_color/lv_bg_main_screen.h"
#include "encoder/encoder.h"
#include "screens/S_Co/screen_CO.h"
#include "driver/gpio.h"
#include "dialog_screen/screen_YES_NO/yes_no_screen.h"
#include "menu_layer/main_menu/main_menu.h"
#include "esp_heap_caps.h"
#include "screens/S_Uv/screen_Uv.h"

// Тег для логирования
static const char *TAG = "app_main";

// Флаги и константы конфигурации
#define LVGL_TASK_DELAY_MS 2 // Задержка для задачи обработки LVGL таймеров (мс)
#define ENCODER_QUEUE_SIZE 40 // Размер очереди событий энкодера
#define MAX_EVENTS_PER_CYCLE 5 // Максимальное количество событий за цикл обработки

// Глобальная очередь для событий энкодера
QueueHandle_t encoder_queue = NULL;

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

// Обработчик событий энкодера (вызывается из драйвера энкодера)
static void IRAM_ATTR encoder_event_handler(uint8_t event) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(encoder_queue, &event, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// Задача обработки событий LVGL
void lvgl_event_handler(void* arg) {
    esp_task_wdt_add(NULL);
    uint8_t event;
    uint32_t event_count = 0;
    static uint32_t counter = 0;
    
    while (1) {
        event_count = 0;
        while (xQueueReceive(encoder_queue, &event, 0) == pdTRUE && 
               event_count < MAX_EVENTS_PER_CYCLE) {
            // Логирование события энкодера
            counter++;
            switch (event) {
                case ENC_LEFT:
                    ESP_LOGI(TAG, "Encoder (%lu): LEFT rotation", counter);
                    break;
                case ENC_RIGHT:
                    ESP_LOGI(TAG, "Encoder (%lu): RIGHT rotation", counter);
                    break;
                case ENC_CLICK:
                    ESP_LOGI(TAG, "Encoder: BUTTON click");
                    break;
                default:
                    ESP_LOGI(TAG, "Encoder event: 0x%02x", event);
            }
            
            // Здесь должна быть обработка события в UI
             main_menu_encoder_event_cb(event);
            
            event_count++;
            esp_task_wdt_reset();
        }
        
        if (event_count >= MAX_EVENTS_PER_CYCLE) {
            taskYIELD();
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        esp_task_wdt_reset();
    }
}

void main_screen_bg(void)
{

  
lv_obj_t* scr_bg = lv_scr_act();
lv_obj_set_style_bg_color(scr_bg, lv_color_hex(0x1e2528), LV_PART_MAIN);
lv_obj_set_scrollbar_mode(scr_bg, LV_SCROLLBAR_MODE_OFF); // дизеблим скрол бар на главном экране



fflush(NULL);
};

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
    ESP_LOGI(TAG, "Encoder initialized");

    // Инициализация дисплея
    bsp_display_start();
    // Включение подсветки дисплея
    bsp_display_backlight_on();
    ESP_LOGI(TAG, "++Display LVGL demo");
    
    // Создание очереди для событий энкодера
    encoder_queue = xQueueCreate(ENCODER_QUEUE_SIZE, sizeof(uint8_t));

    // Блокировка дисплея перед созданием UI
    bsp_display_lock(0);
     main_screen_bg();
    // Создание экрана CO
    //main_CO_screen();
    // Создание главного меню
    Main_Menu_List();
  //screen_Uv_create(LV_SCR_LOAD_ANIM_FADE_ON); // подключение другого экрана, но обновление элементов, а именно экрана

    // Разблокировка дисплея
    bsp_display_unlock();
    
    enc_init(10, GPIO_ROT_ENC_SW, GPIO_ROT_ENC_A, GPIO_ROT_ENC_B);
    enc_register_event(encoder_event_handler);
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
    
    // Задача обработки событий LVGL на ядре PRO_CPU (обычно CPU0)
    xTaskCreatePinnedToCore(
        lvgl_event_handler,         // Функция задачи
        "lvgl_events",              // Имя задачи
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
    
    // Основной цикл приложения
    while (1) {
        // Периодическая задержка (30 секунд)
        vTaskDelay(pdMS_TO_TICKS(30000));
    }

    // Сообщение о запуске приложения (не достижимо из-за бесконечного цикла)
    ESP_LOGI(TAG, "Application started");

// Логирование информации о памяти (только если LOG_MEM_INFO == 1)
#if LOG_MEM_INFO
    static char buffer[128];
    while (1) {
        // Форматирование информации о памяти
        sprintf(buffer, "   Biggest /     Free /    Total\n"
                "\t  SRAM : [%8d / %8d / %8d]\n"
                "\t PSRAM : [%8d / %8d / %8d]",
                heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
                heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_total_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM),
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
                heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
        // Логирование информации о памяти
        ESP_LOGI("MEM", "%s", buffer);

        // Задержка 30 секунд между логами
        vTaskDelay(pdMS_TO_TICKS(30000));
    }
#endif
}
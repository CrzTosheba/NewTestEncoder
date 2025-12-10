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
#include "menu_layer/CO_Menu/CO_general_params.h"
#include "menu_layer/CO_Menu/CO_heating_graph_params.h"
#include "menu_layer/CO_Menu/CO_pumps_params.h"
#include "menu_layer/CO_Menu/CO_valve_params.h"
#include "menu_layer/CO_Menu/CO_manual_params.h"
#include "menu_layer/CO_Menu/CO_schedule_params.h"
#include "menu_layer/CO_Menu/CO_dry_run_params.h"
#include "menu_layer/CO_Menu/CO_ext_alarm_params.h"
#include "menu_layer/CO_Menu/CO_sensor_break_params.h"
#include "menu_layer/CO_Menu/CO_dev_alarm_params.h"
#include "menu_layer/GVS_Menu/GVS_pumps_params.h"
#include "menu_layer/GVS_Menu/GVS_valve_params.h"
#include "menu_layer/GVS_Menu/GVS_manual_params.h"
#include "menu_layer/GVS_Menu/GVS_schedule_params.h"
#include "menu_layer/GVS_Menu/GVS_dry_run_params.h"
#include "menu_layer/GVS_Menu/GVS_ext_alarm_params.h"
#include "menu_layer/GVS_Menu/GVS_sensor_break_params.h"
#include "menu_layer/GVS_Menu/GVS_dev_alarm_params.h"
#include "esp_heap_caps.h"
#include "encoder/encoder_manager.h"
#include "screen_logic/screen_navigation.h"
#include "screen_logic/access_control.h"
#include "nvs_flash.h"

/**
 * @file main.c
 * @brief Главный файл приложения ESP32-S3 LCD с LVGL
 * 
 * Этот файл содержит точку входа приложения и инициализацию всех систем:
 * - Инициализация дисплея и LVGL
 * - Настройка энкодера для пользовательского ввода
 * - Создание задач FreeRTOS
 * - Мониторинг памяти
 */

// Тег для логирования ESP-IDF
static const char *TAG = "app_main";

/**
 * @brief Инициализация GPIO пинов для энкодера
 * 
 * Настраивает GPIO пины для работы с поворотным энкодером:
 * - SW (Switch) - кнопка энкодера
 * - A и B - каналы для определения направления вращения
 * 
 * Все пины настроены как входы с внутренней подтяжкой к VCC.
 * Прерывания отключены, так как используется опрос в задаче.
 */
static void rotary_encoder_gpio_init(void) {
    // Конфигурация GPIO для пинов энкодера
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_ROT_ENC_SW) | // Битовая маска для SW (кнопка)
                        (1ULL << GPIO_ROT_ENC_A) |  // Битовая маска для канала A
                        (1ULL << GPIO_ROT_ENC_B),   // Битовая маска для канала B
        .mode = GPIO_MODE_INPUT,                    // Режим ввода
        .pull_up_en = GPIO_PULLUP_ENABLE,           // Включить внутреннюю подтяжку к VCC
        .pull_down_en = GPIO_PULLDOWN_DISABLE,      // Отключить подтяжку к GND
        .intr_type = GPIO_INTR_DISABLE              // Без прерываний (используем опрос)
    };
    gpio_config(&io_conf); // Применение конфигурации
}

/**
 * @brief Настройка фона главного экрана
 * 
 * Устанавливает темный фон для главного экрана и отключает скроллбар.
 * Эта функция вызывается при инициализации приложения.
 */
void main_screen_bg(void)
{
    lv_obj_t* scr_bg = lv_scr_act(); // Получаем указатель на активный экран
    // Устанавливаем темно-серый фон (#1e2528)
    lv_obj_set_style_bg_color(scr_bg, lv_color_hex(0x1e2528), LV_PART_MAIN);
    // Отключаем скроллбар на главном экране для чистого интерфейса
    lv_obj_set_scrollbar_mode(scr_bg, LV_SCROLLBAR_MODE_OFF);
    fflush(NULL); // Принудительная очистка буферов вывода
}

/**
 * @brief Главная функция приложения - точка входа
 * 
 * Выполняет инициализацию всех подсистем:
 * 1. Настройка watchdog таймера
 * 2. Инициализация GPIO для энкодера
 * 3. Инициализация дисплея и LVGL
 * 4. Создание пользовательского интерфейса
 * 5. Создание задач FreeRTOS
 * 6. Основной цикл с мониторингом памяти
 * 
 * @note Функция не возвращает управление (бесконечный цикл)
 */
void app_main(void)
{
    // ========== Инициализация Watchdog таймера ==========
    // Увеличенный таймаут (15 секунд) для сложных операций инициализации
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = 15000,                          // Таймаут: 15 секунд
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, // Мониторинг всех ядер
        .trigger_panic = true                          // Перезагрузка при таймауте
    };
    ESP_ERROR_CHECK(esp_task_wdt_reconfigure(&wdt_config));
    
    // Регистрируем главную задачу в watchdog для мониторинга
    esp_task_wdt_add(NULL);
    
    // ========== Инициализация NVS (Non-Volatile Storage) ==========
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized");
    
    // ========== Инициализация GPIO для энкодера ==========
    rotary_encoder_gpio_init();
    ESP_LOGI(TAG, "Encoder GPIO initialized");

    // ========== Инициализация дисплея и LVGL ==========
    // bsp_display_start() инициализирует LVGL и создает задачу для lv_timer_handler()
    bsp_display_start();
    bsp_display_backlight_on(); // Включаем подсветку дисплея
    ESP_LOGI(TAG, "++Display LVGL demo");
    
    // ========== Инициализация менеджера энкодера ==========
    // Создает очередь событий и регистрирует обработчик
    encoder_manager_init();
    ESP_LOGI(TAG, "Encoder manager initialized");
    
    // ========== Инициализация управления доступом ==========
    access_control_init();
    ESP_LOGI(TAG, "Access control initialized");

    // ========== Создание пользовательского интерфейса ==========
    // ВАЖНО: Все операции с LVGL должны выполняться с заблокированным мьютексом
    // Блокировка дисплея перед созданием UI (0 = бесконечное ожидание)
    bsp_display_lock(0);
    main_screen_bg();              // Настройка фона главного экрана
    
    // Инициализация системы навигации (создает главное меню)
    screen_navigation_init();
    
    // Инициализация параметров отопления
    co_general_params_init();
    ESP_LOGI(TAG, "CO general parameters initialized");
    
    // Инициализация параметров графика отопления
    co_heating_graph_params_init();
    ESP_LOGI(TAG, "CO heating graph parameters initialized");
    
    // Инициализация параметров насосов
    co_pumps_params_init();
    ESP_LOGI(TAG, "CO pumps parameters initialized");
    
    // Инициализация параметров клапан
    co_valve_params_init();
    ESP_LOGI(TAG, "CO valve parameters initialized");
    
    // Инициализация параметров ручного режима
    co_manual_params_init();
    
    // Инициализация параметров расписания
    co_schedule_params_init();
    ESP_LOGI(TAG, "CO schedule parameters initialized");
    
    // Инициализация параметров аварий
    co_dry_run_params_init();
    co_ext_alarm_params_init();
    co_sensor_break_params_init();
    co_dev_alarm_params_init();
    ESP_LOGI(TAG, "CO alarms parameters initialized");
    
    // Инициализация параметров насосов ГВС
    gvs_pumps_params_init();
    ESP_LOGI(TAG, "GVS pumps parameters initialized");
    
    // Инициализация параметров клапан ГВС
    gvs_valve_params_init();
    ESP_LOGI(TAG, "GVS valve parameters initialized");

    // Инициализация параметров ручной режим ГВС
    gvs_manual_params_init();
    ESP_LOGI(TAG, "GVS manual parameters initialized");

    // Инициализация параметров расписания ГВС
    gvs_schedule_params_init();
    ESP_LOGI(TAG, "GVS schedule parameters initialized");

    // Инициализация параметров сухого хода ГВС
    gvs_dry_run_params_init();
    ESP_LOGI(TAG, "GVS dry run parameters initialized");
    
    // Инициализация параметров аварий ГВС
    gvs_ext_alarm_params_init();
    gvs_sensor_break_params_init();
    gvs_dev_alarm_params_init();
    ESP_LOGI(TAG, "GVS alarms parameters initialized");

    // Разблокировка дисплея после завершения операций с LVGL
    bsp_display_unlock();
    
    // ========== Инициализация драйвера энкодера ==========
    // enc_init(задержка_опроса_мс, GPIO_SW, GPIO_A, GPIO_B)
    enc_init(10, GPIO_ROT_ENC_SW, GPIO_ROT_ENC_A, GPIO_ROT_ENC_B);
    ESP_LOGI(TAG, "Encoder driver initialized");
    
    // ========== Создание задач FreeRTOS ==========
    // Задачи создаются с увеличенными размерами стеков для надежности
    
    // Задача обработки энкодера (опрос GPIO пинов)
    // Выполняется на APP_CPU (второе ядро) с приоритетом 5
    xTaskCreatePinnedToCore(
        enc_loop,                    // Функция задачи
        "rotary_encoder_task",       // Имя задачи (для отладки)
        6144,                        // Размер стека: 6KB
        NULL,                        // Параметры задачи
        5,                           // Приоритет задачи
        NULL,                        // Handle задачи (не сохраняем)
        APP_CPU_NUM                  // Ядро: APP_CPU (ядро 1)
    );        
    
    // Задача обработки событий энкодера (обработка очереди событий)
    // Выполняется на PRO_CPU (первое ядро) с приоритетом 3
    xTaskCreatePinnedToCore(
        encoder_manager_task,        // Функция задачи
        "encoder_manager",           // Имя задачи
        8192,                        // Размер стека: 8KB
        NULL,                        // Параметры задачи
        3,                           // Приоритет задачи
        NULL,                        // Handle задачи
        PRO_CPU_NUM                  // Ядро: PRO_CPU (ядро 0)
    );
    
    // ========== ВАЖНОЕ ЗАМЕЧАНИЕ О LVGL ТАЙМЕРАХ ==========
    // НЕ создаем отдельную задачу для lv_timer_handler()!
    // esp_lvgl_port уже создает свою задачу, которая вызывает lv_timer_handler()
    // Двойной вызов lv_timer_handler() из разных задач может привести к:
    // - Повреждению внутренних структур данных LVGL
    // - Перезагрузке контроллера
    // - Непредсказуемому поведению интерфейса
    
    ESP_LOGI(TAG, "All tasks created successfully");
    
    // ========== Вывод информации о памяти при старте ==========
    ESP_LOGI(TAG, "=== System Memory Info ===");
    ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "Min free heap: %d bytes", esp_get_minimum_free_heap_size());
    #ifdef CONFIG_SPIRAM
    // Информация о внешней PSRAM (если включена)
    ESP_LOGI(TAG, "SPIRAM free: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGI(TAG, "Internal RAM free: %d bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    #endif
    ESP_LOGI(TAG, "Largest free block: %d bytes", 
             heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
    
    // ========== Основной цикл приложения ==========
    // Выполняет периодический мониторинг состояния системы
    uint32_t loop_count = 0;
    while (1) {
        // Сброс watchdog таймера на каждой итерации
        esp_task_wdt_reset();
        
        // Периодический вывод информации о памяти (каждые 30 секунд)
        // Помогает отслеживать утечки памяти в реальном времени
        if (loop_count % 30 == 0) {
            ESP_LOGI(TAG, "=== Memory Status (loop %lu) ===", loop_count);
            ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
            ESP_LOGI(TAG, "Min free heap: %d bytes", esp_get_minimum_free_heap_size());
            #ifdef CONFIG_SPIRAM
            ESP_LOGI(TAG, "SPIRAM free: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
            #endif
        }
        
        loop_count++;
        // Задержка 1 секунда для уменьшения нагрузки на CPU
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
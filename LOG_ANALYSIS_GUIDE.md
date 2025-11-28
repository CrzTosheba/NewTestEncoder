# Руководство по анализу логов ESP32-S3

## Быстрый старт

### Загрузка и мониторинг через скрипт:
```powershell
.\flash_and_monitor.ps1
```

### Или вручную:
```powershell
# 1. Сборка
idf.py build

# 2. Загрузка
idf.py -p COM3 flash

# 3. Мониторинг
idf.py -p COM3 monitor
```

---

## 🔍 Анализ логов

### Ключевые теги для мониторинга

#### 1. **Инициализация системы**
```
I (xxx) cpu_start: App cpu up.
I (xxx) heap_init: Initializing. RAM available for dynamic allocation:
I (xxx) heap_init: At 3FF80000 len 0001F800 (126 KiB): DRAM
I (xxx) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (xxx) heap_init: At 3FFB0000 len 00008000 (32 KiB): DRAM
```
✅ **Нормально:** Показывает доступную память  
⚠️ **Проблема:** Если памяти мало - возможны проблемы

---

#### 2. **Инициализация дисплея**
```
I (xxx) app_main: ++Display LVGL demo
I (xxx) bsp_display: Display initialized
```
✅ **Нормально:** Дисплей инициализирован  
❌ **Проблема:** Если нет - проверьте подключение LCD

---

#### 3. **Инициализация энкодера**
```
I (xxx) app_main: Encoder GPIO initialized
I (xxx) app_main: Encoder manager initialized
I (xxx) app_main: Encoder driver initialized
I (xxx) EncoderManager: Encoder manager task started
```
✅ **Нормально:** Все компоненты энкодера инициализированы  
❌ **Проблема:** Если нет - проверьте GPIO пины

---

#### 4. **LVGL и задачи**
```
I (xxx) app_main: All tasks created successfully
```
✅ **Нормально:** Все задачи FreeRTOS созданы  
⚠️ **Проверьте:** Размеры стеков задач

---

#### 5. **Навигация экранов**
```
I (xxx) SCREEN_NAV: Initializing screen navigation
I (xxx) SCREEN_NAV: Navigating to screen: X
I (xxx) Main_Menu_main: Главное меню успешно инициализировано
```
✅ **Нормально:** Навигация работает  
❌ **Проблема:** Если зависает - возможна утечка памяти

---

### ⚠️ КРИТИЧЕСКИЕ ОШИБКИ

#### 1. **Переполнение стека**
```
E (xxx) task_wdt: Task watchdog got triggered. The following tasks did not reset the watchdog in time:
E (xxx) task_wdt:   - lvgl_timers (CPU 0)
```
❌ **Проблема:** Стек задачи переполнен  
🔧 **Решение:** Увеличить размер стека в `main.c`

---

#### 2. **Нехватка памяти**
```
E (xxx) heap: Failed to allocate memory
W (xxx) heap: Free heap size: XXXXX bytes
```
❌ **Проблема:** Недостаточно памяти  
🔧 **Решение:** 
- Проверить утечки памяти
- Увеличить размер PSRAM
- Оптимизировать использование памяти

---

#### 3. **Ошибки LVGL**
```
E (xxx) LVGL: Failed to allocate draw buffer
E (xxx) LVGL: Invalid object
```
❌ **Проблема:** Проблемы с LVGL  
🔧 **Решение:**
- Проверить размер буферов LVGL
- Убедиться, что объекты не удаляются дважды

---

#### 4. **GPIO ошибки**
```
E (xxx) gpio: GPIO pin error
E (xxx) gpio: Invalid GPIO number
```
❌ **Проблема:** Неправильная конфигурация GPIO  
🔧 **Решение:** Проверить пины в `esp32_s3_lcd_ev_board.h`

---

### 📊 Мониторинг производительности

#### Проверка использования памяти:
Добавьте в код для мониторинга:
```c
void print_memory_info(void) {
    ESP_LOGI("MEM", "Free heap: %d bytes", esp_get_free_heap_size());
    ESP_LOGI("MEM", "Min free heap: %d bytes", esp_get_minimum_free_heap_size());
    #ifdef CONFIG_SPIRAM
    ESP_LOGI("MEM", "SPIRAM free: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    #endif
}
```

#### Проверка стека задач:
```c
UBaseType_t stack = uxTaskGetStackHighWaterMark(NULL);
ESP_LOGI("TASK", "Stack remaining: %d bytes", stack * sizeof(StackType_t));
```

---

### 🔍 Типичные проблемы и решения

#### Проблема: Контроллер не отвечает
**Решение:**
1. Проверьте подключение USB кабеля
2. Проверьте COM порт: `Get-PnpDevice -Class Ports`
3. Попробуйте другой USB порт
4. Нажмите кнопку RESET на плате

---

#### Проблема: Ошибка загрузки
```
A fatal error occurred: Failed to connect to ESP32-S3
```
**Решение:**
1. Убедитесь, что контроллер в режиме загрузки (нажмите BOOT + RESET)
2. Проверьте драйверы USB-to-Serial
3. Попробуйте другой COM порт

---

#### Проблема: Медленная работа интерфейса
**Причины:**
- Недостаточно памяти для буферов LVGL
- Слишком низкая частота CPU
- Проблемы с PSRAM

**Решение:**
- Увеличить размер буферов LVGL
- Увеличить частоту CPU до 240 MHz
- Проверить скорость PSRAM

---

### 📝 Полезные команды

#### Сохранение логов в файл:
```powershell
idf.py -p COM3 monitor | Tee-Object -FilePath logs.txt
```

#### Фильтрация логов по тегу:
В мониторе используйте фильтры:
```
--print-filter="*:I app_main:V SCREEN_NAV:V"
```

#### Очистка экрана в мониторе:
Нажмите `Ctrl+L`

---

### 🎯 Чек-лист при анализе логов

- [ ] Система загрузилась без ошибок
- [ ] Дисплей инициализирован
- [ ] Энкодер работает
- [ ] Все задачи созданы
- [ ] Нет ошибок памяти
- [ ] Нет переполнений стека
- [ ] LVGL работает корректно
- [ ] Навигация между экранами работает
- [ ] Нет утечек памяти (проверить через время)

---

## 🔧 Расширенный мониторинг

### Добавление отладочной информации

В `main.c` можно добавить периодический вывод информации:
```c
void print_system_info(void) {
    ESP_LOGI(TAG, "=== System Info ===");
    ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "Min free: %d bytes", esp_get_minimum_free_heap_size());
    ESP_LOGI(TAG, "Largest free block: %d bytes", 
             heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
}
```

Вызывайте эту функцию периодически для мониторинга.

---

## 📚 Дополнительные ресурсы

- [ESP-IDF Logging](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/logging.html)
- [Heap Memory Debugging](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/heap-debugging.html)
- [FreeRTOS Task Monitoring](https://www.freertos.org/uxTaskGetStackHighWaterMark.html)


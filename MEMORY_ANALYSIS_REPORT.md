# Отчет по анализу памяти проекта ESP32-S3 LCD

## Дата анализа: 2024
## Версия ESP-IDF: 5.5.1
## Версия LVGL: 9.x

---

## 🔴 КРИТИЧЕСКИЕ ПРОБЛЕМЫ

### 1. Утечка памяти: Очередь FreeRTOS не удаляется
**Файл:** `main/encoder/encoder_manager.c`

**Проблема:**
```c
static QueueHandle_t encoder_queue = NULL;

void encoder_manager_init(void) {
    if (encoder_queue == NULL) {
        encoder_queue = xQueueCreate(20, sizeof(encoder_event_t));
        // ...
    }
}
```

Очередь создается, но **никогда не удаляется**. Это приводит к утечке памяти при перезапуске приложения или при деинициализации.

**Решение:**
Добавить функцию деинициализации:
```c
void encoder_manager_deinit(void) {
    if (encoder_queue != NULL) {
        vQueueDelete(encoder_queue);
        encoder_queue = NULL;
    }
    enc_unregister_event();
    current_callback = NULL;
}
```

---

### 2. Утечка памяти: Статические стили LVGL не освобождаются
**Файлы:**
- `main/screens/S_Pass/password_screen.c` (5 статических стилей)
- `main/scale_logic_time/time_scale.c` (1 статический стиль)
- `main/menu_layer/main_menu/main_menu.c` (статический стиль в функции)
- `main/menu_layer/CO_Menu/CO_main_menu.c` (статический стиль в функции)
- `main/menu_layer/In_Out_Menu/In_Out_main_menu.c` (статический стиль в функции)

**Проблема:**
Стили LVGL создаются через `lv_style_init()`, но **никогда не освобождаются** через `lv_style_deinit()`.

**Пример:**
```c
// password_screen.c
static lv_style_t style_active_digit;
static lv_style_t style_active_roller;
static lv_style_t style_inactive_roller;
static lv_style_t style_inactive_central_digit;
static lv_style_t style_transparent_bg;

// В функции password_screen():
lv_style_init(&style_active_digit);
// ... но lv_style_deinit() никогда не вызывается!
```

**Решение:**
Добавить очистку стилей в функции cleanup:
```c
void password_screen_cleanup(void) {
    // ... существующий код ...
    
    // Освобождаем стили
    lv_style_deinit(&style_active_digit);
    lv_style_deinit(&style_active_roller);
    lv_style_deinit(&style_inactive_roller);
    lv_style_deinit(&style_inactive_central_digit);
    lv_style_deinit(&style_transparent_bg);
}
```

---

## ⚠️ ПРОБЛЕМЫ СРЕДНЕЙ ВАЖНОСТИ

### 3. Потенциальная проблема: Статические указатели на LVGL объекты
**Файлы:**
- `main/screen_logic/screen_navigation.c`
- `main/menu_layer/main_menu/main_menu.c`
- `main/menu_layer/CO_Menu/CO_main_menu.c`
- `main/menu_layer/In_Out_Menu/In_Out_main_menu.c`
- `main/scale_logic_time/time_scale.c`

**Проблема:**
Статические указатели на LVGL объекты могут стать невалидными после удаления объектов, но код проверяет валидность через `lv_obj_is_valid()`.

**Рекомендация:**
✅ Код уже использует проверки `lv_obj_is_valid()`, что хорошо. Однако стоит убедиться, что все указатели обнуляются после удаления объектов.

**Пример хорошей практики (уже используется):**
```c
if (is_obj_valid(co_mask)) {
    lv_obj_del(co_mask);
    co_mask = NULL;  // ✅ Правильно - обнуляем указатель
}
```

---

### 4. Потенциальная проблема: Использование lv_obj_del vs lv_obj_del_async
**Файлы:** Множество файлов

**Проблема:**
В некоторых местах используется `lv_obj_del()` (синхронное удаление), в других - `lv_obj_del_async()` (асинхронное удаление).

**Рекомендация:**
- Использовать `lv_obj_del_async()` в критических секциях или когда удаление происходит из обработчиков событий
- Использовать `lv_obj_del()` только когда это безопасно (не из обработчиков событий)

**Текущее состояние:**
- ✅ `screen_navigation.c` использует `lv_obj_del_async()` - правильно
- ✅ `password_screen.c` использует `lv_obj_del_async()` - правильно
- ⚠️ Некоторые меню используют `lv_obj_del()` - может быть проблемой в некоторых случаях

---

## ✅ ПОЛОЖИТЕЛЬНЫЕ МОМЕНТЫ

### 5. Правильное использование lv_malloc/lv_free
**Файл:** `main/screen_logic/arc_menu.c`

**Хорошая практика:**
```c
custom_obj_data_t *data = (custom_obj_data_t*)lv_malloc(sizeof(custom_obj_data_t));
// ...
lv_obj_add_event_cb(obj, free_custom_data, LV_EVENT_DELETE, NULL);

void free_custom_data(lv_event_t *e) {
    // ...
    lv_free(data);  // ✅ Правильно использует lv_free
}
```

✅ Используется правильный аллокатор LVGL (`lv_malloc`/`lv_free`) для данных, связанных с объектами LVGL.

---

### 6. Правильная проверка валидности объектов
**Файлы:** Множество файлов

**Хорошая практика:**
Код везде проверяет валидность объектов перед использованием:
```c
if (obj && lv_obj_is_valid(obj)) {
    // безопасная работа с объектом
}
```

✅ Это предотвращает обращение к удаленным объектам.

---

## 📊 АНАЛИЗ ИСПОЛЬЗОВАНИЯ ПАМЯТИ

### 7. Использование PSRAM для LVGL буферов
**Конфигурация:** `sdkconfig` и `components/esp_lvgl_port/src/lvgl9/esp_lvgl_port_disp.c`

**Текущее состояние:**
```c
// esp_lvgl_port_disp.c:218
draw_buf_alloc_caps |= MALLOC_CAP_SPIRAM;  // ✅ Используется PSRAM
```

✅ Буферы дисплея LVGL правильно выделяются в PSRAM, что экономит внутреннюю SRAM.

**Рекомендация:**
Убедитесь, что в `sdkconfig` включено:
```
CONFIG_SPIRAM=y
CONFIG_SPIRAM_MODE_OCT=y
```

---

### 8. Размеры стеков задач
**Файл:** `main/main.c`

**Текущие размеры:**
- `rotary_encoder_task`: 6144 байт (6 KB)
- `encoder_manager`: 8192 байт (8 KB)
- `lvgl_timers`: 16384 байт (16 KB)

**Анализ:**
- ✅ Размеры стеков выглядят разумными
- ⚠️ Стеки выделяются из внутренней SRAM (не из PSRAM)
- 💡 Если возникают переполнения стека, можно увеличить размеры или оптимизировать код

**Рекомендация:**
Мониторить использование стека через `uxTaskGetStackHighWaterMark()`:
```c
UBaseType_t stack_remaining = uxTaskGetStackHighWaterMark(NULL);
ESP_LOGI(TAG, "Stack remaining: %d bytes", stack_remaining * sizeof(StackType_t));
```

---

## 🔧 РЕКОМЕНДАЦИИ ПО ИСПРАВЛЕНИЮ

### Приоритет 1 (Критично):
1. ✅ Добавить `encoder_manager_deinit()` и вызывать при деинициализации
2. ✅ Добавить `lv_style_deinit()` для всех статических стилей в cleanup функциях

### Приоритет 2 (Важно):
3. ✅ Убедиться, что все указатели обнуляются после удаления объектов
4. ✅ Добавить мониторинг использования памяти и стека

### Приоритет 3 (Рекомендуется):
5. ✅ Рассмотреть использование `lv_obj_del_async()` везде, где возможно
6. ✅ Добавить проверки на переполнение стека

---

## 📝 ПРИМЕРЫ ИСПРАВЛЕНИЙ

### Исправление 1: Деинициализация менеджера энкодера

**Файл:** `main/encoder/encoder_manager.h`
```c
// Добавить объявление
void encoder_manager_deinit(void);
```

**Файл:** `main/encoder/encoder_manager.c`
```c
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
    
    ESP_LOGI(TAG, "Encoder manager deinitialized");
}
```

### Исправление 2: Очистка стилей в password_screen

**Файл:** `main/screens/S_Pass/password_screen.c`
```c
void password_screen_cleanup(void) {
    ESP_LOGI(TAG, "Starting password screen cleanup");
    
    // ... существующий код удаления объектов ...
    
    // ОСВОБОЖДАЕМ СТИЛИ
    if (password_screen_active) {
        lv_style_deinit(&style_active_digit);
        lv_style_deinit(&style_active_roller);
        lv_style_deinit(&style_inactive_roller);
        lv_style_deinit(&style_inactive_central_digit);
        lv_style_deinit(&style_transparent_bg);
        ESP_LOGI(TAG, "Password screen styles deinitialized");
    }
    
    // ... остальной код ...
}
```

### Исправление 3: Очистка стиля в time_scale

**Файл:** `main/scale_logic_time/time_scale.c`
```c
// Добавить функцию очистки
void time_scale_cleanup(void) {
    if (scale) {
        lv_obj_del(scale);
        scale = NULL;
    }
    if (scale_marker) {
        lv_obj_del(scale_marker);
        scale_marker = NULL;
    }
    if (time_label) {
        lv_obj_del(time_label);
        time_label = NULL;
    }
    for (int i = 0; i < 2; i++) {
        if (lines[i]) {
            lv_obj_del(lines[i]);
            lines[i] = NULL;
        }
    }
    
    // ОСВОБОЖДАЕМ СТИЛЬ
    lv_style_deinit(&style_line);
    
    is_time_scale_visible = false;
}
```

---

## 📈 МОНИТОРИНГ ПАМЯТИ

### Рекомендуется добавить функции мониторинга:

```c
void print_memory_info(void) {
    ESP_LOGI("MEM", "=== Memory Info ===");
    
    // Свободная внутренняя память
    size_t free_internal = esp_get_free_internal_heap_size();
    ESP_LOGI("MEM", "Free internal heap: %d bytes", free_internal);
    
    // Свободная PSRAM
    #ifdef CONFIG_SPIRAM
    size_t free_spiram = esp_get_free_heap_size() - free_internal;
    ESP_LOGI("MEM", "Free SPIRAM: %d bytes", free_spiram);
    #endif
    
    // Минимальная свободная память
    size_t min_free = esp_get_minimum_free_heap_size();
    ESP_LOGI("MEM", "Min free heap: %d bytes", min_free);
    
    // LVGL память (если доступно)
    #if LV_USE_BUILTIN_MALLOC
    ESP_LOGI("MEM", "LVGL memory usage: %d bytes", lv_mem_get_size());
    #endif
}
```

---

## ✅ ЗАКЛЮЧЕНИЕ

**Общая оценка:** Код в целом написан хорошо, с правильными проверками валидности объектов. Однако есть несколько утечек памяти, которые нужно исправить.

**Основные проблемы:**
1. Очередь FreeRTOS не удаляется
2. Стили LVGL не освобождаются
3. Нет функции деинициализации для некоторых модулей

**Рекомендации:**
- Исправить критические утечки памяти (приоритет 1)
- Добавить мониторинг памяти для отслеживания проблем
- Рассмотреть добавление unit-тестов для проверки освобождения памяти

---

## 📚 ДОПОЛНИТЕЛЬНЫЕ РЕСУРСЫ

- [LVGL Memory Management](https://docs.lvgl.io/master/overview/memory.html)
- [ESP-IDF Heap Memory Debugging](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/heap-debugging.html)
- [FreeRTOS Memory Management](https://www.freertos.org/a00111.html)


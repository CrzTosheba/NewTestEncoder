#include "encoder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

// Время задержки для определения длительного удержания кнопки (в миллисекундах)
#define HOLD_DELAY (1000)

// Callback-функция для обработки событий
enc_event _event = NULL;

// Флаг наличия зарегистрированного обработчика событий
char _isEvent = 0;

// Параметры энкодера
int  _delay;      // Задержка между опросами (в миллисекундах)
int  _gpio_sw;    // GPIO для кнопки энкодера
int  _gpio_a;     // GPIO для канала A энкодера
int  _gpio_b;     // GPIO для канала B энкодера

// Регистрация обработчика событий
void enc_register_event(const enc_event e){
    _event = e;         // Сохраняем указатель на callback-функцию
    _isEvent = 1;       // Устанавливаем флаг наличия обработчика
}

// Отмена регистрации обработчика событий
void enc_unregister_event(){
    _isEvent = 0;       // Сбрасываем флаг наличия обработчика
}

// Позиции кнопки
typedef enum {
    KEY_POS_DOWN = 0,   // Кнопка нажата
    KEY_POS_UP = 1      // Кнопка отпущена
} key_pos_st_t;

// Переменные состояния кнопки
static uint8_t _key_val_prev = 1;  // Предыдущее состояние кнопки (по умолчанию - отпущена)
static uint32_t _hold = 0;         // Счетчик времени удержания

// Автомат обработки состояния кнопки
static uint8_t sw_avtomat(const uint8_t key_val_cur)
{
    uint8_t ret = ENC_NONE; // Возвращаемое значение (по умолчанию - нет события)
    uint8_t key_val_prev = _key_val_prev; // Сохраняем предыдущее состояние
    _key_val_prev = key_val_cur;          // Обновляем текущее состояние

    // Если оба состояния "отпущена" - ничего не делаем
    if(key_val_cur + key_val_prev == 2)
        return ret;

    // Рассчитываем количество тиков для определения длительного удержания
    int hticks = HOLD_DELAY / _delay;

    // Обработка отпускания кнопки (переход из нажатого состояния в отпущенное)
    if(key_val_cur > key_val_prev){
        ret = ret | ENC_CLICK; // Генерируем событие клика
        
        // Если кнопка удерживалась достаточно долго - добавляем событие длительного удержания
        if(_hold > hticks)
            ret = ret | ENC_LONG_HOLD;
        return ret;
    }

    // Обработка нажатия кнопки (переход из отпущенного состояния в нажатое)
    ret = ret | ENC_HOLD; // Генерируем событие удержания
    
    // Сброс счетчика удержания при новом нажатии
    if(key_val_prev == KEY_POS_UP){
        _hold = 0;
    } 

    // Инкремент счетчика времени удержания
    _hold ++;
    
    // Если достигли порога длительного удержания - добавляем соответствующее событие
    if(_hold % hticks == 0)
        ret = ret | ENC_LONG_HOLD;

    return ret;
}

// Переменные состояния энкодера
static uint8_t _rotary_prev = 0;    // Предыдущее стабильное состояние энкодера
static uint8_t _rotary_stable = 0;   // Текущее стабильное состояние (после фильтрации)
static uint8_t _stable_count = 0;    // Счетчик стабильных состояний

// Автомат обработки вращения энкодера
static uint8_t rotary_avtomat(const uint8_t rotary_a, const uint8_t rotary_b)
{
    uint8_t ret = ENC_NONE; // Возвращаемое значение (по умолчанию - нет события)
    
    // Формируем текущее состояние энкодера из двух битов:
    // Бит 1: состояние канала A
    // Бит 0: состояние канала B
    uint8_t rotary_cur = (rotary_a << 1) | rotary_b;
    
    // Фильтр дребезга контактов
    if(rotary_cur != _rotary_stable) {
        // Состояние изменилось - сбрасываем счетчик стабильности
        _rotary_stable = rotary_cur;
        _stable_count = 0;
        return ret;
    }

    // Учитываем только переходы 11->01 (справо) и 11->10 (влево) (один щелчок энкодера)
    if((_rotary_prev == 0x03) && (rotary_cur == 0x01))
    {
        ret = ENC_RIGHT;
    }
    else if((_rotary_prev == 0x03) && (rotary_cur == 0x02))
    {
        ret = ENC_LEFT;
    }
    
    // Сохраняем предыдущее состояние
    _rotary_prev = rotary_cur;

    return ret;
}

// Инициализация энкодера
void enc_init(int delay, int gpio_sw, int gpio_a, int gpio_b){
    _delay = delay;       // Сохраняем задержку между опросами
    _gpio_sw = gpio_sw;   // Сохраняем номер GPIO для кнопки
    _gpio_a = gpio_a;     // Сохраняем номер GPIO для канала A
    _gpio_b = gpio_b;     // Сохраняем номер GPIO для канала B
    
    // Инициализация начального состояния энкодера и кнопки
    _rotary_prev = (gpio_get_level(_gpio_a) << 1) | gpio_get_level(_gpio_b);
    _rotary_stable = _rotary_prev; // Стабильное состояние = текущему
    _key_val_prev = gpio_get_level(_gpio_sw); // Состояние кнопки
}

// Основной цикл обработки энкодера
void enc_loop(){
    while (1) {
        // Задержка между опросами состояния энкодера
        vTaskDelay(_delay / portTICK_PERIOD_MS);
        
        // Если обработчик не зарегистрирован - пропускаем итерацию
        if(!_isEvent) 
            continue;

        // Чтение текущего состояния пинов
        uint8_t sw = gpio_get_level(_gpio_sw);
        uint8_t a = gpio_get_level(_gpio_a);
        uint8_t b = gpio_get_level(_gpio_b);
        
        // Обработка состояния кнопки
        uint8_t kp_sw = sw_avtomat(sw);
        
        // Обработка вращения энкодера
        uint8_t kp_rot = rotary_avtomat(a, b);
        
        // Комбинирование событий от кнопки и вращения
        uint8_t full = kp_sw | kp_rot;

        // Фильтрация событий:
        // Отправляем только если есть значимое событие (не только HOLD)
        if(full && full != ENC_HOLD) {
            // Вызов callback-функции с комбинированным событием
            (*_event)(full);
        }
    }
}
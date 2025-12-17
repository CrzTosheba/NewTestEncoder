#ifndef CO_PARAMS_LIMITS_H
#define CO_PARAMS_LIMITS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Структура для пределов значений параметров
typedef struct {
    float min;
    float max;
    float step;
} param_limits_t;

// Структура для пределов int параметров
typedef struct {
    int min;
    int max;
    int step;
} param_limits_int_t;

// Пределы для параметров меню "Общие"
#define PARAM_LIMITS_GENERAL_COUNT 6
extern const param_limits_t co_general_param_limits[PARAM_LIMITS_GENERAL_COUNT];

// Пределы для параметров меню "График отопления"
#define PARAM_LIMITS_HEATING_GRAPH_COUNT 14
extern const param_limits_t co_heating_graph_param_limits_float[PARAM_LIMITS_HEATING_GRAPH_COUNT];
extern const param_limits_int_t co_heating_graph_param_limits_int[1]; // Для C1-Number

// Пределы для параметров меню "Насосы"
#define PARAM_LIMITS_PUMPS_INT_COUNT 17
extern const param_limits_int_t co_pumps_param_limits_int[PARAM_LIMITS_PUMPS_INT_COUNT];

// Пределы для параметров меню "Клапан"
#define PARAM_LIMITS_VALVE_INT_COUNT 3
#define PARAM_LIMITS_VALVE_FLOAT_COUNT 4
extern const param_limits_int_t co_valve_param_limits_int[PARAM_LIMITS_VALVE_INT_COUNT];
extern const param_limits_t co_valve_param_limits_float[PARAM_LIMITS_VALVE_FLOAT_COUNT];

// Пределы для параметров меню "Ручной режим"
#define PARAM_LIMITS_MANUAL_INT_COUNT 3
extern const param_limits_int_t co_manual_param_limits_int[PARAM_LIMITS_MANUAL_INT_COUNT];

// Пределы для параметров меню "Расписание" (8 параметров на день: часы и минуты для 2 периодов)
#define PARAM_LIMITS_SCHEDULE_INT_COUNT 8
extern const param_limits_int_t co_schedule_param_limits_int[PARAM_LIMITS_SCHEDULE_INT_COUNT];

// Пределы для параметров меню "Сухой ход"
#define PARAM_LIMITS_DRY_RUN_INT_COUNT 3
extern const param_limits_int_t co_dry_run_param_limits_int[PARAM_LIMITS_DRY_RUN_INT_COUNT];

// Пределы для параметров меню "Внешняя авария"
#define PARAM_LIMITS_EXT_ALARM_INT_COUNT 4
extern const param_limits_int_t co_ext_alarm_param_limits_int[PARAM_LIMITS_EXT_ALARM_INT_COUNT];

// Пределы для параметров меню "Обрыв датчика"
#define PARAM_LIMITS_SENSOR_BREAK_INT_COUNT 3
extern const param_limits_int_t co_sensor_break_param_limits_int[PARAM_LIMITS_SENSOR_BREAK_INT_COUNT];

// Пределы для параметров меню "Авар. отклонение"
#define PARAM_LIMITS_DEV_ALARM_INT_COUNT 5
#define PARAM_LIMITS_DEV_ALARM_FLOAT_COUNT 1
extern const param_limits_int_t co_dev_alarm_param_limits_int[PARAM_LIMITS_DEV_ALARM_INT_COUNT];
extern const param_limits_t co_dev_alarm_param_limits_float[PARAM_LIMITS_DEV_ALARM_FLOAT_COUNT];

#ifdef __cplusplus
}
#endif

#endif // CO_PARAMS_LIMITS_H


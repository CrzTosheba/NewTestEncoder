#ifndef GVS_DEV_ALARM_PARAMS_LIMITS_H
#define GVS_DEV_ALARM_PARAMS_LIMITS_H

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

// Пределы для параметров меню "Авар. отклонение" ГВС
#define PARAM_LIMITS_DEV_ALARM_GVS_INT_COUNT 5
#define PARAM_LIMITS_DEV_ALARM_GVS_FLOAT_COUNT 1
extern const param_limits_int_t gvs_dev_alarm_param_limits_int[PARAM_LIMITS_DEV_ALARM_GVS_INT_COUNT];
extern const param_limits_t gvs_dev_alarm_param_limits_float[PARAM_LIMITS_DEV_ALARM_GVS_FLOAT_COUNT];

#ifdef __cplusplus
}
#endif

#endif // GVS_DEV_ALARM_PARAMS_LIMITS_H


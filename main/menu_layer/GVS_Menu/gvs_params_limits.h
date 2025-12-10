#ifndef GVS_PARAMS_LIMITS_H
#define GVS_PARAMS_LIMITS_H

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

// Пределы для параметров меню "Общие" ГВС
#define PARAM_LIMITS_GENERAL_GVS_COUNT 6
extern const param_limits_t gvs_general_param_limits[PARAM_LIMITS_GENERAL_GVS_COUNT];

// Пределы для параметров меню "Расписание" ГВС (8 параметров на день: часы и минуты для 2 периодов)
#define PARAM_LIMITS_SCHEDULE_GVS_INT_COUNT 8
extern const param_limits_int_t gvs_schedule_param_limits_int[PARAM_LIMITS_SCHEDULE_GVS_INT_COUNT];

#ifdef __cplusplus
}
#endif

#endif // GVS_PARAMS_LIMITS_H


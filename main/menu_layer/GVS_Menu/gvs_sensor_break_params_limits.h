#ifndef GVS_SENSOR_BREAK_PARAMS_LIMITS_H
#define GVS_SENSOR_BREAK_PARAMS_LIMITS_H

#include <stdint.h>
#include "menu_layer/CO_Menu/co_params_limits.h" // Используем общую структуру param_limits_int_t

#ifdef __cplusplus
extern "C" {
#endif

// Пределы для параметров меню "Обрыв датчика" ГВС
#define PARAM_LIMITS_SENSOR_BREAK_GVS_INT_COUNT 3
extern const param_limits_int_t gvs_sensor_break_param_limits_int[PARAM_LIMITS_SENSOR_BREAK_GVS_INT_COUNT];

#ifdef __cplusplus
}
#endif

#endif // GVS_SENSOR_BREAK_PARAMS_LIMITS_H


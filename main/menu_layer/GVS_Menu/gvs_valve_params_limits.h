#ifndef GVS_VALVE_PARAMS_LIMITS_H
#define GVS_VALVE_PARAMS_LIMITS_H

#include <stdint.h>
#include "menu_layer/CO_Menu/co_params_limits.h" // Используем общую структуру param_limits_t и param_limits_int_t

#ifdef __cplusplus
extern "C" {
#endif

// Пределы для параметров меню "Клапан" ГВС
#define PARAM_LIMITS_GVS_VALVE_INT_COUNT 3
#define PARAM_LIMITS_GVS_VALVE_FLOAT_COUNT 4
extern const param_limits_int_t gvs_valve_param_limits_int[PARAM_LIMITS_GVS_VALVE_INT_COUNT];
extern const param_limits_t gvs_valve_param_limits_float[PARAM_LIMITS_GVS_VALVE_FLOAT_COUNT];

#ifdef __cplusplus
}
#endif

#endif // GVS_VALVE_PARAMS_LIMITS_H


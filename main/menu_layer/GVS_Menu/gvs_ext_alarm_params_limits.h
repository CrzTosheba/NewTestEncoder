#ifndef GVS_EXT_ALARM_PARAMS_LIMITS_H
#define GVS_EXT_ALARM_PARAMS_LIMITS_H

#include <stdint.h>
#include "menu_layer/CO_Menu/co_params_limits.h" // Используем общую структуру param_limits_int_t

#ifdef __cplusplus
extern "C" {
#endif

// Пределы для параметров меню "Внешняя авария" ГВС
#define PARAM_LIMITS_EXT_ALARM_GVS_INT_COUNT 4
extern const param_limits_int_t gvs_ext_alarm_param_limits_int[PARAM_LIMITS_EXT_ALARM_GVS_INT_COUNT];

#ifdef __cplusplus
}
#endif

#endif // GVS_EXT_ALARM_PARAMS_LIMITS_H


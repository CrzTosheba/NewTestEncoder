#ifndef GVS_PUMPS_PARAMS_LIMITS_H
#define GVS_PUMPS_PARAMS_LIMITS_H

#include <stdint.h>
#include "menu_layer/CO_Menu/co_params_limits.h" // Используем общую структуру param_limits_int_t

#ifdef __cplusplus
extern "C" {
#endif

// Пределы для параметров меню "Насосы" ГВС
#define PARAM_LIMITS_GVS_PUMPS_INT_COUNT 17
extern const param_limits_int_t gvs_pumps_param_limits_int[PARAM_LIMITS_GVS_PUMPS_INT_COUNT];

#ifdef __cplusplus
}
#endif

#endif // GVS_PUMPS_PARAMS_LIMITS_H


#ifndef GVS_DRY_RUN_PARAMS_LIMITS_H
#define GVS_DRY_RUN_PARAMS_LIMITS_H

#include <stdint.h>
#include "menu_layer/CO_Menu/co_params_limits.h" // Используем общую структуру param_limits_int_t

#ifdef __cplusplus
extern "C" {
#endif

// Пределы для параметров меню "Сухой ход" ГВС
#define PARAM_LIMITS_DRY_RUN_GVS_INT_COUNT 3
extern const param_limits_int_t gvs_dry_run_param_limits_int[PARAM_LIMITS_DRY_RUN_GVS_INT_COUNT];

#ifdef __cplusplus
}
#endif

#endif // GVS_DRY_RUN_PARAMS_LIMITS_H


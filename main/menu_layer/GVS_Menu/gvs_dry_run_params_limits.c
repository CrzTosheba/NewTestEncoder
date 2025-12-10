#include "gvs_dry_run_params_limits.h"

// Пределы для параметров сухого хода ГВС (3 параметра: int)
// Порядок: GVS_PS_EnAlarm, GVS_PS_AlarmDelay, GVS_PS_AlarmRType
const param_limits_int_t gvs_dry_run_param_limits_int[PARAM_LIMITS_DRY_RUN_GVS_INT_COUNT] = {
    {0, 1, 1},              // GVS_PS_EnAlarm (0-1, enum: НЕТ/ДА)
    {0, 3600, 1},           // GVS_PS_AlarmDelay (0-3600 с)
    {0, 11, 1}              // GVS_PS_AlarmRType (0-11, enum: АВТО, РУЧН, РУЧН-1, ..., РУЧН-10)
};


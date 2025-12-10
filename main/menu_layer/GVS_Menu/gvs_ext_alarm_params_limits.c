#include "gvs_ext_alarm_params_limits.h"

// Пределы для параметров внешней аварии ГВС (4 параметра: int)
// Порядок: GVS_N1_EnExtAlarm, GVS_N2_EnExtAlarm, GVS_N_ExtAlarmDelay, GVS_N_ExtAlarmRType
const param_limits_int_t gvs_ext_alarm_param_limits_int[PARAM_LIMITS_EXT_ALARM_GVS_INT_COUNT] = {
    {0, 1, 1},              // GVS_N1_EnExtAlarm (0-1, enum: НЕТ/ДА)
    {0, 1, 1},              // GVS_N2_EnExtAlarm (0-1, enum: НЕТ/ДА)
    {0, 3600, 1},           // GVS_N_ExtAlarmDelay (0-3600 с)
    {0, 11, 1}              // GVS_N_ExtAlarmRType (0-11, enum: АВТО, РУЧН, РУЧН-1, ..., РУЧН-10)
};


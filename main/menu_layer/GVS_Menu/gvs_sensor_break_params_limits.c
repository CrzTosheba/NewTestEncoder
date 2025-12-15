#include "gvs_sensor_break_params_limits.h"

// Пределы для параметров обрыва датчика ГВС (3 параметра: int)
// Порядок: GVS_T1_EnAlarm, GVS_AIAlarmDelay, GVS_T1_AlarmRType
const param_limits_int_t gvs_sensor_break_param_limits_int[PARAM_LIMITS_SENSOR_BREAK_GVS_INT_COUNT] = {
    {0, 1, 1},              // GVS_T1_EnAlarm (0-1, enum: НЕТ/ДА)
    {0, 3600, 1},           // GVS_AIAlarmDelay (0-3600 с)
    {0, 11, 1}              // GVS_T1_AlarmRType (0-11, enum: АВТО, РУЧН, РУЧН-1, ..., РУЧН-10)
};


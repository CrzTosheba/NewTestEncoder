#include "gvs_dev_alarm_params_limits.h"

// Пределы для параметров аварийного отклонения ГВС (5 int + 1 float)
// Порядок int: GVS_T1_EnDevAlarm, GVS_T1_EnHighAlarm, GVS_T1_EnLowAlarm, GVS_T1_DevAlarmDelay, GVS_T1_DevAlarmRType
const param_limits_int_t gvs_dev_alarm_param_limits_int[PARAM_LIMITS_DEV_ALARM_GVS_INT_COUNT] = {
    {0, 1, 1},              // GVS_T1_EnDevAlarm (0-1, enum: НЕТ/ДА)
    {0, 1, 1},              // GVS_T1_EnHighAlarm (0-1, enum: НЕТ/ДА)
    {0, 1, 1},              // GVS_T1_EnLowAlarm (0-1, enum: НЕТ/ДА)
    {0, 3600, 1},           // GVS_T1_DevAlarmDelay (0-3600 с)
    {0, 11, 1}              // GVS_T1_DevAlarmRType (0-11, enum: АВТО, РУЧН, РУЧН-1, ..., РУЧН-10)
};

// Порядок float: GVS_T1_AlarmDev
const param_limits_t gvs_dev_alarm_param_limits_float[PARAM_LIMITS_DEV_ALARM_GVS_FLOAT_COUNT] = {
    {0.0f, 60.0f, 0.1f}     // GVS_T1_AlarmDev (0.0-60.0 °C, шаг 0.1)
};


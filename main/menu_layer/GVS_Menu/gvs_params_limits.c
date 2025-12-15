#include "gvs_params_limits.h"

// Пределы для параметров меню "Общие" ГВС
const param_limits_t gvs_general_param_limits[PARAM_LIMITS_GENERAL_GVS_COUNT] = {
    {0, 0, 0},              // Mode (enum, не используется)
    {10.0f, 150.0f, 0.1f},  // GVS_T1-Econom (10.0-150.0 °C)
    {10.0f, 150.0f, 0.1f},  // GVS_T1-Comfort (10.0-150.0 °C)
    {10.0f, 150.0f, 0.1f},  // GVS_T1-Standby (10.0-150.0 °C)
    {5.0f, 150.0f, 0.1f},   // GVS_T1-DesiredMax (5.0-150.0 °C)
    {5.0f, 150.0f, 0.1f},   // GVS_T1-DesiredMin (5.0-150.0 °C)
};

// Пределы для параметров меню "Расписание" ГВС
const param_limits_int_t gvs_schedule_param_limits_int[PARAM_LIMITS_SCHEDULE_GVS_INT_COUNT] = {
    {0, 23, 1},             // HoursFrom1 (0-23 ч)
    {0, 23, 1},             // HoursTo1 (0-23 ч)
    {0, 59, 1},             // MinFrom1 (0-59 мин)
    {0, 59, 1},             // MinTo1 (0-59 мин)
    {0, 23, 1},             // HoursFrom2 (0-23 ч)
    {0, 23, 1},             // HoursTo2 (0-23 ч)
    {0, 59, 1},             // MinFrom2 (0-59 мин)
    {0, 59, 1}              // MinTo2 (0-59 мин)
};



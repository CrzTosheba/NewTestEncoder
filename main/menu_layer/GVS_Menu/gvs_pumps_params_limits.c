#include "gvs_pumps_params_limits.h"

// Пределы для параметров насосов ГВС (int)
const param_limits_int_t gvs_pumps_param_limits_int[PARAM_LIMITS_GVS_PUMPS_INT_COUNT] = {
    {0, 2, 1},              // GVS_N_Number (НЕТ/1/2 насоса)
    {0, 3600, 1},           // GVS_N_BeforeStartPause (0-3600 с)
    {0, 3600, 1},           // GVS_N_BeforeStopPause (0-3600 с)
    {0, 3600, 1},           // GVS_N_ChangeOverPause (0-3600 с)
    {0, 1, 1},              // GVS_N_ChangeMode (enum: 0-1)
    {1, 360, 1},            // GVS_N_ChangeWHours (1-360 ч)
    {0, 360, 1},            // GVS_N_ChangeWDays (0-360 сут)
    {0, 23, 1},             // GVS_N_ChangeHours (0-23 ч)
    {0, 59, 1},             // GVS_N_ChangeMinutes (0-59 мин)
    {0, 1, 1},              // GVS_N1_ResetWHours (enum: 0-1)
    {0, 999999, 1},         // GVS_N1_WHours (0-999999 ч, только для отображения)
    {0, 999999, 1},         // GVS_N1_WStarts (0-999999, только для отображения)
    {0, 1, 1},              // GVS_N2_ResetWHours (enum: 0-1)
    {0, 999999, 1},         // GVS_N2_WHours (0-999999 ч, только для отображения)
    {0, 999999, 1},         // GVS_N2_WStarts (0-999999, только для отображения)
    {0, 1, 1},              // GVS_N_Training_En (enum: 0-1)
    {0, 60, 1}              // GVS_N_Training_Period (0-60 с)
};


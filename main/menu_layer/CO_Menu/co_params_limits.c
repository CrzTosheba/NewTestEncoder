#include "co_params_limits.h"

// Пределы для параметров меню "Общие"
const param_limits_t co_general_param_limits[PARAM_LIMITS_GENERAL_COUNT] = {
    {0, 0, 0},              // Mode (enum, не используется)
    {10.0f, 150.0f, 0.1f},  // T1-Econom (10.0-150.0 °C)
    {10.0f, 150.0f, 0.1f},  // T1-Comfort (10.0-150.0 °C)
    {10.0f, 150.0f, 0.1f},  // T1-Standby (10.0-150.0 °C)
    {5.0f, 150.0f, 0.1f},   // T1-DesiredMax (5.0-150.0 °C)
    {5.0f, 150.0f, 0.1f},   // T1-DesiredMin (5.0-150.0 °C)
};

// Пределы для float параметров меню "График отопления"
// Порядок: C1-Slope, C1-T0-1..6, C1-T1-Desired-1..6, C3-T1-6, C3-T1-Desired-6
const param_limits_t co_heating_graph_param_limits_float[PARAM_LIMITS_HEATING_GRAPH_COUNT] = {
    {0.0f, 90.0f, 0.1f},     // C1-Slope (Угол наклона)
    {-50.0f, 50.0f, 0.1f},   // C1-T0-1 (Точка 1. Тнв)
    {0.0f, 100.0f, 0.1f},    // C1-T1-Desired-1 (Точка 1. Тпод_CO)
    {-50.0f, 50.0f, 0.1f},   // C1-T0-2 (Точка 2. Тнв)
    {0.0f, 100.0f, 0.1f},    // C1-T1-Desired-2 (Точка 2. Тпод_CO)
    {-50.0f, 50.0f, 0.1f},   // C1-T0-3 (Точка 3. Тнв)
    {0.0f, 100.0f, 0.1f},    // C1-T1-Desired-3 (Точка 3. Тпод_CO)
    {-50.0f, 50.0f, 0.1f},   // C1-T0-4 (Точка 4. Тнв)
    {0.0f, 100.0f, 0.1f},    // C1-T1-Desired-4 (Точка 4. Тпод_CO)
    {-50.0f, 50.0f, 0.1f},   // C1-T0-5 (Точка 5. Тнв)
    {0.0f, 100.0f, 0.1f},    // C1-T1-Desired-5 (Точка 5. Тпод_CO)
    {-50.0f, 50.0f, 0.1f},   // C1-T0-6 (Точка 6. Тнв)
    {0.0f, 100.0f, 0.1f},    // C1-T1-Desired-6 (Точка 6. Тпод_CO)
    {0.0f, 100.0f, 0.1f},    // C3-T1-6 (Точка 6. Тпод.тс)
    {0.0f, 100.0f, 0.1f},    // C3-T1-Desired-6 (Точка 6. Тпод_CO)
};

// Пределы для int параметров меню "График отопления"
const param_limits_int_t co_heating_graph_param_limits_int[1] = {
    {1, 6, 1},  // C1-Number (Количество точек)
};

// Пределы для параметров насосов (int)
const param_limits_int_t co_pumps_param_limits_int[PARAM_LIMITS_PUMPS_INT_COUNT] = {
    {0, 2, 1},              // N-Number (НЕТ/1/2 насоса)
    {0, 3600, 1},           // N-BeforeStartPause (0-3600 с)
    {0, 3600, 1},           // N-BeforeStopPause (0-3600 с)
    {0, 3600, 1},           // N-ChangeOverPause (0-3600 с)
    {0, 1, 1},              // N-ChangeMode (enum: 0-1)
    {1, 360, 1},            // N-ChangeWHours (1-360 ч)
    {0, 360, 1},            // N-ChangeWDays (0-360 сут)
    {0, 23, 1},             // N-ChangeHours (0-23 ч)
    {0, 59, 1},             // N-ChangeMinutes (0-59 мин)
    {0, 1, 1},              // N1-ResetWHours (enum: 0-1)
    {0, 999999, 1},         // N1-WHours (0-999999 ч, только для отображения)
    {0, 999999, 1},         // N1-WStarts (0-999999, только для отображения)
    {0, 1, 1},              // N2-ResetWHours (enum: 0-1)
    {0, 999999, 1},         // N2-WHours (0-999999 ч, только для отображения)
    {0, 999999, 1},         // N2-WStarts (0-999999, только для отображения)
    {0, 1, 1},              // N-Training-En (enum: 0-1)
    {0, 60, 1}              // N-Training-Period (0-60 с)
};

// Пределы для параметров клапан
const param_limits_int_t co_valve_param_limits_int[PARAM_LIMITS_VALVE_INT_COUNT] = {
    {0, 2, 1},              // M-RegType (0-2, enum: П/ПИ/ПИД)
    {0, 100, 1},            // M-Length (0-100, Длина штока, мм)
    {40, 1000, 1}           // M-IControl-Min (40-1000 мс)
};

const param_limits_t co_valve_param_limits_float[PARAM_LIMITS_VALVE_FLOAT_COUNT] = {
    {0.0f, 100.0f, 0.1f},   // M-Speed (0.0-100.0, Скорость, с/мм)
    {5.0f, 250.0f, 0.1f},   // M-PCoef (5.0-250.0, П-коэффициент)
    {1.0f, 999.0f, 0.1f},   // M-ICoef (1.0-999.0, И-коэффициент)
    {0.0f, 15.0f, 0.1f}     // M-Deadband (0.0-15.0, Нейтральная зона, °C)
};

// Пределы для параметров ручного режима (enum как int)
const param_limits_int_t co_manual_param_limits_int[PARAM_LIMITS_MANUAL_INT_COUNT] = {
    {0, 1, 1},              // N1-DControl (enum: 0-1)
    {0, 1, 1},              // N2-DControl (enum: 0-1)
    {0, 1, 1}               // M-IControl (enum: 0-1)
};

// Пределы для параметров расписания (8 параметров: часы и минуты для 2 периодов)
// Порядок: HoursFrom1, HoursTo1, MinFrom1, MinTo1, HoursFrom2, HoursTo2, MinFrom2, MinTo2
const param_limits_int_t co_schedule_param_limits_int[PARAM_LIMITS_SCHEDULE_INT_COUNT] = {
    {0, 23, 1},             // HoursFrom1 (0-23 ч)
    {0, 23, 1},             // HoursTo1 (0-23 ч)
    {0, 59, 1},             // MinFrom1 (0-59 мин)
    {0, 59, 1},             // MinTo1 (0-59 мин)
    {0, 23, 1},             // HoursFrom2 (0-23 ч)
    {0, 23, 1},             // HoursTo2 (0-23 ч)
    {0, 59, 1},             // MinFrom2 (0-59 мин)
    {0, 59, 1}              // MinTo2 (0-59 мин)
};

// Пределы для параметров сухого хода (3 параметра: int)
// Порядок: PS-EnAlarm, PS-AlarmDelay, PS-AlarmRType
const param_limits_int_t co_dry_run_param_limits_int[PARAM_LIMITS_DRY_RUN_INT_COUNT] = {
    {0, 1, 1},              // PS-EnAlarm (0-1, enum: НЕТ/ДА)
    {0, 3600, 1},           // PS-AlarmDelay (0-3600 с)
    {0, 11, 1}              // PS-AlarmRType (0-11, enum: АВТО, РУЧН, РУЧН-1, ..., РУЧН-10)
};

// Пределы для параметров внешней аварии (4 параметра: int)
// Порядок: N1-EnExtAlarm, N2-EnExtAlarm, N-ExtAlarmDelay, N-ExtAlarmRType
const param_limits_int_t co_ext_alarm_param_limits_int[PARAM_LIMITS_EXT_ALARM_INT_COUNT] = {
    {0, 1, 1},              // N1-EnExtAlarm (0-1, enum: НЕТ/ДА)
    {0, 1, 1},              // N2-EnExtAlarm (0-1, enum: НЕТ/ДА)
    {0, 3600, 1},           // N-ExtAlarmDelay (0-3600 с)
    {0, 11, 1}              // N-ExtAlarmRType (0-11, enum: АВТО, РУЧН, РУЧН-1, ..., РУЧН-10)
};

// Пределы для параметров обрыва датчика (3 параметра: int)
// Порядок: T1-EnAlarm, AIAlarmDelay, T1-AlarmRType
const param_limits_int_t co_sensor_break_param_limits_int[PARAM_LIMITS_SENSOR_BREAK_INT_COUNT] = {
    {0, 1, 1},              // T1-EnAlarm (0-1, enum: НЕТ/ДА)
    {0, 3600, 1},           // AIAlarmDelay (0-3600 с)
    {0, 11, 1}              // T1-AlarmRType (0-11, enum: АВТО, РУЧН, РУЧН-1, ..., РУЧН-10)
};

// Пределы для параметров аварийного отклонения (5 int + 1 float)
// Порядок int: T1-EnDevAlarm, T1-EnHighAlarm, T1-EnLowAlarm, T1-DevAlarmDelay, T1-DevAlarmRType
const param_limits_int_t co_dev_alarm_param_limits_int[PARAM_LIMITS_DEV_ALARM_INT_COUNT] = {
    {0, 1, 1},              // T1-EnDevAlarm (0-1, enum: НЕТ/ДА)
    {0, 1, 1},              // T1-EnHighAlarm (0-1, enum: НЕТ/ДА)
    {0, 1, 1},              // T1-EnLowAlarm (0-1, enum: НЕТ/ДА)
    {0, 3600, 1},           // T1-DevAlarmDelay (0-3600 с)
    {0, 11, 1}              // T1-DevAlarmRType (0-11, enum: АВТО, РУЧН, РУЧН-1, ..., РУЧН-10)
};

// Порядок float: T1-AlarmDev
const param_limits_t co_dev_alarm_param_limits_float[PARAM_LIMITS_DEV_ALARM_FLOAT_COUNT] = {
    {0.0f, 60.0f, 0.1f}     // T1-AlarmDev (0.0-60.0 °C, шаг 0.1)
};


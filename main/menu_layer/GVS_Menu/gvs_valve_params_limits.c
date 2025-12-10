#include "gvs_valve_params_limits.h"

// Пределы для параметров клапан ГВС (int)
const param_limits_int_t gvs_valve_param_limits_int[PARAM_LIMITS_GVS_VALVE_INT_COUNT] = {
    {0, 2, 1},              // GVS_M_RegType (M-RegType: enum: 0-2, П/ПИ/ПИД)
    {0, 100, 1},            // GVS_M_Length (M-Length: 0-100, Длина штока, мм)
    {40, 1000, 1}           // GVS_M_IControl_Min (M-IControl-Min: 40-1000 мс)
};

// Пределы для параметров клапан ГВС (float)
const param_limits_t gvs_valve_param_limits_float[PARAM_LIMITS_GVS_VALVE_FLOAT_COUNT] = {
    {0.0f, 100.0f, 0.1f},   // GVS_M_Speed (M-Speed: 0.0-100.0, Скорость, с/мм)
    {5.0f, 250.0f, 0.1f},   // GVS_M_PCoef (M-PCoef: 5.0-250.0, П-коэффициент)
    {1.0f, 999.0f, 0.1f},   // GVS_M_ICoef (M-ICoef: 1.0-999.0, И-коэффициент)
    {0.0f, 15.0f, 0.1f}     // GVS_M_Deadband (M-Deadband: 0.0-15.0, Нейтральная зона, °C)
};


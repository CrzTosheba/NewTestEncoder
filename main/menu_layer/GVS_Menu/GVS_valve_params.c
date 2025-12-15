#include "GVS_valve_params.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "GVS_VALVE_PARAMS";

// Глобальные переменные для параметров клапан ГВС
gvs_reg_type_t GVS_M_RegType = GVS_REG_TYPE_PI;        // M-RegType: По умолчанию ПИ
int GVS_M_Length = 10;                                  // M-Length: По умолчанию 10 (Длина штока, мм)
float GVS_M_Speed = 16.0f;                             // M-Speed: По умолчанию 16.0 (Скорость, с/мм)
float GVS_M_PCoef = 80.0f;                             // M-PCoef: По умолчанию 80.0 (П-коэффициент)
float GVS_M_ICoef = 30.0f;                              // M-ICoef: По умолчанию 30.0 (И-коэффициент)
float GVS_M_Deadband = 1.0f;                           // M-Deadband: По умолчанию 1.0°C
int GVS_M_IControl_Min = 200;                          // M-IControl-Min: По умолчанию 200 мс

// Функция инициализации параметров
void gvs_valve_params_init(void) {
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "GVS valve parameters initialized with default values");
    ESP_LOGI(TAG, "RegType=%d, Length=%d, Speed=%.1f, PCoef=%.1f, ICoef=%.1f, Deadband=%.1f, IControl_Min=%d",
             GVS_M_RegType, GVS_M_Length, GVS_M_Speed, GVS_M_PCoef, GVS_M_ICoef, GVS_M_Deadband, GVS_M_IControl_Min);
}


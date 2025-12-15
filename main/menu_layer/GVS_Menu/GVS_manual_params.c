#include "GVS_manual_params.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "GVS_MANUAL_PARAMS";

// Глобальные переменные для параметров ручного режима ГВС
gvs_manual_pump_t GVS_N1_DControl = GVS_MANUAL_PUMP_OFF;    // По умолчанию ВЫКЛ
gvs_manual_pump_t GVS_N2_DControl = GVS_MANUAL_PUMP_OFF;    // По умолчанию ВЫКЛ
gvs_manual_valve_t GVS_M_IControl = GVS_MANUAL_VALVE_STOP; // По умолчанию СТОП

// Функция инициализации параметров
void gvs_manual_params_init(void) {
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "GVS manual parameters initialized with default values");
    ESP_LOGI(TAG, "N1_DControl=%d, N2_DControl=%d, M_IControl=%d",
             GVS_N1_DControl, GVS_N2_DControl, GVS_M_IControl);
}


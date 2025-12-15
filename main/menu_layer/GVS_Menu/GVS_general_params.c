#include "GVS_general_params.h"
#include "esp_log.h"

static const char *TAG = "GVS_PARAMS";

// Глобальные переменные для параметров ГВС
gvs_mode_t GVS_Mode = GVS_MODE_COMF;        // По умолчанию КОМФ
float GVS_T1_Econom = 55.0f;                  // По умолчанию 5.0°C
float GVS_T1_Comfort = 65.0f;                // По умолчанию 65.0°C
float GVS_T1_Standby = 25.0f;                // По умолчанию 25.0°C
float GVS_T1_DesiredMax = 75.0f;            // По умолчанию 75.0°C
float GVS_T1_DesiredMin = 10.0f;            // По умолчанию 10.0°C

// Функция инициализации параметров (без сохранения в NVS)
void gvs_general_params_init(void) {
    ESP_LOGI(TAG, "GVS general parameters initialized with default values");
    ESP_LOGI(TAG, "Mode: %d, T1_Econom: %.1f, T1_Comfort: %.1f, T1_Standby: %.1f, T1_DesiredMax: %.1f, T1_DesiredMin: %.1f",
             GVS_Mode, GVS_T1_Econom, GVS_T1_Comfort, GVS_T1_Standby, GVS_T1_DesiredMax, GVS_T1_DesiredMin);
}


#include "GVS_dry_run_params.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "GVS_DRY_RUN_PARAMS";

// Глобальные переменные для параметров сухого хода ГВС
gvs_ps_enalarm_t GVS_PS_EnAlarm = GVS_PS_ENALARM_NO;           // По умолчанию НЕТ
int GVS_PS_AlarmDelay = 15;                                    // По умолчанию 15 с
gvs_ps_alarm_rtype_t GVS_PS_AlarmRType = GVS_PS_ALARM_RTYPE_MANUAL_3; // По умолчанию РУЧН-3

// Функция инициализации параметров
void gvs_dry_run_params_init(void) {
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "GVS dry run parameters initialized with default values");
    ESP_LOGI(TAG, "PS_EnAlarm=%d, PS_AlarmDelay=%d, PS_AlarmRType=%d",
             GVS_PS_EnAlarm, GVS_PS_AlarmDelay, GVS_PS_AlarmRType);
}


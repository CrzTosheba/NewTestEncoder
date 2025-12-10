#include "GVS_ext_alarm_params.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "GVS_EXT_ALARM_PARAMS";

// Глобальные переменные для параметров внешней аварии ГВС
gvs_ext_alarm_en_t GVS_N1_EnExtAlarm = GVS_EXT_ALARM_NO;           // По умолчанию НЕТ
gvs_ext_alarm_en_t GVS_N2_EnExtAlarm = GVS_EXT_ALARM_NO;           // По умолчанию НЕТ
int GVS_N_ExtAlarmDelay = 2;                                        // По умолчанию 2 с
gvs_ext_alarm_rtype_t GVS_N_ExtAlarmRType = GVS_EXT_ALARM_RTYPE_MANUAL_3; // По умолчанию РУЧН-3

// Функция инициализации параметров
void gvs_ext_alarm_params_init(void) {
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "GVS external alarm parameters initialized with default values");
    ESP_LOGI(TAG, "N1_EnExtAlarm=%d, N2_EnExtAlarm=%d, N_ExtAlarmDelay=%d, N_ExtAlarmRType=%d",
             GVS_N1_EnExtAlarm, GVS_N2_EnExtAlarm, GVS_N_ExtAlarmDelay, GVS_N_ExtAlarmRType);
}


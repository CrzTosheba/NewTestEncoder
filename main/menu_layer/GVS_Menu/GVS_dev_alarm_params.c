#include "GVS_dev_alarm_params.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "GVS_DEV_ALARM_PARAMS";

// Глобальные переменные для параметров аварийного отклонения ГВС
int GVS_T1_EnDevAlarm = 0;           // По умолчанию НЕТ
int GVS_T1_EnHighAlarm = 0;           // По умолчанию НЕТ
int GVS_T1_EnLowAlarm = 0;           // По умолчанию НЕТ
int GVS_T1_DevAlarmDelay = 600;      // По умолчанию 600 с
int GVS_T1_DevAlarmRType = 4;        // По умолчанию РУЧН-3 (значение 4)
float GVS_T1_AlarmDev = 20.0f;       // По умолчанию 20.0 °C

/**
 * @brief Инициализирует параметры аварийного отклонения ГВС
 */
void gvs_dev_alarm_params_init(void) {
    ESP_LOGI(TAG, "Initializing GVS deviation alarm parameters");
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "GVS deviation alarm parameters initialized with default values");
    ESP_LOGI(TAG, "T1_EnDevAlarm=%d, T1_EnHighAlarm=%d, T1_EnLowAlarm=%d, T1_DevAlarmDelay=%d, T1_DevAlarmRType=%d, T1_AlarmDev=%.1f",
             GVS_T1_EnDevAlarm, GVS_T1_EnHighAlarm, GVS_T1_EnLowAlarm, GVS_T1_DevAlarmDelay, GVS_T1_DevAlarmRType, GVS_T1_AlarmDev);
}


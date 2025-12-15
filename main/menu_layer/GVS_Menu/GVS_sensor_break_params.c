#include "GVS_sensor_break_params.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "GVS_SENSOR_BREAK_PARAMS";

// Глобальные переменные для параметров обрыва датчика ГВС
gvs_sensor_alarm_en_t GVS_T1_EnAlarm = GVS_SENSOR_ALARM_NO;           // По умолчанию НЕТ
int GVS_AIAlarmDelay = 15;                                             // По умолчанию 15 с
gvs_sensor_alarm_rtype_t GVS_T1_AlarmRType = GVS_SENSOR_ALARM_RTYPE_MANUAL_3; // По умолчанию РУЧН-3

// Функция инициализации параметров
void gvs_sensor_break_params_init(void) {
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "GVS sensor break parameters initialized with default values");
    ESP_LOGI(TAG, "T1_EnAlarm=%d, AIAlarmDelay=%d, T1_AlarmRType=%d",
             GVS_T1_EnAlarm, GVS_AIAlarmDelay, GVS_T1_AlarmRType);
}


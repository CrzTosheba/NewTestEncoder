#include "CO_sensor_break_params.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "CO_SENSOR_BREAK_PARAMS";
static const char *NVS_NAMESPACE = "co_sens_brk";

// Глобальные переменные для параметров обрыва датчика
int T1_EnAlarm = 0;        // По умолчанию НЕТ
int AIAlarmDelay = 15;     // По умолчанию 15 с
int T1_AlarmRType = 4;     // По умолчанию РУЧН-3

/**
 * @brief Сохраняет параметры в NVS
 */
void co_sensor_break_params_save(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to save parameters to NVS namespace: %s", NVS_NAMESPACE);
    
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    
    err = nvs_set_i32(nvs_handle, "T1_EnAlarm", T1_EnAlarm);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving T1_EnAlarm: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "AIAlarmDly", AIAlarmDelay);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving AIAlarmDelay: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "T1_AlarmRType", T1_AlarmRType);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving T1_AlarmRType: %s", esp_err_to_name(err));
    
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Parameters saved to NVS successfully");
    }
    
    nvs_close(nvs_handle);
}

/**
 * @brief Загружает параметры из NVS
 */
void co_sensor_break_params_load(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to load parameters from NVS namespace: %s", NVS_NAMESPACE);
    
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGI(TAG, "NVS namespace not found, using default values");
        } else {
            ESP_LOGW(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        }
        return;
    }
    
    int32_t int_val;
    err = nvs_get_i32(nvs_handle, "T1_EnAlarm", &int_val);
    if (err == ESP_OK) T1_EnAlarm = (int)int_val;
    err = nvs_get_i32(nvs_handle, "AIAlarmDly", &int_val);
    if (err == ESP_OK) AIAlarmDelay = (int)int_val;
    err = nvs_get_i32(nvs_handle, "T1_AlarmRType", &int_val);
    if (err == ESP_OK) T1_AlarmRType = (int)int_val;
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Parameters loaded from NVS successfully");
}

/**
 * @brief Инициализирует параметры обрыва датчика
 */
void co_sensor_break_params_init(void) {
    ESP_LOGI(TAG, "Initializing CO sensor break parameters");
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "CO sensor break parameters initialized with default values");
    ESP_LOGI(TAG, "T1_EnAlarm=%d, AIAlarmDelay=%d, T1_AlarmRType=%d",
             T1_EnAlarm, AIAlarmDelay, T1_AlarmRType);
}


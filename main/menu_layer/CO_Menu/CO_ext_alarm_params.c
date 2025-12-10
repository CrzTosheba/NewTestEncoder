#include "CO_ext_alarm_params.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "CO_EXT_ALARM_PARAMS";
static const char *NVS_NAMESPACE = "co_ext_alarm";

// Глобальные переменные для параметров внешней аварии
int N1_EnExtAlarm = 0;        // По умолчанию НЕТ
int N2_EnExtAlarm = 0;        // По умолчанию НЕТ
int N_ExtAlarmDelay = 2;       // По умолчанию 2 с
int N_ExtAlarmRType = 4;       // По умолчанию РУЧН-3

/**
 * @brief Сохраняет параметры в NVS
 */
void co_ext_alarm_params_save(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to save parameters to NVS namespace: %s", NVS_NAMESPACE);
    
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    
    err = nvs_set_i32(nvs_handle, "N1_EnExtAlm", N1_EnExtAlarm);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N1_EnExtAlarm: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "N2_EnExtAlm", N2_EnExtAlarm);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N2_EnExtAlarm: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "N_ExtAlmDly", N_ExtAlarmDelay);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_ExtAlarmDelay: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "N_ExtAlmRType", N_ExtAlarmRType);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_ExtAlarmRType: %s", esp_err_to_name(err));
    
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
void co_ext_alarm_params_load(void) {
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
    err = nvs_get_i32(nvs_handle, "N1_EnExtAlm", &int_val);
    if (err == ESP_OK) N1_EnExtAlarm = (int)int_val;
    err = nvs_get_i32(nvs_handle, "N2_EnExtAlm", &int_val);
    if (err == ESP_OK) N2_EnExtAlarm = (int)int_val;
    err = nvs_get_i32(nvs_handle, "N_ExtAlmDly", &int_val);
    if (err == ESP_OK) N_ExtAlarmDelay = (int)int_val;
    err = nvs_get_i32(nvs_handle, "N_ExtAlmRType", &int_val);
    if (err == ESP_OK) N_ExtAlarmRType = (int)int_val;
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Parameters loaded from NVS successfully");
}

/**
 * @brief Инициализирует параметры внешней аварии
 */
void co_ext_alarm_params_init(void) {
    ESP_LOGI(TAG, "Initializing CO external alarm parameters");
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "CO external alarm parameters initialized with default values");
    ESP_LOGI(TAG, "N1_EnExtAlarm=%d, N2_EnExtAlarm=%d, N_ExtAlarmDelay=%d, N_ExtAlarmRType=%d",
             N1_EnExtAlarm, N2_EnExtAlarm, N_ExtAlarmDelay, N_ExtAlarmRType);
}


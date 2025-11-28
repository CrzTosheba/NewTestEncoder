#include "CO_dev_alarm_params.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "CO_DEV_ALARM_PARAMS";
static const char *NVS_NAMESPACE = "co_dev_alarm";

// Глобальные переменные для параметров аварийного отклонения
int T1_EnDevAlarm = 0;
int T1_EnHighAlarm = 0;
int T1_EnLowAlarm = 0;
int T1_DevAlarmDelay = 0;
int T1_DevAlarmRType = 0;
float T1_AlarmDev = 0.0f;

/**
 * @brief Сохраняет параметры в NVS
 */
void co_dev_alarm_params_save(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to save parameters to NVS namespace: %s", NVS_NAMESPACE);
    
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    
    err = nvs_set_i32(nvs_handle, "T1_EnDevAlm", T1_EnDevAlarm);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving T1_EnDevAlarm: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "T1_EnHighAlm", T1_EnHighAlarm);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving T1_EnHighAlarm: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "T1_EnLowAlm", T1_EnLowAlarm);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving T1_EnLowAlarm: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "T1_DevAlmDly", T1_DevAlarmDelay);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving T1_DevAlarmDelay: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "T1_DevAlmRType", T1_DevAlarmRType);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving T1_DevAlarmRType: %s", esp_err_to_name(err));
    err = nvs_set_blob(nvs_handle, "T1_AlarmDev", &T1_AlarmDev, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving T1_AlarmDev: %s", esp_err_to_name(err));
    
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
void co_dev_alarm_params_load(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to load parameters from NVS namespace: %s", NVS_NAMESPACE);
    
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error opening NVS handle (namespace may not exist): %s", esp_err_to_name(err));
        return;
    }
    
    int32_t int_val;
    err = nvs_get_i32(nvs_handle, "T1_EnDevAlm", &int_val);
    if (err == ESP_OK) T1_EnDevAlarm = (int)int_val;
    err = nvs_get_i32(nvs_handle, "T1_EnHighAlm", &int_val);
    if (err == ESP_OK) T1_EnHighAlarm = (int)int_val;
    err = nvs_get_i32(nvs_handle, "T1_EnLowAlm", &int_val);
    if (err == ESP_OK) T1_EnLowAlarm = (int)int_val;
    err = nvs_get_i32(nvs_handle, "T1_DevAlmDly", &int_val);
    if (err == ESP_OK) T1_DevAlarmDelay = (int)int_val;
    err = nvs_get_i32(nvs_handle, "T1_DevAlmRType", &int_val);
    if (err == ESP_OK) T1_DevAlarmRType = (int)int_val;
    
    size_t required_size = sizeof(float);
    err = nvs_get_blob(nvs_handle, "T1_AlarmDev", &T1_AlarmDev, &required_size);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error loading T1_AlarmDev: %s", esp_err_to_name(err));
    }
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Parameters loaded from NVS successfully");
}

/**
 * @brief Инициализирует параметры аварийного отклонения
 */
void co_dev_alarm_params_init(void) {
    ESP_LOGI(TAG, "Initializing CO deviation alarm parameters");
    co_dev_alarm_params_load();
}


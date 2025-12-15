#include "CO_dry_run_params.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "CO_DRY_RUN_PARAMS";
static const char *NVS_NAMESPACE = "co_dry_run";

// Глобальные переменные для параметров сухого хода
co_ps_enalarm_t PS_EnAlarm = CO_PS_ENALARM_NO;           // По умолчанию НЕТ
int PS_AlarmDelay = 15;                                  // По умолчанию 15 с
co_ps_alarm_rtype_t PS_AlarmRType = CO_PS_ALARM_RTYPE_MANUAL_3; // По умолчанию РУЧН-3

/**
 * @brief Сохраняет параметры в NVS
 */
void co_dry_run_params_save(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to save parameters to NVS namespace: %s", NVS_NAMESPACE);
    
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    
    err = nvs_set_i32(nvs_handle, "PS_EnAlarm", (int32_t)PS_EnAlarm);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving PS_EnAlarm: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "PS_AlarmDly", PS_AlarmDelay);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving PS_AlarmDelay: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "PS_AlarmRType", (int32_t)PS_AlarmRType);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving PS_AlarmRType: %s", esp_err_to_name(err));
    
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
void co_dry_run_params_load(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to load parameters from NVS namespace: %s", NVS_NAMESPACE);
    
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error opening NVS handle (namespace may not exist): %s", esp_err_to_name(err));
        return;
    }
    
    int32_t int_val;
    err = nvs_get_i32(nvs_handle, "PS_EnAlarm", &int_val);
    if (err == ESP_OK) PS_EnAlarm = (co_ps_enalarm_t)int_val;
    err = nvs_get_i32(nvs_handle, "PS_AlarmDly", &int_val);
    if (err == ESP_OK) PS_AlarmDelay = (int)int_val;
    err = nvs_get_i32(nvs_handle, "PS_AlarmRType", &int_val);
    if (err == ESP_OK) PS_AlarmRType = (co_ps_alarm_rtype_t)int_val;
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Parameters loaded from NVS successfully");
}

/**
 * @brief Инициализирует параметры сухого хода
 */
void co_dry_run_params_init(void) {
    ESP_LOGI(TAG, "Initializing CO dry run parameters");
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "CO dry run parameters initialized with default values");
}


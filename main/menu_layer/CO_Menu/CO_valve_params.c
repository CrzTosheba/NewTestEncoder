#include "CO_valve_params.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "CO_VALVE_PARAMS";
static const char *NVS_NAMESPACE = "co_valve";  // Максимум 15 символов для NVS

// Глобальные переменные для параметров клапан
int M_ControlType = 0;                    // По умолчанию 0
int M_RegType = 0;                        // По умолчанию 0
int M_Length = 100;                       // По умолчанию 100 мм
float M_Speed = 1.0f;                     // По умолчанию 1.0 с/мм
float M_PCoef = 1.0f;                     // По умолчанию 1.0
float M_ICoef = 0.1f;                     // По умолчанию 0.1
float M_Deadband = 1.0f;                  // По умолчанию 1.0°C
int M_IControl_Min = 100;                 // По умолчанию 100 мс

/**
 * @brief Сохраняет параметры в NVS
 */
void co_valve_params_save(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to save parameters to NVS namespace: %s", NVS_NAMESPACE);
    
    // Открываем NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    
    // Сохраняем int параметры (как int32_t)
    err = nvs_set_i32(nvs_handle, "M_ControlType", M_ControlType);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving M_ControlType: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "M_RegType", M_RegType);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving M_RegType: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "M_Length", M_Length);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving M_Length: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "M_IControl_Min", M_IControl_Min);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving M_IControl_Min: %s", esp_err_to_name(err));
    
    // Сохраняем float параметры (как blob)
    err = nvs_set_blob(nvs_handle, "M_Speed", &M_Speed, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving M_Speed: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "M_PCoef", &M_PCoef, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving M_PCoef: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "M_ICoef", &M_ICoef, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving M_ICoef: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "M_Deadband", &M_Deadband, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving M_Deadband: %s", esp_err_to_name(err));
    
    // Коммитим изменения
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
void co_valve_params_load(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to load parameters from NVS namespace: %s", NVS_NAMESPACE);
    
    // Открываем NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error opening NVS handle (namespace may not exist): %s", esp_err_to_name(err));
        return;
    }
    
    // Загружаем int параметры
    int32_t int_val;
    
    err = nvs_get_i32(nvs_handle, "M_ControlType", &int_val);
    if (err == ESP_OK) {
        M_ControlType = (int)int_val;
        ESP_LOGI(TAG, "Loaded M_ControlType: %d", M_ControlType);
    }
    
    err = nvs_get_i32(nvs_handle, "M_RegType", &int_val);
    if (err == ESP_OK) {
        M_RegType = (int)int_val;
        ESP_LOGI(TAG, "Loaded M_RegType: %d", M_RegType);
    }
    
    err = nvs_get_i32(nvs_handle, "M_Length", &int_val);
    if (err == ESP_OK) {
        M_Length = (int)int_val;
        ESP_LOGI(TAG, "Loaded M_Length: %d", M_Length);
    }
    
    err = nvs_get_i32(nvs_handle, "M_IControl_Min", &int_val);
    if (err == ESP_OK) {
        M_IControl_Min = (int)int_val;
        ESP_LOGI(TAG, "Loaded M_IControl_Min: %d", M_IControl_Min);
    }
    
    // Загружаем float параметры
    size_t required_size = sizeof(float);
    
    err = nvs_get_blob(nvs_handle, "M_Speed", &M_Speed, &required_size);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Loaded M_Speed: %.3f", M_Speed);
    }
    
    required_size = sizeof(float);
    err = nvs_get_blob(nvs_handle, "M_PCoef", &M_PCoef, &required_size);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Loaded M_PCoef: %.3f", M_PCoef);
    }
    
    required_size = sizeof(float);
    err = nvs_get_blob(nvs_handle, "M_ICoef", &M_ICoef, &required_size);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Loaded M_ICoef: %.3f", M_ICoef);
    }
    
    required_size = sizeof(float);
    err = nvs_get_blob(nvs_handle, "M_Deadband", &M_Deadband, &required_size);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Loaded M_Deadband: %.3f", M_Deadband);
    }
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Parameters loaded from NVS successfully");
}

/**
 * @brief Инициализирует параметры клапан (загружает из NVS или использует значения по умолчанию)
 */
void co_valve_params_init(void) {
    ESP_LOGI(TAG, "Initializing CO valve parameters");
    co_valve_params_load();
}


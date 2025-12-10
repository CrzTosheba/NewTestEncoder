#include "CO_general_params.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "CO_PARAMS";
static const char *NVS_NAMESPACE = "co_params";

// Глобальные переменные для параметров отопления
heating_mode_t Mode = MODE_COMF;        // По умолчанию КОМФ
float T1_Econom = 55.0f;                // По умолчанию 18.0°C
float T1_Comfort = 65.0f;               // По умолчанию 22.0°C
float T1_Standby = 25.0f;               // По умолчанию 15.0°C
float T1_DesiredMax = 75.0f;            // По умолчанию 75.0°C
float T1_DesiredMin = 10.0f;            // По умолчанию 10.0°C

/**
 * @brief Сохраняет параметры в NVS
 */
void co_general_params_save(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Открываем NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    
    // Сохраняем Mode (как uint8_t)
    uint8_t mode_val = (uint8_t)Mode;
    err = nvs_set_u8(nvs_handle, "Mode", mode_val);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving Mode: %s", esp_err_to_name(err));
    }
    
    // Сохраняем float параметры
    err = nvs_set_blob(nvs_handle, "T1_Econom", &T1_Econom, sizeof(float));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving T1_Econom: %s", esp_err_to_name(err));
    }
    
    err = nvs_set_blob(nvs_handle, "T1_Comfort", &T1_Comfort, sizeof(float));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving T1_Comfort: %s", esp_err_to_name(err));
    }
    
    err = nvs_set_blob(nvs_handle, "T1_Standby", &T1_Standby, sizeof(float));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving T1_Standby: %s", esp_err_to_name(err));
    }
    
    err = nvs_set_blob(nvs_handle, "T1_DesiredMax", &T1_DesiredMax, sizeof(float));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving T1_DesiredMax: %s", esp_err_to_name(err));
    }
    
    err = nvs_set_blob(nvs_handle, "T1_DesiredMin", &T1_DesiredMin, sizeof(float));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving T1_DesiredMin: %s", esp_err_to_name(err));
    }
    
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
void co_general_params_load(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    size_t required_size;
    
    // Открываем NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS namespace not found or error opening: %s. Using defaults.", esp_err_to_name(err));
        return;
    }
    
    uint8_t mode_val;
    
    // Загружаем Mode
    err = nvs_get_u8(nvs_handle, "Mode", &mode_val);
    if (err == ESP_OK) {
        Mode = (heating_mode_t)mode_val;
        ESP_LOGI(TAG, "Loaded Mode: %d", Mode);
    } else {
        ESP_LOGW(TAG, "Mode not found in NVS, using default");
    }
    
    required_size = sizeof(float);
    
    // Загружаем float параметры
    err = nvs_get_blob(nvs_handle, "T1_Econom", &T1_Econom, &required_size);
    if (err == ESP_OK && required_size == sizeof(float)) {
        ESP_LOGI(TAG, "Loaded T1_Econom: %.1f", T1_Econom);
    } else {
        ESP_LOGW(TAG, "T1_Econom not found in NVS, using default");
    }
    
    required_size = sizeof(float);
    err = nvs_get_blob(nvs_handle, "T1_Comfort", &T1_Comfort, &required_size);
    if (err == ESP_OK && required_size == sizeof(float)) {
        ESP_LOGI(TAG, "Loaded T1_Comfort: %.1f", T1_Comfort);
    } else {
        ESP_LOGW(TAG, "T1_Comfort not found in NVS, using default");
    }
    
    required_size = sizeof(float);
    err = nvs_get_blob(nvs_handle, "T1_Standby", &T1_Standby, &required_size);
    if (err == ESP_OK && required_size == sizeof(float)) {
        ESP_LOGI(TAG, "Loaded T1_Standby: %.1f", T1_Standby);
    } else {
        ESP_LOGW(TAG, "T1_Standby not found in NVS, using default");
    }
    
    required_size = sizeof(float);
    err = nvs_get_blob(nvs_handle, "T1_DesiredMax", &T1_DesiredMax, &required_size);
    if (err == ESP_OK && required_size == sizeof(float)) {
        ESP_LOGI(TAG, "Loaded T1_DesiredMax: %.1f", T1_DesiredMax);
    } else {
        ESP_LOGW(TAG, "T1_DesiredMax not found in NVS, using default");
    }
    
    required_size = sizeof(float);
    err = nvs_get_blob(nvs_handle, "T1_DesiredMin", &T1_DesiredMin, &required_size);
    if (err == ESP_OK && required_size == sizeof(float)) {
        ESP_LOGI(TAG, "Loaded T1_DesiredMin: %.1f", T1_DesiredMin);
    } else {
        ESP_LOGW(TAG, "T1_DesiredMin not found in NVS, using default");
    }
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Parameters loaded from NVS");
}

// Функция инициализации параметров
void co_general_params_init(void) {
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    // (они уже установлены при объявлении переменных)
    ESP_LOGI(TAG, "CO general parameters initialized with default values");
    ESP_LOGI(TAG, "Mode: %d, T1_Econom: %.1f, T1_Comfort: %.1f, T1_Standby: %.1f, T1_DesiredMax: %.1f, T1_DesiredMin: %.1f",
             Mode, T1_Econom, T1_Comfort, T1_Standby, T1_DesiredMax, T1_DesiredMin);
}


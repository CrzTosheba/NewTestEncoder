#include "CO_manual_params.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "CO_MANUAL_PARAMS";
static const char *NVS_NAMESPACE = "co_manual";  // Максимум 15 символов для NVS

// Глобальные переменные для параметров ручного режима
manual_pump1_t N1_DControl = MANUAL_PUMP1_OFF;    // По умолчанию выкл
manual_pump2_t N2_DControl = MANUAL_PUMP2_OFF;    // По умолчанию выкл
manual_valve_t M_IControl = MANUAL_VALVE_OFF;     // По умолчанию выкл

/**
 * @brief Сохраняет параметры в NVS
 */
void co_manual_params_save(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to save parameters to NVS namespace: %s", NVS_NAMESPACE);
    
    // Открываем NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    
    // Сохраняем enum параметры (как uint8_t)
    uint8_t pump1_val = (uint8_t)N1_DControl;
    err = nvs_set_u8(nvs_handle, "N1_DControl", pump1_val);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N1_DControl: %s", esp_err_to_name(err));
    
    uint8_t pump2_val = (uint8_t)N2_DControl;
    err = nvs_set_u8(nvs_handle, "N2_DControl", pump2_val);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N2_DControl: %s", esp_err_to_name(err));
    
    uint8_t valve_val = (uint8_t)M_IControl;
    err = nvs_set_u8(nvs_handle, "M_IControl", valve_val);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving M_IControl: %s", esp_err_to_name(err));
    
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
void co_manual_params_load(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to load parameters from NVS namespace: %s", NVS_NAMESPACE);
    
    // Открываем NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error opening NVS handle (namespace may not exist): %s", esp_err_to_name(err));
        return;
    }
    
    // Загружаем enum параметры
    uint8_t pump1_val;
    err = nvs_get_u8(nvs_handle, "N1_DControl", &pump1_val);
    if (err == ESP_OK) {
        N1_DControl = (manual_pump1_t)pump1_val;
        ESP_LOGI(TAG, "Loaded N1_DControl: %d", N1_DControl);
    }
    
    uint8_t pump2_val;
    err = nvs_get_u8(nvs_handle, "N2_DControl", &pump2_val);
    if (err == ESP_OK) {
        N2_DControl = (manual_pump2_t)pump2_val;
        ESP_LOGI(TAG, "Loaded N2_DControl: %d", N2_DControl);
    }
    
    uint8_t valve_val;
    err = nvs_get_u8(nvs_handle, "M_IControl", &valve_val);
    if (err == ESP_OK) {
        M_IControl = (manual_valve_t)valve_val;
        ESP_LOGI(TAG, "Loaded M_IControl: %d", M_IControl);
    }
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Parameters loaded from NVS successfully");
}

/**
 * @brief Инициализирует параметры ручного режима (загружает из NVS или использует значения по умолчанию)
 */
void co_manual_params_init(void) {
    ESP_LOGI(TAG, "Initializing CO manual parameters");
    co_manual_params_load();
}


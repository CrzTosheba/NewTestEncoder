#include "CO_heating_graph_params.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "CO_HEATING_GRAPH_PARAMS";
static const char *NVS_NAMESPACE = "co_heat_graph";  // Максимум 15 символов для NVS

// Глобальные переменные для параметров графика отопления
heating_graph_type_t C1_Type = HEATING_GRAPH_TYPE_POINTS;  // По умолчанию по точкам
float C1_Slope = 45.0f;                                    // По умолчанию 45.0°
int C1_Number = 6;                                         // По умолчанию 6 точек
float C1_T0_1 = -20.0f;                                    // По умолчанию -20.0°C
float C1_T1_Desired_1 = 30.0f;                            // По умолчанию 30.0°C
float C1_T0_2 = -10.0f;                                   // По умолчанию -10.0°C
float C1_T1_Desired_2 = 40.0f;                            // По умолчанию 40.0°C
float C1_T0_3 = 0.0f;                                     // По умолчанию 0.0°C
float C1_T1_Desired_3 = 50.0f;                            // По умолчанию 50.0°C
float C1_T0_4 = 10.0f;                                    // По умолчанию 10.0°C
float C1_T1_Desired_4 = 60.0f;                            // По умолчанию 60.0°C
float C1_T0_5 = 20.0f;                                    // По умолчанию 20.0°C
float C1_T1_Desired_5 = 70.0f;                            // По умолчанию 70.0°C
float C1_T0_6 = 30.0f;                                    // По умолчанию 30.0°C
float C1_T1_Desired_6 = 75.0f;                            // По умолчанию 75.0°C
float C3_T1_6 = 70.0f;                                    // По умолчанию 70.0°C
float C3_T1_Desired_6 = 75.0f;                            // По умолчанию 75.0°C

/**
 * @brief Сохраняет параметры в NVS
 */
void co_heating_graph_params_save(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Открываем NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    
    // Сохраняем C1_Type (как uint8_t)
    uint8_t type_val = (uint8_t)C1_Type;
    err = nvs_set_u8(nvs_handle, "C1_Type", type_val);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving C1_Type: %s", esp_err_to_name(err));
    }
    
    // Сохраняем C1_Number (как int32_t)
    err = nvs_set_i32(nvs_handle, "C1_Number", C1_Number);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving C1_Number: %s", esp_err_to_name(err));
    }
    
    // Сохраняем float параметры
    err = nvs_set_blob(nvs_handle, "C1_Slope", &C1_Slope, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_Slope: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T0_1", &C1_T0_1, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T0_1: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T1_Desired_1", &C1_T1_Desired_1, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T1_Desired_1: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T0_2", &C1_T0_2, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T0_2: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T1_Desired_2", &C1_T1_Desired_2, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T1_Desired_2: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T0_3", &C1_T0_3, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T0_3: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T1_Desired_3", &C1_T1_Desired_3, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T1_Desired_3: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T0_4", &C1_T0_4, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T0_4: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T1_Desired_4", &C1_T1_Desired_4, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T1_Desired_4: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T0_5", &C1_T0_5, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T0_5: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T1_Desired_5", &C1_T1_Desired_5, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T1_Desired_5: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T0_6", &C1_T0_6, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T0_6: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C1_T1_Desired_6", &C1_T1_Desired_6, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C1_T1_Desired_6: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C3_T1_6", &C3_T1_6, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C3_T1_6: %s", esp_err_to_name(err));
    
    err = nvs_set_blob(nvs_handle, "C3_T1_Desired_6", &C3_T1_Desired_6, sizeof(float));
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving C3_T1_Desired_6: %s", esp_err_to_name(err));
    
    // Коммитим изменения
    ESP_LOGI(TAG, "Committing NVS changes...");
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Parameters saved to NVS successfully");
        ESP_LOGI(TAG, "Saved values: C1_Type=%d, C1_Number=%d, C1_Slope=%.1f", 
                 C1_Type, C1_Number, C1_Slope);
    }
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "NVS handle closed");
}

/**
 * @brief Загружает параметры из NVS
 */
void co_heating_graph_params_load(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    size_t required_size;
    
    // Открываем NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS namespace not found or error opening: %s. Using defaults.", esp_err_to_name(err));
        return;
    }
    
    uint8_t type_val;
    int32_t number_val;
    
    // Загружаем C1_Type
    err = nvs_get_u8(nvs_handle, "C1_Type", &type_val);
    if (err == ESP_OK) {
        C1_Type = (heating_graph_type_t)type_val;
        ESP_LOGI(TAG, "Loaded C1_Type: %d", C1_Type);
    } else {
        ESP_LOGW(TAG, "C1_Type not found in NVS, using default");
    }
    
    // Загружаем C1_Number
    err = nvs_get_i32(nvs_handle, "C1_Number", &number_val);
    if (err == ESP_OK) {
        C1_Number = (int)number_val;
        ESP_LOGI(TAG, "Loaded C1_Number: %d", C1_Number);
    } else {
        ESP_LOGW(TAG, "C1_Number not found in NVS, using default");
    }
    
    required_size = sizeof(float);
    
    // Загружаем float параметры
    #define LOAD_FLOAT_PARAM(name, var) \
        required_size = sizeof(float); \
        err = nvs_get_blob(nvs_handle, name, &var, &required_size); \
        if (err == ESP_OK && required_size == sizeof(float)) { \
            ESP_LOGI(TAG, "Loaded %s: %.1f", name, var); \
        } else { \
            ESP_LOGW(TAG, "%s not found in NVS, using default", name); \
        }
    
    LOAD_FLOAT_PARAM("C1_Slope", C1_Slope);
    LOAD_FLOAT_PARAM("C1_T0_1", C1_T0_1);
    LOAD_FLOAT_PARAM("C1_T1_Desired_1", C1_T1_Desired_1);
    LOAD_FLOAT_PARAM("C1_T0_2", C1_T0_2);
    LOAD_FLOAT_PARAM("C1_T1_Desired_2", C1_T1_Desired_2);
    LOAD_FLOAT_PARAM("C1_T0_3", C1_T0_3);
    LOAD_FLOAT_PARAM("C1_T1_Desired_3", C1_T1_Desired_3);
    LOAD_FLOAT_PARAM("C1_T0_4", C1_T0_4);
    LOAD_FLOAT_PARAM("C1_T1_Desired_4", C1_T1_Desired_4);
    LOAD_FLOAT_PARAM("C1_T0_5", C1_T0_5);
    LOAD_FLOAT_PARAM("C1_T1_Desired_5", C1_T1_Desired_5);
    LOAD_FLOAT_PARAM("C1_T0_6", C1_T0_6);
    LOAD_FLOAT_PARAM("C1_T1_Desired_6", C1_T1_Desired_6);
    LOAD_FLOAT_PARAM("C3_T1_6", C3_T1_6);
    LOAD_FLOAT_PARAM("C3_T1_Desired_6", C3_T1_Desired_6);
    
    #undef LOAD_FLOAT_PARAM
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Parameters loaded from NVS");
}

// Функция инициализации параметров
void co_heating_graph_params_init(void) {
    // Сначала пытаемся загрузить из NVS
    co_heating_graph_params_load();
    
    // Если загрузка не удалась, используем значения по умолчанию
    // (они уже установлены при объявлении переменных)
}


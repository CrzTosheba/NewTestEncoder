#include "CO_pumps_params.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "CO_PUMPS_PARAMS";
static const char *NVS_NAMESPACE = "co_pumps";  // Максимум 15 символов для NVS

// Глобальные переменные для параметров насосов
int N_Number = 2;                                    // По умолчанию 2 насоса
int N_BeforeStartPause = 2;                          // По умолчанию 2 с
int N_BeforeStopPause = 2;                           // По умолчанию 2 с
int N_ChangeOverPause = 5;                           // По умолчанию 5 с
pump_change_mode_t N_ChangeMode = PUMP_CHANGE_MODE_TIME; // По умолчанию ЧАСЫ
int N_ChangeWHours = 48;                            // По умолчанию 48 ч
int N_ChangeWDays = 2;                              // По умолчанию 2 сут
int N_ChangeHours = 3;                              // По умолчанию 3 ч
int N_ChangeMinutes = 0;                            // По умолчанию 0 мин
pump_reset_t N1_ResetWHours = PUMP_RESET_OFF;       // По умолчанию НЕТ
int N1_WHours = 0;                                  // Только для отображения
int N1_WStarts = 0;                                 // Только для отображения
pump_reset_t N2_ResetWHours = PUMP_RESET_OFF;       // По умолчанию НЕТ
int N2_WHours = 0;                                  // Только для отображения
int N2_WStarts = 0;                                 // Только для отображения
pump_training_t N_Training_En = PUMP_TRAINING_OFF;  // По умолчанию ВЫКЛ
int N_Training_Period = 10;                         // По умолчанию 10 с

/**
 * @brief Сохраняет параметры в NVS
 */
void co_pumps_params_save(void) {
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
    uint8_t mode_val = (uint8_t)N_ChangeMode;
    err = nvs_set_u8(nvs_handle, "N_ChangeMode", mode_val);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_ChangeMode: %s", esp_err_to_name(err));
    
    uint8_t reset1_val = (uint8_t)N1_ResetWHours;
    err = nvs_set_u8(nvs_handle, "N1_ResetWHours", reset1_val);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N1_ResetWHours: %s", esp_err_to_name(err));
    
    uint8_t reset2_val = (uint8_t)N2_ResetWHours;
    err = nvs_set_u8(nvs_handle, "N2_ResetWHours", reset2_val);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N2_ResetWHours: %s", esp_err_to_name(err));
    
    uint8_t training_val = (uint8_t)N_Training_En;
    err = nvs_set_u8(nvs_handle, "N_Training_En", training_val);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_Training_En: %s", esp_err_to_name(err));
    
    // Сохраняем int параметры (как int32_t)
    err = nvs_set_i32(nvs_handle, "N_Number", N_Number);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_Number: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "N_BeforeStopPause", N_BeforeStopPause);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_BeforeStopPause: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "N_ChangeOverPause", N_ChangeOverPause);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_ChangeOverPause: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "N_ChangeWHours", N_ChangeWHours);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_ChangeWHours: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "N_ChangeWDays", N_ChangeWDays);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_ChangeWDays: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "N_ChangeHours", N_ChangeHours);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_ChangeHours: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "N_ChangeMinutes", N_ChangeMinutes);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_ChangeMinutes: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "N1_WHours", N1_WHours);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N1_WHours: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "N1_WStarts", N1_WStarts);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N1_WStarts: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "N2_WHours", N2_WHours);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N2_WHours: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "N2_WStarts", N2_WStarts);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N2_WStarts: %s", esp_err_to_name(err));
    
    err = nvs_set_i32(nvs_handle, "N_Training_Period", N_Training_Period);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving N_Training_Period: %s", esp_err_to_name(err));
    
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
void co_pumps_params_load(void) {
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
    uint8_t mode_val;
    err = nvs_get_u8(nvs_handle, "N_ChangeMode", &mode_val);
    if (err == ESP_OK) {
        N_ChangeMode = (pump_change_mode_t)mode_val;
        ESP_LOGI(TAG, "Loaded N_ChangeMode: %d", N_ChangeMode);
    }
    
    uint8_t reset1_val;
    err = nvs_get_u8(nvs_handle, "N1_ResetWHours", &reset1_val);
    if (err == ESP_OK) {
        N1_ResetWHours = (pump_reset_t)reset1_val;
        ESP_LOGI(TAG, "Loaded N1_ResetWHours: %d", N1_ResetWHours);
    }
    
    uint8_t reset2_val;
    err = nvs_get_u8(nvs_handle, "N2_ResetWHours", &reset2_val);
    if (err == ESP_OK) {
        N2_ResetWHours = (pump_reset_t)reset2_val;
        ESP_LOGI(TAG, "Loaded N2_ResetWHours: %d", N2_ResetWHours);
    }
    
    uint8_t training_val;
    err = nvs_get_u8(nvs_handle, "N_Training_En", &training_val);
    if (err == ESP_OK) {
        N_Training_En = (pump_training_t)training_val;
        ESP_LOGI(TAG, "Loaded N_Training_En: %d", N_Training_En);
    }
    
    // Загружаем int параметры
    int32_t int_val;
    
    err = nvs_get_i32(nvs_handle, "N_Number", &int_val);
    if (err == ESP_OK) {
        N_Number = (int)int_val;
        ESP_LOGI(TAG, "Loaded N_Number: %d", N_Number);
    }
    
    err = nvs_get_i32(nvs_handle, "N_BeforeStopPause", &int_val);
    if (err == ESP_OK) {
        N_BeforeStopPause = (int)int_val;
        ESP_LOGI(TAG, "Loaded N_BeforeStopPause: %d", N_BeforeStopPause);
    }
    
    err = nvs_get_i32(nvs_handle, "N_ChangeOverPause", &int_val);
    if (err == ESP_OK) {
        N_ChangeOverPause = (int)int_val;
        ESP_LOGI(TAG, "Loaded N_ChangeOverPause: %d", N_ChangeOverPause);
    }
    
    err = nvs_get_i32(nvs_handle, "N_ChangeWHours", &int_val);
    if (err == ESP_OK) {
        N_ChangeWHours = (int)int_val;
        ESP_LOGI(TAG, "Loaded N_ChangeWHours: %d", N_ChangeWHours);
    }
    
    err = nvs_get_i32(nvs_handle, "N_ChangeWDays", &int_val);
    if (err == ESP_OK) {
        N_ChangeWDays = (int)int_val;
        ESP_LOGI(TAG, "Loaded N_ChangeWDays: %d", N_ChangeWDays);
    }
    
    err = nvs_get_i32(nvs_handle, "N_ChangeHours", &int_val);
    if (err == ESP_OK) {
        N_ChangeHours = (int)int_val;
        ESP_LOGI(TAG, "Loaded N_ChangeHours: %d", N_ChangeHours);
    }
    
    err = nvs_get_i32(nvs_handle, "N_ChangeMinutes", &int_val);
    if (err == ESP_OK) {
        N_ChangeMinutes = (int)int_val;
        ESP_LOGI(TAG, "Loaded N_ChangeMinutes: %d", N_ChangeMinutes);
    }
    
    err = nvs_get_i32(nvs_handle, "N1_WHours", &int_val);
    if (err == ESP_OK) {
        N1_WHours = (int)int_val;
        ESP_LOGI(TAG, "Loaded N1_WHours: %d", N1_WHours);
    }
    
    err = nvs_get_i32(nvs_handle, "N1_WStarts", &int_val);
    if (err == ESP_OK) {
        N1_WStarts = (int)int_val;
        ESP_LOGI(TAG, "Loaded N1_WStarts: %d", N1_WStarts);
    }
    
    err = nvs_get_i32(nvs_handle, "N2_WHours", &int_val);
    if (err == ESP_OK) {
        N2_WHours = (int)int_val;
        ESP_LOGI(TAG, "Loaded N2_WHours: %d", N2_WHours);
    }
    
    err = nvs_get_i32(nvs_handle, "N2_WStarts", &int_val);
    if (err == ESP_OK) {
        N2_WStarts = (int)int_val;
        ESP_LOGI(TAG, "Loaded N2_WStarts: %d", N2_WStarts);
    }
    
    err = nvs_get_i32(nvs_handle, "N_Training_Period", &int_val);
    if (err == ESP_OK) {
        N_Training_Period = (int)int_val;
        ESP_LOGI(TAG, "Loaded N_Training_Period: %d", N_Training_Period);
    }
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Parameters loaded from NVS successfully");
}

/**
 * @brief Инициализирует параметры насосов (загружает из NVS или использует значения по умолчанию)
 */
void co_pumps_params_init(void) {
    ESP_LOGI(TAG, "Initializing CO pumps parameters");
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "CO pumps parameters initialized with default values");
}


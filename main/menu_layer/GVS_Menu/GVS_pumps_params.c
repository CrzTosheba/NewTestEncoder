#include "GVS_pumps_params.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "GVS_PUMPS_PARAMS";

// Глобальные переменные для параметров насосов ГВС
gvs_pump_number_t GVS_N_Number = GVS_PUMP_NUMBER_2;              // По умолчанию 2 насоса
int GVS_N_BeforeStartPause = 2;                                  // По умолчанию 2 с
int GVS_N_BeforeStopPause = 2;                                   // По умолчанию 2 с
int GVS_N_ChangeOverPause = 5;                                   // По умолчанию 5 с
gvs_pump_change_mode_t GVS_N_ChangeMode = GVS_PUMP_CHANGE_MODE_HOURS; // По умолчанию ЧАСЫ
int GVS_N_ChangeWHours = 48;                                     // По умолчанию 48 ч
int GVS_N_ChangeWDays = 2;                                       // По умолчанию 2 сут
int GVS_N_ChangeHours = 3;                                      // По умолчанию 3 ч
int GVS_N_ChangeMinutes = 0;                                     // По умолчанию 0 мин
gvs_pump_reset_t GVS_N1_ResetWHours = GVS_PUMP_RESET_NO;         // По умолчанию НЕТ
int GVS_N1_WHours = 0;                                           // Только для отображения
int GVS_N1_WStarts = 0;                                          // Только для отображения
gvs_pump_reset_t GVS_N2_ResetWHours = GVS_PUMP_RESET_NO;         // По умолчанию НЕТ
int GVS_N2_WHours = 0;                                           // Только для отображения
int GVS_N2_WStarts = 0;                                          // Только для отображения
gvs_pump_training_t GVS_N_Training_En = GVS_PUMP_TRAINING_OFF;  // По умолчанию ВЫКЛ
int GVS_N_Training_Period = 10;                                  // По умолчанию 10 с

// Функция инициализации параметров
void gvs_pumps_params_init(void) {
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "GVS pumps parameters initialized with default values");
    ESP_LOGI(TAG, "N_Number=%d, BeforeStartPause=%d, BeforeStopPause=%d, ChangeOverPause=%d",
             GVS_N_Number, GVS_N_BeforeStartPause, GVS_N_BeforeStopPause, GVS_N_ChangeOverPause);
    ESP_LOGI(TAG, "ChangeMode=%d, ChangeWHours=%d, ChangeWDays=%d, ChangeHours=%d, ChangeMinutes=%d",
             GVS_N_ChangeMode, GVS_N_ChangeWHours, GVS_N_ChangeWDays, GVS_N_ChangeHours, GVS_N_ChangeMinutes);
    ESP_LOGI(TAG, "N1_ResetWHours=%d, N2_ResetWHours=%d, Training_En=%d, Training_Period=%d",
             GVS_N1_ResetWHours, GVS_N2_ResetWHours, GVS_N_Training_En, GVS_N_Training_Period);
}


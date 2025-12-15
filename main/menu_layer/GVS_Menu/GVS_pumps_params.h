#ifndef GVS_PUMPS_PARAMS_H
#define GVS_PUMPS_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Типы enum для насосов ГВС
typedef enum {
    GVS_PUMP_NUMBER_NONE = 0,  // НЕТ
    GVS_PUMP_NUMBER_1 = 1,     // 1
    GVS_PUMP_NUMBER_2 = 2      // 2
} gvs_pump_number_t;

typedef enum {
    GVS_PUMP_CHANGE_MODE_HOURS = 0,  // ЧАСЫ
    GVS_PUMP_CHANGE_MODE_DAYS = 1    // ДЕНЬ
} gvs_pump_change_mode_t;

typedef enum {
    GVS_PUMP_RESET_NO = 0,   // НЕТ
    GVS_PUMP_RESET_YES = 1   // ДА
} gvs_pump_reset_t;

typedef enum {
    GVS_PUMP_TRAINING_OFF = 0,  // ВЫКЛ
    GVS_PUMP_TRAINING_ON = 1    // ВКЛ
} gvs_pump_training_t;

// Глобальные переменные для параметров насосов ГВС
extern gvs_pump_number_t GVS_N_Number;                    // Количество насосов
extern int GVS_N_BeforeStartPause;                        // Пауза перед старт, с
extern int GVS_N_BeforeStopPause;                        // Пауза перед стоп, с
extern int GVS_N_ChangeOverPause;                        // Пауза переключ., с
extern gvs_pump_change_mode_t GVS_N_ChangeMode;          // Режим переключения
extern int GVS_N_ChangeWHours;                           // Период работы, ч
extern int GVS_N_ChangeWDays;                            // Период работы, сут
extern int GVS_N_ChangeHours;                            // Время переключ., ч
extern int GVS_N_ChangeMinutes;                          // Время переключ., мин
extern gvs_pump_reset_t GVS_N1_ResetWHours;              // Сброс.наработку Н1
extern int GVS_N1_WHours;                                // Время наработки Н1, ч (только для отображения)
extern int GVS_N1_WStarts;                               // Кол-во запусков Н1 (только для отображения)
extern gvs_pump_reset_t GVS_N2_ResetWHours;              // Сброс.наработку Н2
extern int GVS_N2_WHours;                                // Время наработки Н2, ч (только для отображения)
extern int GVS_N2_WStarts;                               // Кол-во запусков Н2 (только для отображения)
extern gvs_pump_training_t GVS_N_Training_En;           // Тренировать
extern int GVS_N_Training_Period;                       // Период тренировки, c

// Функции для работы с параметрами
void gvs_pumps_params_init(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_PUMPS_PARAMS_H


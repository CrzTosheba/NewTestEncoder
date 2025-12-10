#ifndef CO_PUMPS_PARAMS_H
#define CO_PUMPS_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Типы enum для насосов
typedef enum {
    PUMP_CHANGE_MODE_TIME = 0,      // ЧАСЫ
    PUMP_CHANGE_MODE_WORK = 1       // ДЕНЬ
} pump_change_mode_t;

typedef enum {
    PUMP_RESET_OFF = 0,             // НЕТ
    PUMP_RESET_ON = 1               // ДА
} pump_reset_t;

typedef enum {
    PUMP_TRAINING_OFF = 0,          // ВЫКЛ
    PUMP_TRAINING_ON = 1            // ВКЛ
} pump_training_t;

// Глобальные переменные для параметров насосов
extern int N_Number;                    // Количество насосов
extern int N_BeforeStartPause;          // Пауза перед старт, с
extern int N_BeforeStopPause;          // Пауза перед стоп, с
extern int N_ChangeOverPause;          // Пауза переключ., с
extern pump_change_mode_t N_ChangeMode; // Режим переключения
extern int N_ChangeWHours;              // Период работы, ч
extern int N_ChangeWDays;               // Период работы, сут
extern int N_ChangeHours;               // Время переключ., ч
extern int N_ChangeMinutes;              // Время переключ., мин
extern pump_reset_t N1_ResetWHours;     // Сброс.наработку Н1
extern int N1_WHours;                   // Время наработки Н1, ч
extern int N1_WStarts;                  // Кол-во запусков Н1
extern pump_reset_t N2_ResetWHours;     // Сброс.наработку Н2
extern int N2_WHours;                   // Время наработки Н2, ч
extern int N2_WStarts;                  // Кол-во запусков Н2
extern pump_training_t N_Training_En;   // Тренировать
extern int N_Training_Period;           // Период тренировки, c

// Функции для работы с параметрами
void co_pumps_params_init(void);
void co_pumps_params_save(void);
void co_pumps_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_PUMPS_PARAMS_H


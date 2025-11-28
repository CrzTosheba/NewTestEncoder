#ifndef CO_EXT_ALARM_PARAMS_H
#define CO_EXT_ALARM_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные для параметров внешней аварии
extern int N1_EnExtAlarm;
extern int N2_EnExtAlarm;
extern int N_ExtAlarmDelay;
extern int N_ExtAlarmRType;

// Функции для работы с параметрами
void co_ext_alarm_params_init(void);
void co_ext_alarm_params_save(void);
void co_ext_alarm_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_EXT_ALARM_PARAMS_H


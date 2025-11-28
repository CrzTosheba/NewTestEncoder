#ifndef CO_SENSOR_BREAK_PARAMS_H
#define CO_SENSOR_BREAK_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные для параметров обрыва датчика
extern int T1_EnAlarm;
extern int AIAlarmDelay;
extern int T1_AlarmRType;

// Функции для работы с параметрами
void co_sensor_break_params_init(void);
void co_sensor_break_params_save(void);
void co_sensor_break_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_SENSOR_BREAK_PARAMS_H


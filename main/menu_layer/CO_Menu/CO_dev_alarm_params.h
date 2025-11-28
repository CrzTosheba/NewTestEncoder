#ifndef CO_DEV_ALARM_PARAMS_H
#define CO_DEV_ALARM_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные для параметров аварийного отклонения
extern int T1_EnDevAlarm;
extern int T1_EnHighAlarm;
extern int T1_EnLowAlarm;
extern int T1_DevAlarmDelay;
extern int T1_DevAlarmRType;
extern float T1_AlarmDev;

// Функции для работы с параметрами
void co_dev_alarm_params_init(void);
void co_dev_alarm_params_save(void);
void co_dev_alarm_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_DEV_ALARM_PARAMS_H


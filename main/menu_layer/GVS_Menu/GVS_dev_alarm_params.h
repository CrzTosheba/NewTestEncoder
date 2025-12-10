#ifndef GVS_DEV_ALARM_PARAMS_H
#define GVS_DEV_ALARM_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные для параметров аварийного отклонения ГВС
extern int GVS_T1_EnDevAlarm;
extern int GVS_T1_EnHighAlarm;
extern int GVS_T1_EnLowAlarm;
extern int GVS_T1_DevAlarmDelay;
extern int GVS_T1_DevAlarmRType;
extern float GVS_T1_AlarmDev;

// Функции для работы с параметрами
void gvs_dev_alarm_params_init(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_DEV_ALARM_PARAMS_H


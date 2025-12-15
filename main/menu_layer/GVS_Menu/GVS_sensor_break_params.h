#ifndef GVS_SENSOR_BREAK_PARAMS_H
#define GVS_SENSOR_BREAK_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Тип enum для активации аварии
typedef enum {
    GVS_SENSOR_ALARM_NO = 0,  // НЕТ
    GVS_SENSOR_ALARM_YES = 1  // ДА
} gvs_sensor_alarm_en_t;

// Тип enum для сброса аварии
typedef enum {
    GVS_SENSOR_ALARM_RTYPE_AUTO = 0,   // АВТО
    GVS_SENSOR_ALARM_RTYPE_MANUAL = 1, // РУЧН
    GVS_SENSOR_ALARM_RTYPE_MANUAL_1 = 2,  // РУЧН-1
    GVS_SENSOR_ALARM_RTYPE_MANUAL_2 = 3,  // РУЧН-2
    GVS_SENSOR_ALARM_RTYPE_MANUAL_3 = 4,  // РУЧН-3
    GVS_SENSOR_ALARM_RTYPE_MANUAL_4 = 5,  // РУЧН-4
    GVS_SENSOR_ALARM_RTYPE_MANUAL_5 = 6,  // РУЧН-5
    GVS_SENSOR_ALARM_RTYPE_MANUAL_6 = 7,  // РУЧН-6
    GVS_SENSOR_ALARM_RTYPE_MANUAL_7 = 8,  // РУЧН-7
    GVS_SENSOR_ALARM_RTYPE_MANUAL_8 = 9,  // РУЧН-8
    GVS_SENSOR_ALARM_RTYPE_MANUAL_9 = 10, // РУЧН-9
    GVS_SENSOR_ALARM_RTYPE_MANUAL_10 = 11 // РУЧН-10
} gvs_sensor_alarm_rtype_t;

// Глобальные переменные для параметров обрыва датчика ГВС
extern gvs_sensor_alarm_en_t GVS_T1_EnAlarm;      // GVS_T1-EnAlarm
extern int GVS_AIAlarmDelay;                       // GVS_AIAlarmDelay
extern gvs_sensor_alarm_rtype_t GVS_T1_AlarmRType; // GVS_T1-AlarmRType

// Функции для работы с параметрами
void gvs_sensor_break_params_init(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_SENSOR_BREAK_PARAMS_H


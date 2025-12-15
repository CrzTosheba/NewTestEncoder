#ifndef GVS_EXT_ALARM_PARAMS_H
#define GVS_EXT_ALARM_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Тип enum для активации внешней аварии
typedef enum {
    GVS_EXT_ALARM_NO = 0,  // НЕТ
    GVS_EXT_ALARM_YES = 1  // ДА
} gvs_ext_alarm_en_t;

// Тип enum для сброса внешней аварии
typedef enum {
    GVS_EXT_ALARM_RTYPE_AUTO = 0,   // АВТО
    GVS_EXT_ALARM_RTYPE_MANUAL = 1, // РУЧН
    GVS_EXT_ALARM_RTYPE_MANUAL_1 = 2,  // РУЧН-1
    GVS_EXT_ALARM_RTYPE_MANUAL_2 = 3,  // РУЧН-2
    GVS_EXT_ALARM_RTYPE_MANUAL_3 = 4,  // РУЧН-3
    GVS_EXT_ALARM_RTYPE_MANUAL_4 = 5,  // РУЧН-4
    GVS_EXT_ALARM_RTYPE_MANUAL_5 = 6,  // РУЧН-5
    GVS_EXT_ALARM_RTYPE_MANUAL_6 = 7,  // РУЧН-6
    GVS_EXT_ALARM_RTYPE_MANUAL_7 = 8,  // РУЧН-7
    GVS_EXT_ALARM_RTYPE_MANUAL_8 = 9,  // РУЧН-8
    GVS_EXT_ALARM_RTYPE_MANUAL_9 = 10, // РУЧН-9
    GVS_EXT_ALARM_RTYPE_MANUAL_10 = 11 // РУЧН-10
} gvs_ext_alarm_rtype_t;

// Глобальные переменные для параметров внешней аварии ГВС
extern gvs_ext_alarm_en_t GVS_N1_EnExtAlarm;      // GVS_N1-EnExtAlarm
extern gvs_ext_alarm_en_t GVS_N2_EnExtAlarm;      // GVS_N2-EnExtAlarm
extern int GVS_N_ExtAlarmDelay;                   // GVS_N-ExtAlarmDelay
extern gvs_ext_alarm_rtype_t GVS_N_ExtAlarmRType; // GVS_N-ExtAlarmRType

// Функции для работы с параметрами
void gvs_ext_alarm_params_init(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_EXT_ALARM_PARAMS_H


#ifndef GVS_DRY_RUN_PARAMS_H
#define GVS_DRY_RUN_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Тип enum для активации
typedef enum {
    GVS_PS_ENALARM_NO = 0,  // НЕТ
    GVS_PS_ENALARM_YES = 1  // ДА
} gvs_ps_enalarm_t;

// Тип enum для сброса
typedef enum {
    GVS_PS_ALARM_RTYPE_AUTO = 0,   // АВТО
    GVS_PS_ALARM_RTYPE_MANUAL = 1, // РУЧН
    GVS_PS_ALARM_RTYPE_MANUAL_1 = 2,  // РУЧН-1
    GVS_PS_ALARM_RTYPE_MANUAL_2 = 3,  // РУЧН-2
    GVS_PS_ALARM_RTYPE_MANUAL_3 = 4,  // РУЧН-3
    GVS_PS_ALARM_RTYPE_MANUAL_4 = 5,  // РУЧН-4
    GVS_PS_ALARM_RTYPE_MANUAL_5 = 6,  // РУЧН-5
    GVS_PS_ALARM_RTYPE_MANUAL_6 = 7,  // РУЧН-6
    GVS_PS_ALARM_RTYPE_MANUAL_7 = 8,  // РУЧН-7
    GVS_PS_ALARM_RTYPE_MANUAL_8 = 9,  // РУЧН-8
    GVS_PS_ALARM_RTYPE_MANUAL_9 = 10, // РУЧН-9
    GVS_PS_ALARM_RTYPE_MANUAL_10 = 11 // РУЧН-10
} gvs_ps_alarm_rtype_t;

// Глобальные переменные для параметров сухого хода ГВС
extern gvs_ps_enalarm_t GVS_PS_EnAlarm;
extern int GVS_PS_AlarmDelay;
extern gvs_ps_alarm_rtype_t GVS_PS_AlarmRType;

// Функции для работы с параметрами
void gvs_dry_run_params_init(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_DRY_RUN_PARAMS_H


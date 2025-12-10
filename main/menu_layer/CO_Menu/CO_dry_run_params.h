#ifndef CO_DRY_RUN_PARAMS_H
#define CO_DRY_RUN_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Тип enum для активации
typedef enum {
    CO_PS_ENALARM_NO = 0,  // НЕТ
    CO_PS_ENALARM_YES = 1  // ДА
} co_ps_enalarm_t;

// Тип enum для сброса
typedef enum {
    CO_PS_ALARM_RTYPE_AUTO = 0,   // АВТО
    CO_PS_ALARM_RTYPE_MANUAL = 1, // РУЧН
    CO_PS_ALARM_RTYPE_MANUAL_1 = 2,  // РУЧН-1
    CO_PS_ALARM_RTYPE_MANUAL_2 = 3,  // РУЧН-2
    CO_PS_ALARM_RTYPE_MANUAL_3 = 4,  // РУЧН-3
    CO_PS_ALARM_RTYPE_MANUAL_4 = 5,  // РУЧН-4
    CO_PS_ALARM_RTYPE_MANUAL_5 = 6,  // РУЧН-5
    CO_PS_ALARM_RTYPE_MANUAL_6 = 7,  // РУЧН-6
    CO_PS_ALARM_RTYPE_MANUAL_7 = 8,  // РУЧН-7
    CO_PS_ALARM_RTYPE_MANUAL_8 = 9,  // РУЧН-8
    CO_PS_ALARM_RTYPE_MANUAL_9 = 10, // РУЧН-9
    CO_PS_ALARM_RTYPE_MANUAL_10 = 11 // РУЧН-10
} co_ps_alarm_rtype_t;

// Глобальные переменные для параметров сухого хода
extern co_ps_enalarm_t PS_EnAlarm;
extern int PS_AlarmDelay;
extern co_ps_alarm_rtype_t PS_AlarmRType;

// Функции для работы с параметрами
void co_dry_run_params_init(void);
void co_dry_run_params_save(void);
void co_dry_run_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_DRY_RUN_PARAMS_H


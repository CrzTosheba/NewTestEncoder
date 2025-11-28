#ifndef CO_DRY_RUN_PARAMS_H
#define CO_DRY_RUN_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные для параметров сухого хода
extern int PS_EnAlarm;
extern int PS_AlarmDelay;
extern int PS_AlarmRType;

// Функции для работы с параметрами
void co_dry_run_params_init(void);
void co_dry_run_params_save(void);
void co_dry_run_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_DRY_RUN_PARAMS_H


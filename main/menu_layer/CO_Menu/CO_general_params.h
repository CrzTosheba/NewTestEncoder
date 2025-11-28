#ifndef CO_GENERAL_PARAMS_H
#define CO_GENERAL_PARAMS_H

#include "CO_general_menu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные для параметров отопления
extern heating_mode_t Mode;              // Режим (КОМФ/ЭКОН)
extern float T1_Econom;                  // Тэконом, °C
extern float T1_Comfort;                 // Ткомф, °C
extern float T1_Standby;                 // Тожид, °C
extern float T1_DesiredMax;              // Макс.Тпод_СО, °C
extern float T1_DesiredMin;              // Мин.Тпод_СО, °C

// Функции для работы с параметрами
void co_general_params_init(void);
void co_general_params_save(void);
void co_general_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_GENERAL_PARAMS_H


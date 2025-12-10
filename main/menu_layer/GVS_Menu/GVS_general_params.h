#ifndef GVS_GENERAL_PARAMS_H
#define GVS_GENERAL_PARAMS_H

#include "GVS_general_menu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные для параметров ГВС
extern gvs_mode_t GVS_Mode;              // Режим (РУЧН/РАСП/ЭКОН/КОМФ/АВАР)
extern float GVS_T1_Econom;              // Тэконом, °C
extern float GVS_T1_Comfort;             // Ткомф, °C
extern float GVS_T1_Standby;              // Тожид, °C
extern float GVS_T1_DesiredMax;          // Макс.Тпод_ГВС, °C
extern float GVS_T1_DesiredMin;          // Мин.Тпод_ГВС, °C

// Функции для работы с параметрами (без сохранения в NVS)
void gvs_general_params_init(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_GENERAL_PARAMS_H


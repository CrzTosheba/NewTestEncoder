#ifndef CO_HEATING_GRAPH_PARAMS_H
#define CO_HEATING_GRAPH_PARAMS_H

#include "CO_heating_graph_menu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные для параметров графика отопления
extern heating_graph_type_t C1_Type;              // Способ задания
extern float C1_Slope;                            // Угол наклона
extern int C1_Number;                              // Количество точек
extern float C1_T0_1;                             // Точка 1. Тнв, °C
extern float C1_T1_Desired_1;                     // Точка 1. Тпод_CO, °C
extern float C1_T0_2;                             // Точка 2. Тнв, °C
extern float C1_T1_Desired_2;                     // Точка 2. Тпод_CO, °C
extern float C1_T0_3;                             // Точка 3. Тнв, °C
extern float C1_T1_Desired_3;                     // Точка 3. Тпод_CO, °C
extern float C1_T0_4;                             // Точка 4. Тнв, °C
extern float C1_T1_Desired_4;                     // Точка 4. Тпод_CO, °C
extern float C1_T0_5;                             // Точка 5. Тнв, °C
extern float C1_T1_Desired_5;                     // Точка 5. Тпод_CO, °C
extern float C1_T0_6;                             // Точка 6. Тнв, °C
extern float C1_T1_Desired_6;                     // Точка 6. Тпод_CO, °C
extern float C3_T1_6;                             // Точка 6. Тпод.тс, °C
extern float C3_T1_Desired_6;                     // Точка 6. Тпод_CO, °C

// Функции для работы с параметрами
void co_heating_graph_params_init(void);
void co_heating_graph_params_save(void);
void co_heating_graph_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_HEATING_GRAPH_PARAMS_H


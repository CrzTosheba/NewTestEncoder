#ifndef CO_VALVE_PARAMS_H
#define CO_VALVE_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные для параметров клапан
extern int M_ControlType;        // Управл.сигнал
extern int M_RegType;            // Тип регулятора
extern int M_Length;             // Длина штока, мм
extern float M_Speed;            // Скорость, с/мм
extern float M_PCoef;            // П-коэффициент
extern float M_ICoef;            // И-коэффициент
extern float M_Deadband;         // Нейтральная зона, °C
extern int M_IControl_Min;       // Мин. ширина ИМПС, мс

// Функции для работы с параметрами
void co_valve_params_init(void);
void co_valve_params_save(void);
void co_valve_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_VALVE_PARAMS_H


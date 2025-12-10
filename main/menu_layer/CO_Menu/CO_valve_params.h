#ifndef CO_VALVE_PARAMS_H
#define CO_VALVE_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Тип enum для типа регулятора СО
typedef enum {
    CO_REG_TYPE_P = 0,      // П
    CO_REG_TYPE_PI = 1,     // ПИ
    CO_REG_TYPE_PID = 2     // ПИД
} co_reg_type_t;

// Глобальные переменные для параметров клапан
extern co_reg_type_t M_RegType;            // M-RegType: Тип регулятора (enum: П/ПИ/ПИД)
extern int M_Length;             // M-Length: Длина штока, мм
extern float M_Speed;            // M-Speed: Скорость, с/мм
extern float M_PCoef;            // M-PCoef: П-коэффициент
extern float M_ICoef;            // M-ICoef: И-коэффициент
extern float M_Deadband;         // M-Deadband: Нейтральная зона, °C
extern int M_IControl_Min;       // M-IControl-Min: Мин. ширина ИМПС, мс

// Функции для работы с параметрами
void co_valve_params_init(void);
void co_valve_params_save(void);
void co_valve_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_VALVE_PARAMS_H


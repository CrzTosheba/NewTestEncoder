#ifndef GVS_VALVE_PARAMS_H
#define GVS_VALVE_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Тип enum для типа регулятора ГВС
typedef enum {
    GVS_REG_TYPE_P = 0,      // П
    GVS_REG_TYPE_PI = 1,     // ПИ
    GVS_REG_TYPE_PID = 2     // ПИД
} gvs_reg_type_t;

// Глобальные переменные для параметров клапан ГВС
extern gvs_reg_type_t GVS_M_RegType;        // M-RegType: Тип регулятора
extern int GVS_M_Length;                    // M-Length: Длина штока, мм
extern float GVS_M_Speed;                   // M-Speed: Скорость, с/мм
extern float GVS_M_PCoef;                   // M-PCoef: П-коэффициент
extern float GVS_M_ICoef;                   // M-ICoef: И-коэффициент
extern float GVS_M_Deadband;                // M-Deadband: Нейтральная зона, °C
extern int GVS_M_IControl_Min;              // M-IControl-Min: Мин. ширина ИМПС, мс

// Функции для работы с параметрами
void gvs_valve_params_init(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_VALVE_PARAMS_H


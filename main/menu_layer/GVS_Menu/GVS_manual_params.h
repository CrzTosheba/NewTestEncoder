#ifndef GVS_MANUAL_PARAMS_H
#define GVS_MANUAL_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Типы enum для ручного режима ГВС
typedef enum {
    GVS_MANUAL_PUMP_OFF = 0,  // ВЫКЛ
    GVS_MANUAL_PUMP_ON = 1    // ВКЛ
} gvs_manual_pump_t;

typedef enum {
    GVS_MANUAL_VALVE_CLOSED = 0,  // ЗАКР
    GVS_MANUAL_VALVE_OPEN = 1,    // ОТКР
    GVS_MANUAL_VALVE_STOP = 2      // СТОП
} gvs_manual_valve_t;

// Глобальные переменные для параметров ручного режима ГВС
extern gvs_manual_pump_t GVS_N1_DControl;    // Насос 1
extern gvs_manual_pump_t GVS_N2_DControl;    // Насос 2
extern gvs_manual_valve_t GVS_M_IControl;    // Клапан

// Функции для работы с параметрами (без сохранения в NVS)
void gvs_manual_params_init(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_MANUAL_PARAMS_H


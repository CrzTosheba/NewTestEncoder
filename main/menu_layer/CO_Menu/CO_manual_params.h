#ifndef CO_MANUAL_PARAMS_H
#define CO_MANUAL_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Типы enum для ручного режима (пока простые значения, будут редактироваться)
typedef enum {
    MANUAL_PUMP1_OFF = 0,
    MANUAL_PUMP1_ON = 1
} manual_pump1_t;

typedef enum {
    MANUAL_PUMP2_OFF = 0,
    MANUAL_PUMP2_ON = 1
} manual_pump2_t;

typedef enum {
    MANUAL_VALVE_OFF = 0,
    MANUAL_VALVE_ON = 1
} manual_valve_t;

// Глобальные переменные для параметров ручного режима
extern manual_pump1_t N1_DControl;    // Насос 1
extern manual_pump2_t N2_DControl;    // Насос 2
extern manual_valve_t M_IControl;      // Клапан

// Функции для работы с параметрами
void co_manual_params_init(void);
void co_manual_params_save(void);
void co_manual_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_MANUAL_PARAMS_H


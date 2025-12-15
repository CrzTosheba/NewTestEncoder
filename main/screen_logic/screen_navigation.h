#ifndef SCREEN_NAVIGATION_H
#define SCREEN_NAVIGATION_H

#include "lvgl.h"
#include "encoder/encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

// Типы экранов для навигации
typedef enum {
    SCREEN_MAIN_MENU,      // Главное меню
    SCREEN_PASSWORD_INPUT, // Экран ввода пароля
    SCREEN_GVS,            // Экран ГВС
    SCREEN_CO,             // Экран отопления (меню)
    SCREEN_PODP,           // Экран подпитки
    SCREEN_UV,             // Экран узла ввода
    SCREEN_IN_OUT,         // Экран входов/выходов
    SCREEN_ALARMS,
    SCREEN_SERVICE
} screen_type_t;

// Функции навигации
void screen_navigation_init(void);
void screen_navigation_go_to(screen_type_t screen);
void screen_navigation_encoder_event_cb(uint8_t e);
screen_type_t screen_navigation_get_current_screen(void);

// Функции для сохранения и восстановления позиции курсора
void screen_navigation_save_cursor_position(void);
void screen_navigation_restore_cursor_position(void);

// Функция полной очистки
void screen_navigation_full_cleanup(void);

// Внешние обработчики энкодера
extern void main_menu_encoder_event_cb(uint8_t e);
extern void password_encoder_event_cb(uint8_t e);
extern void input_output_encoder_event_cb(uint8_t e);
extern void co_menu_encoder_event_cb(uint8_t e);
extern void co_general_menu_encoder_event_cb(uint8_t e);
extern void gvs_menu_encoder_event_cb(uint8_t e);
extern void gvs_general_menu_encoder_event_cb(uint8_t e);
extern void gvs_pumps_menu_encoder_event_cb(uint8_t e);
extern void gvs_valve_menu_encoder_event_cb(uint8_t e);
extern void gvs_manual_menu_encoder_event_cb(uint8_t e);
extern void gvs_schedule_menu_encoder_event_cb(uint8_t e);
extern void gvs_schedule_day_menu_encoder_event_cb(uint8_t e);
extern void gvs_alarms_menu_encoder_event_cb(uint8_t e);
extern void gvs_dry_run_menu_encoder_event_cb(uint8_t e);

// Функции управления видимостью меню
extern void main_menu_show(void);
extern void main_menu_hide(void);
extern void input_output_menu_show(void);
extern void input_output_menu_hide(void);
extern void co_menu_show(void);
extern void co_menu_hide(void);
extern void gvs_menu_show(void);
extern void gvs_menu_hide(void);
extern void gvs_general_menu_show(void);
extern void gvs_general_menu_hide(void);
extern void gvs_pumps_menu_show(void);
extern void gvs_pumps_menu_hide(void);
extern void gvs_valve_menu_show(void);
extern void gvs_valve_menu_hide(void);
extern void gvs_manual_menu_show(void);
extern void gvs_manual_menu_hide(void);
extern void gvs_schedule_menu_show(void);
extern void gvs_schedule_menu_hide(void);
extern void gvs_schedule_day_menu_show(void);
extern void gvs_schedule_day_menu_hide(void);
extern void gvs_alarms_menu_show(void);
extern void gvs_alarms_menu_hide(void);
extern void gvs_dry_run_menu_show(void);
extern void gvs_dry_run_menu_hide(void);

#ifdef __cplusplus
}
#endif

#endif // SCREEN_NAVIGATION_H
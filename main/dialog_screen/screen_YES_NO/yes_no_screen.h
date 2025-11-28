#ifndef YES_NO_SCREEN_H
#define YES_NO_SCREEN_H

#include "lvgl.h"
#include "encoder/encoder.h"

// Тип callback функции для сохранения
typedef void (*yes_no_save_callback_t)(void);
typedef void (*yes_no_cancel_callback_t)(void);

// Флаги состояния окна
extern bool confirmation_active;
extern bool selected_yes;

// Прототипы функций
void create_yes_no_screen(void);
void create_yes_no_screen_with_callbacks(yes_no_save_callback_t save_cb, yes_no_cancel_callback_t cancel_cb);
void close_yes_no_screen(void);
void yes_no_menu_encoder_event_cb(uint8_t e);
void update_buttons_style(void);


#endif /*YES_NO_SCREEN_H*/
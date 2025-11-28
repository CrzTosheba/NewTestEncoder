#ifndef YES_NO_SCREEN_H
#define YES_NO_SCREEN_H

#include "lvgl.h"
#include "encoder/encoder.h"



// Флаги состояния окна
extern bool confirmation_active;
extern bool selected_yes;

// Прототипы функций
void create_yes_no_screen();
void close_yes_no_screen();
void yes_no_menu_encoder_event_cb(uint8_t e);
void update_buttons_style();


#endif /*YES_NO_SCREEN_H*/
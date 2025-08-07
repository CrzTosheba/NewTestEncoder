#ifndef _ENCODER_
#define _ENCODER_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


enum enc_event_en {
    ENC_NONE           = 0,
    ENC_LEFT           = 1,
    ENC_RIGHT          = 2,
    ENC_HOLD           = 4,
    ENC_LONG_HOLD      = 8,
    ENC_CLICK          = 16,
    ENC_DOUBLE_CLICK   = 32
};

typedef void (*enc_event)(uint8_t t);

void enc_init(int delay, int gpio_sw, int gpio_a, int gpio_b);
void enc_register_event(const enc_event e);
void enc_unregister_event();
void enc_loop();

#endif //_ENCODER_
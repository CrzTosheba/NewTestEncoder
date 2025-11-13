#ifndef SCREEN_SERVICE_H
#define SCREEN_SERVICE_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif
LV_FONT_DECLARE(Roboto_bold_24);
void screen_service_create(lv_obj_t *parent);


#ifdef __cplusplus
}
#endif

#endif // SCREEN_SERVICE_H
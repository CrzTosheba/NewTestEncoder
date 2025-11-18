#ifndef SCREEN_CONTAINER_MANAGER_H
#define SCREEN_CONTAINER_MANAGER_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Типы контейнеров
typedef enum {
    CONTAINER_TYPE_MAIN_MENU,
    CONTAINER_TYPE_IN_OUT,
    CONTAINER_TYPE_PASSWORD
} container_type_t;

// Функции управления контейнерами
lv_obj_t* screen_container_create(container_type_t type);
void screen_container_destroy(lv_obj_t* container);
void screen_container_show(lv_obj_t* container);
void screen_container_hide(lv_obj_t* container);

// Стандартные размеры и позиции контейнеров
#define CONTENT_CONTAINER_WIDTH  492
#define CONTENT_CONTAINER_HEIGHT 380
#define CONTENT_CONTAINER_X      -20
#define CONTENT_CONTAINER_Y      50

#ifdef __cplusplus
}
#endif

#endif // SCREEN_CONTAINER_MANAGER_H
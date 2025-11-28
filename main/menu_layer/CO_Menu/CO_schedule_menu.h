#ifndef CO_SCHEDULE_MENU_H
#define CO_SCHEDULE_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции главного меню расписания
void CO_Schedule_Menu_List(void);
void co_schedule_menu_encoder_event_cb(uint8_t e);
void co_schedule_menu_cleanup(void);
void co_schedule_menu_show(void);
void co_schedule_menu_hide(void);

// Функции подменю дня недели
void CO_Schedule_Day_Menu_List(int day_index);
void co_schedule_day_menu_encoder_event_cb(uint8_t e);
void co_schedule_day_menu_cleanup(void);
void co_schedule_day_menu_show(void);
void co_schedule_day_menu_hide(void);

// Объявляем глобальные переменные для доступа из других файлов
extern lv_obj_t *co_schedule_cont;
extern lv_obj_t *co_schedule_day_cont;
extern int current_schedule_day; // Текущий выбранный день (0-5: Пн-Сб)

#ifdef __cplusplus
}
#endif

#endif // CO_SCHEDULE_MENU_H


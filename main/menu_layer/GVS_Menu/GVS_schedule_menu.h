#ifndef GVS_SCHEDULE_MENU_H
#define GVS_SCHEDULE_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Функции главного меню расписания ГВС
void GVS_Schedule_Menu_List(void);
void gvs_schedule_menu_encoder_event_cb(uint8_t e);
void gvs_schedule_menu_cleanup(void);
void gvs_schedule_menu_show(void);
void gvs_schedule_menu_hide(void);

// Функции подменю дня недели ГВС
void GVS_Schedule_Day_Menu_List(int day_index);
void gvs_schedule_day_menu_encoder_event_cb(uint8_t e);
void gvs_schedule_day_menu_cleanup(void);
void gvs_schedule_day_menu_show(void);
void gvs_schedule_day_menu_hide(void);

// Объявляем глобальные переменные для доступа из других файлов
extern lv_obj_t *gvs_schedule_cont;
extern lv_obj_t *gvs_schedule_day_cont;
extern int current_gvs_schedule_day; // Текущий выбранный день (0-6: Пн-Вс)

#ifdef __cplusplus
}
#endif

#endif // GVS_SCHEDULE_MENU_H


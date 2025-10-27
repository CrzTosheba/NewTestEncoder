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
    SCREEN_CO,             // Экран отопления
    SCREEN_PODP,           // Экран подпитки
    SCREEN_UV,             // Экран узла ввода
    SCREEN_IN_OUT          // Экран входов/выходов
} screen_type_t;

// Функции навигации
void screen_navigation_init(void);                    // Инициализация навигации
void screen_navigation_go_to(screen_type_t screen);   // Переход к указанному экрану
void screen_navigation_encoder_event_cb(uint8_t e);   // Обработчик событий энкодера для навигации
screen_type_t screen_navigation_get_current_screen(void); // Получение текущего экрана

// Внешние обработчики энкодера (объявлены в других файлах)
extern void main_menu_encoder_event_cb(uint8_t e);
extern void password_encoder_event_cb(uint8_t e);

#ifdef __cplusplus
}
#endif

#endif // SCREEN_NAVIGATION_H
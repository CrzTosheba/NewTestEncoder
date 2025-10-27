// password_screen.h
#ifndef PASSWORD_SCREEN_H
#define PASSWORD_SCREEN_H

#include "lvgl.h"

// Объявление функций
void password_screen(void);                    // Создание экрана ввода пароля
void password_encoder_event_cb(uint8_t e);     // Обработчик энкодера для экрана пароля
void password_screen_cleanup(void);            // Очистка экрана пароля

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24); 
LV_FONT_DECLARE(Roboto_bold_36); 
LV_FONT_DECLARE(Roboto_bold_48); 

// Объявления изображений
LV_IMG_DECLARE(lv_im_radius_yellow);
LV_IMG_DECLARE(lv_im_radius_gray);

// Настраиваемые параметры роллера
#define ROLLER_RADIUS 400                    // Радиус дуги
#define ROLLER_ANGLE_STEP 0.3                // Угловой шаг между цифрами
#define DIGIT_SPACING 0.4                    // Расстояние между цифрами внутри роллера (множитель)
#define VISIBLE_DIGITS 7                     // Количество видимых цифр в роллере (должно быть нечетным)

// Настройки прозрачности
#define MIN_OPACITY 100                      // Минимальная прозрачность (0-255)
#define OPACITY_STEP 40                      // Шаг изменения прозрачности

// Цвета
#define ACTIVE_DIGIT_COLOR lv_color_hex(0x000000)           // Цвет активной цифры (черный)
#define INACTIVE_CENTRAL_DIGIT_COLOR lv_color_hex(0xFFFFFF) // Цвет центральной цифры неактивного роллера (белый)
#define ACTIVE_ROLLER_COLOR lv_color_hex(0x727779)          // Цвет цифр активного роллера
#define INACTIVE_ROLLER_COLOR lv_color_hex(0x727779)        // Цвет цифр неактивных роллеров

// Позиции изображений под роллерами
#define ROLLER_IMG1_X 268    // Позиция X для первого роллера
#define ROLLER_IMG1_Y 70     // Позиция Y для первого роллера
#define ROLLER_IMG2_X 328    // Позиция X для второго роллера
#define ROLLER_IMG2_Y 70     // Позиция Y для второго роллера
#define ROLLER_IMG3_X 388    // Позиция X для третьего роллера
#define ROLLER_IMG3_Y 70     // Позиция Y для третьего роллера

// Центры роллеров с цифрами
#define ROLLER1_CENTER_X 740    // Центр X для первого роллера
#define ROLLER1_CENTER_Y 240    // Центр Y для первого роллера
#define ROLLER2_CENTER_X 800    // Центр X для второго роллера
#define ROLLER2_CENTER_Y 240    // Центр Y для второго роллера
#define ROLLER3_CENTER_X 860    // Центр X для третьего роллера
#define ROLLER3_CENTER_Y 240    // Центр Y для третьего роллера

#endif /*PASSWORD_SCREEN_H*/
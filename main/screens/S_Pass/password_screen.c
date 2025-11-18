// password_screen.c
#include "password_screen.h"
#include "encoder/encoder.h"
#include "stdint.h"
#include "esp_log.h"
#include "my_widgets/w_rad_mask.h"
#include <math.h>
#include <stdlib.h>
#include "screen_logic/screen_navigation.h"

static const char *TAG = "PASSWORD_SCREEN";

// Массив для хранения указателей на метки цифр
static lv_obj_t *digit_labels[VISIBLE_DIGITS][3];
// Массив для хранения указателей на изображения под роллерами
static lv_obj_t *roller_images[3];
// Массив значений цифр
static uint8_t digit_values[3] = {0, 0, 0};
// Индекс текущего активного роллера (0, 1 или 2)
static uint8_t current_digit_index = 0;
// Индекс центральной цифры
static const uint8_t central_digit_index = VISIBLE_DIGITS / 2;
// Флаг завершения ввода пароля
static bool password_input_complete = false;

// Экран для ввода пароля
static lv_obj_t *password_screen_obj = NULL;
// Флаг, указывающий что экран пароля активен
static bool password_screen_active = false;

// Массив центров для каждого роллера
static const int32_t roller_centers[3][2] = {
    {ROLLER1_CENTER_X, ROLLER1_CENTER_Y},
    {ROLLER2_CENTER_X, ROLLER2_CENTER_Y},
    {ROLLER3_CENTER_X, ROLLER3_CENTER_Y}
};

// Стили для цифр
static lv_style_t style_active_digit;
static lv_style_t style_active_roller;
static lv_style_t style_inactive_roller;
static lv_style_t style_inactive_central_digit;
static lv_style_t style_transparent_bg;

// Прототипы функций
static void update_digit_positions(void);
static void update_digit_display(void);
static void update_roller_images(void);
static void cleanup_password_objects(void);
static bool is_obj_valid_safe(lv_obj_t *obj);

/**
 * @brief Безопасная проверка объекта LVGL
 */
static bool is_obj_valid_safe(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Очистка всех объектов экрана пароля
 */
static void cleanup_password_objects(void) {
    ESP_LOGI(TAG, "Cleaning up password objects");
    
    // Сбрасываем все указатели на NULL
    for (int row = 0; row < VISIBLE_DIGITS; row++) {
        for (int col = 0; col < 3; col++) {
            digit_labels[row][col] = NULL;
        }
    }
    
    for (int col = 0; col < 3; col++) {
        roller_images[col] = NULL;
    }
    
    // Сбрасываем состояния
    current_digit_index = 0;
    password_input_complete = false;
    
    ESP_LOGI(TAG, "Password objects cleaned up");
}

/**
 * @brief Очистка экрана пароля
 */
void password_screen_cleanup(void) {
    ESP_LOGI(TAG, "Starting password screen cleanup");
    
    if (is_obj_valid_safe(password_screen_obj) && password_screen_active) {
        ESP_LOGI(TAG, "Cleaning up password screen objects");
        
        // Удаляем экран пароля асинхронно
        lv_obj_t *screen_to_delete = password_screen_obj;
        password_screen_obj = NULL;
        lv_obj_del_async(screen_to_delete);
        
        ESP_LOGI(TAG, "Password screen object scheduled for deletion");
    }
    
    // Очищаем все объекты
    cleanup_password_objects();
    
    // Сбрасываем флаги
    password_screen_active = false;
    
    ESP_LOGI(TAG, "Password screen cleanup completed");
}

/**
 * @brief Обновление изображений под роллерами
 */
static void update_roller_images(void) {
    for (int col = 0; col < 3; col++) {
        if (is_obj_valid_safe(roller_images[col])) {
            if (col == current_digit_index) {
                lv_img_set_src(roller_images[col], &lv_im_radius_yellow);
            } else {
                lv_img_set_src(roller_images[col], &lv_im_radius_gray);
            }
        }
    }
}

/**
 * @brief Обновление отображения цифр
 */
static void update_digit_display(void) {
    // Проверяем, что экран все еще активен
    if (!password_screen_active) {
        return;
    }
    
    for (int col = 0; col < 3; col++) {
        // Центральная цифра (текущее значение)
        if (is_obj_valid_safe(digit_labels[central_digit_index][col])) {
            lv_label_set_text_fmt(digit_labels[central_digit_index][col], "%d", digit_values[col]);
        }
        
        // Верхние цифры (предыдущие значения)
        for (int row = 0; row < central_digit_index; row++) {
            if (is_obj_valid_safe(digit_labels[row][col])) {
                uint8_t value = (digit_values[col] + (central_digit_index - row)) % 10;
                lv_label_set_text_fmt(digit_labels[row][col], "%d", value);
            }
        }
        
        // Нижние цифры (следующие значения)
        for (int row = central_digit_index + 1; row < VISIBLE_DIGITS; row++) {
            if (is_obj_valid_safe(digit_labels[row][col])) {
                uint8_t value = (digit_values[col] - (row - central_digit_index) + 10) % 10;
                lv_label_set_text_fmt(digit_labels[row][col], "%d", value);
            }
        }
    }
    
    // Обновляем позиции цифр
    update_digit_positions();
}

/**
 * @brief Обновление позиций цифр по дуге
 */
static void update_digit_positions(void) {
    // Проверяем, что экран все еще активен
    if (!password_screen_active) {
        return;
    }
    
    for (int col = 0; col < 3; col++) {
        const int32_t center_x = roller_centers[col][0];
        const int32_t center_y = roller_centers[col][1];
        const int32_t radius = ROLLER_RADIUS;
        
        for (int row = 0; row < VISIBLE_DIGITS; row++) {
            if (!is_obj_valid_safe(digit_labels[row][col])) continue;
            
            // Угол для позиции на вертикальной дуге (в радианах)
            double angle = -((row - central_digit_index) * ROLLER_ANGLE_STEP * DIGIT_SPACING) + M_PI;
            
            // Рассчитываем позицию на вертикальной дуге
            int32_t x = center_x + radius * cos(angle);
            int32_t y = center_y - radius * sin(angle);
            
            // Устанавливаем позицию
            lv_obj_set_pos(digit_labels[row][col], x - 15, y - 15);
            
            // Рассчитываем прозрачность для эффекта перспективы
            uint8_t opacity = 255 - abs(row - central_digit_index) * OPACITY_STEP;
            if (opacity < MIN_OPACITY) opacity = MIN_OPACITY;
            
            // Устанавливаем прозрачность
            lv_obj_set_style_opa(digit_labels[row][col], opacity, 0);
        }
    }
}

/**
 * @brief Обработчик событий энкодера для экрана пароля
 */
void password_encoder_event_cb(uint8_t e) {
    // Если экран пароля не активен или пароль уже введен, игнорируем события
    if (!password_screen_active || password_input_complete || !is_obj_valid_safe(password_screen_obj)) {
        return;
    }
    
    // Обработка поворота влево
    if (e & ENC_LEFT) {
        digit_values[current_digit_index] = (digit_values[current_digit_index] - 1 + 10) % 10;
        update_digit_display();
    } 
    // Обработка поворота вправо
    else if (e & ENC_RIGHT) {
        digit_values[current_digit_index] = (digit_values[current_digit_index] + 1) % 10;
        update_digit_display();
    }
    
    // Обработка нажатия (переключение между цифрами)
    if (e & ENC_CLICK) {
        // Делаем текущий роллер неактивным
        for (int i = 0; i < VISIBLE_DIGITS; i++) {
            if (is_obj_valid_safe(digit_labels[i][current_digit_index])) {
                if (i == central_digit_index) {
                    lv_obj_add_style(digit_labels[i][current_digit_index], &style_inactive_central_digit, 0);
                } else {
                    lv_obj_add_style(digit_labels[i][current_digit_index], &style_inactive_roller, 0);
                }
            }
        }
        
        // Переключаем на следующую цифру (циклически)
        current_digit_index = (current_digit_index + 1) % 3;
        
        // Если дошли до конца (вернулись к первому роллеру), завершаем ввод
        if (current_digit_index == 0) {
            password_input_complete = true;
            ESP_LOGI(TAG, "Password input complete: %d%d%d", 
                    digit_values[0], digit_values[1], digit_values[2]);
            
            // TODO: Проверить пароль здесь
            
            // Возвращаемся в главное меню
            screen_navigation_go_to(SCREEN_MAIN_MENU);
            return;
        }
        
        // Делаем новый роллер активным
        for (int i = 0; i < VISIBLE_DIGITS; i++) {
            if (is_obj_valid_safe(digit_labels[i][current_digit_index])) {
                if (i == central_digit_index) {
                    lv_obj_add_style(digit_labels[i][current_digit_index], &style_active_digit, 0);
                } else {
                    lv_obj_add_style(digit_labels[i][current_digit_index], &style_active_roller, 0);
                }
            }
        }
        
        // Обновляем отображение изображений под роллерами
        update_roller_images();
        
        // Обновляем отображение
        update_digit_display();
    } 
}

/**
 * @brief Создание экрана ввода пароля
 */
void password_screen(void) {
    ESP_LOGI(TAG, "Creating password screen with roller-like arc layout");
    
    // Очищаем предыдущие объекты, если они есть
    password_screen_cleanup();
    
    // Сбрасываем флаги состояния
    password_input_complete = false;
    current_digit_index = 0;
    
    // Проверяем, что количество видимых цифр нечетное
    if (VISIBLE_DIGITS % 2 == 0) {
        ESP_LOGE(TAG, "VISIBLE_DIGITS must be an odd number");
        return;
    }
    
    // Создаем отдельный экран для пароля
    password_screen_obj = lv_obj_create(NULL);
    if (!password_screen_obj) {
        ESP_LOGE(TAG, "Failed to create password screen object");
        return;
    }
    
    // Устанавливаем флаг активности
    password_screen_active = true;
    
    // Загружаем экран пароля
    lv_scr_load(password_screen_obj);
    
    // Устанавливаем фон экрана пароля
    lv_obj_set_style_bg_color(password_screen_obj, lv_color_hex(0x1e2528), LV_PART_MAIN);
    
    // Инициализация стилей
    lv_style_init(&style_active_digit);
    lv_style_set_text_font(&style_active_digit, &Roboto_bold_36);
    lv_style_set_text_color(&style_active_digit, ACTIVE_DIGIT_COLOR);
    
    lv_style_init(&style_active_roller);
    lv_style_set_text_font(&style_active_roller, &Roboto_bold_36);
    lv_style_set_text_color(&style_active_roller, ACTIVE_ROLLER_COLOR);
    
    lv_style_init(&style_inactive_roller);
    lv_style_set_text_font(&style_inactive_roller, &Roboto_bold_36);
    lv_style_set_text_color(&style_inactive_roller, INACTIVE_ROLLER_COLOR);
    
    lv_style_init(&style_inactive_central_digit);
    lv_style_set_text_font(&style_inactive_central_digit, &Roboto_bold_36);
    lv_style_set_text_color(&style_inactive_central_digit, INACTIVE_CENTRAL_DIGIT_COLOR);
    
    lv_style_init(&style_transparent_bg);
    lv_style_set_bg_opa(&style_transparent_bg, LV_OPA_TRANSP);
    lv_style_set_pad_all(&style_transparent_bg, 0);

    // Создаем изображения под роллерами с индивидуальными позициями
    for (int col = 0; col < 3; col++) {
        roller_images[col] = lv_img_create(password_screen_obj);
        if (!roller_images[col]) {
            ESP_LOGE(TAG, "Failed to create roller image %d", col);
            continue;
        }
        
        // Устанавливаем позицию в зависимости от колонки
        switch(col) {
            case 0:
                lv_obj_set_pos(roller_images[col], ROLLER_IMG1_X, ROLLER_IMG1_Y);
                break;
            case 1:
                lv_obj_set_pos(roller_images[col], ROLLER_IMG2_X, ROLLER_IMG2_Y);
                break;
            case 2:
                lv_obj_set_pos(roller_images[col], ROLLER_IMG3_X, ROLLER_IMG3_Y);
                break;
        }
    }
    
    // Обновляем изображения роллеров
    update_roller_images();
    
    // Создаем дополнительные элементы интерфейса
    lv_obj_t *img = lv_img_create(password_screen_obj);
    if (img) {
        lv_img_set_src(img, &lv_im_radius_gray);
        lv_obj_set_pos(img, 448, 70);
    }
    
    // Создаем объект (область)
    lv_obj_t *obj = lv_obj_create(password_screen_obj);
    if (obj) {
        lv_obj_set_size(obj, 220, 380);     
        lv_obj_set_pos(obj, 578, 70);
        
        static lv_style_t style;
        lv_style_init(&style);
        lv_style_set_bg_color(&style, lv_color_hex(0x2B3639));
        lv_style_set_bg_opa(&style, LV_OPA_COVER);
        lv_style_set_radius(&style, 0);
        lv_style_set_border_width(&style, 0);
        lv_style_set_outline_width(&style, 0);
        lv_style_set_shadow_width(&style, 0);
        lv_style_set_pad_all(&style, 0);
        lv_obj_add_style(obj, &style, 0);
    }
    
    // Текст "ВВЕДИТЕ ПАРОЛЬ"
    lv_obj_t *label = lv_label_create(password_screen_obj);
    if (label) {
        lv_label_set_text(label, "ВВЕДИТЕ ПАРОЛЬ");
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, &Roboto_bold_24, 0);
        lv_obj_align(label, LV_ALIGN_CENTER, 240, 0);
    }
   
    // Создаем метки для цифр
    for (int row = 0; row < VISIBLE_DIGITS; row++) {
        for (int col = 0; col < 3; col++) {
            digit_labels[row][col] = lv_label_create(password_screen_obj);
            if (!digit_labels[row][col]) {
                ESP_LOGE(TAG, "Failed to create digit label [%d][%d]", row, col);
                continue;
            }
            
            lv_obj_set_size(digit_labels[row][col], 30, 30);
            lv_obj_add_style(digit_labels[row][col], &style_transparent_bg, 0);
            
            if (col == current_digit_index) {
                if (row == central_digit_index) {
                    lv_obj_add_style(digit_labels[row][col], &style_active_digit, 0);
                } else {
                    lv_obj_add_style(digit_labels[row][col], &style_active_roller, 0);
                }
            } else {
                if (row == central_digit_index) {
                    lv_obj_add_style(digit_labels[row][col], &style_inactive_central_digit, 0);
                } else {
                    lv_obj_add_style(digit_labels[row][col], &style_inactive_roller, 0);
                }
            }
        }
    }

    // Обновляем отображение цифр
    update_digit_display();

    ESP_LOGI(TAG, "Password screen created with roller-like arc layout");
}
#include "screen_navigation.h"
#include "menu_layer/main_menu/main_menu.h"
#include "screens/S_Pass/password_screen.h"
#include "screens/S_Pass/screen_Pass.h"
#include "encoder/encoder_manager.h"
#include "menu_layer/In_Out_Menu/In_Out_main_menu.h"
#include <inttypes.h>
#include "esp_log.h"

static const char *TAG = "SCREEN_NAV";

static screen_type_t current_screen = SCREEN_MAIN_MENU;
static lv_obj_t *main_screen = NULL;

// Переменные для сохранения позиции курсора
static uint32_t saved_cursor_position = 0;
static bool cursor_position_saved = false;

void screen_navigation_init(void) {
    ESP_LOGI(TAG, "Initializing screen navigation");
    
    // Сохраняем указатель на главный экран
    main_screen = lv_scr_act();
    
    // Создаем главное меню по умолчанию
    Main_Menu_List();
    current_screen = SCREEN_MAIN_MENU;
    
    // Регистрируем обработчик энкодера для навигации
    encoder_manager_register_callback(screen_navigation_encoder_event_cb);
}

/**
 * @brief Сохраняет текущую позицию курсора главного меню
 */
void screen_navigation_save_cursor_position(void) {
    extern uint32_t current_cursor_index;
    saved_cursor_position = current_cursor_index;
    cursor_position_saved = true;
    ESP_LOGI(TAG, "Cursor position saved: %" PRIu32, saved_cursor_position);
}

/**
 * @brief Восстанавливает сохраненную позицию курсора главного меню
 */
void screen_navigation_restore_cursor_position(void) {
    if (cursor_position_saved) {
        extern uint32_t current_cursor_index;
        current_cursor_index = saved_cursor_position;
        ESP_LOGI(TAG, "Cursor position restored: %" PRIu32, saved_cursor_position);
    } else {
        ESP_LOGW(TAG, "No cursor position saved, using default");
    }
}

void screen_navigation_go_to(screen_type_t screen) {
    ESP_LOGI(TAG, "Navigating to screen: %d", screen);
    
    // Убираем текущий обработчик энкодера
    encoder_manager_unregister_callback();
    
    switch(screen) {
        case SCREEN_MAIN_MENU:
            // Восстанавливаем главный экран
            if (main_screen) {
                lv_scr_load(main_screen);
            }
            // Очищаем экран пароля
            password_screen_cleanup();
            // Очищаем меню входов/выходов
            input_output_menu_cleanup();
            
            // Пересоздаем главное меню если оно было уничтожено
            extern lv_obj_t *_cont;
            if (_cont == NULL || !lv_obj_is_valid(_cont)) {
                ESP_LOGI(TAG, "Recreating main menu");
                Main_Menu_List();
            }
            
            // Восстанавливаем позицию курсора
            screen_navigation_restore_cursor_position();
            
            // Показываем главное меню
            main_menu_show();
            
            // Обновляем отображение в соответствии с текущим положением курсора
            main_menu_update_display();
            
            encoder_manager_register_callback(screen_navigation_encoder_event_cb);
            break;
            
        case SCREEN_PASSWORD_INPUT:
            // Сохраняем позицию курсора перед переходом
            screen_navigation_save_cursor_position();
            
            // Скрываем главное меню
            main_menu_hide();
            // Создаем экран пароля
            password_screen();
            encoder_manager_register_callback(password_encoder_event_cb);
            break;
            
        case SCREEN_GVS:
            // TODO: Реализовать переход на экран ГВС
            ESP_LOGI(TAG, "Transition to GVS screen not implemented yet");
            break;
            
        case SCREEN_CO:
            // TODO: Реализовать переход на экран отопления  
            ESP_LOGI(TAG, "Transition to CO screen not implemented yet");
            break;
            
        case SCREEN_PODP:
            // TODO: Реализовать переход на экран подпитки
            ESP_LOGI(TAG, "Transition to PODP screen not implemented yet");
            break;
            
        case SCREEN_UV:
            // TODO: Реализовать переход на экран узла ввода
            ESP_LOGI(TAG, "Transition to UV screen not implemented yet");
            break;        

        case SCREEN_ALARMS:
            // TODO: Реализовать переход на экран аварий
            ESP_LOGI(TAG, "Transition to SCREEN_ALARMS screen not implemented yet");
            break;

        case SCREEN_IN_OUT:
            // Сохраняем позицию курсора перед переходом
            screen_navigation_save_cursor_position();
            
            // Переход на экран входов/выходов
            main_menu_hide();
            Input_Output_Menu_List();
            encoder_manager_register_callback(input_output_encoder_event_cb);
            break;

        case SCREEN_SERVICE:
            // TODO: Реализовать переход на экран сервиса
            ESP_LOGI(TAG, "Transition to SCREEN_SERVICE screen not implemented yet");
            break;
            
        default:
            ESP_LOGE(TAG, "Unknown screen type: %d", screen);
            return;
    }
    
    current_screen = screen;
}

void screen_navigation_encoder_event_cb(uint8_t e) {
    // Если мы в главном меню
    if (current_screen == SCREEN_MAIN_MENU) {
        // Передаем события в главное меню для обработки движения
        main_menu_encoder_event_cb(e);
        
        // Обработка нажатий в главном меню
        if ((e & ENC_CLICK)) {
            extern uint32_t current_cursor_index;
            ESP_LOGI(TAG, "Click detected on menu item: %" PRIu32, current_cursor_index);
            
            switch(current_cursor_index) {
                case 0: // "Открыть доступ"
                    ESP_LOGI(TAG, "Navigating to password screen");
                    screen_navigation_go_to(SCREEN_PASSWORD_INPUT);
                    break;
                case 4: // "Входы/выходы" - индекс 4 соответствует пункту "Входы/выходы"
                    ESP_LOGI(TAG, "Navigating to input/output screen");
                    screen_navigation_go_to(SCREEN_IN_OUT);
                    break;
                // Добавьте другие случаи по мере необходимости
                default:
                    ESP_LOGI(TAG, "No action defined for menu item: %" PRIu32, current_cursor_index);
                    break;
            }
        }
    }
}

screen_type_t screen_navigation_get_current_screen(void) {
    return current_screen;
}
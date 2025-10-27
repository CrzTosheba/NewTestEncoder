#include "screen_navigation.h"
#include "menu_layer/main_menu/main_menu.h"
#include "screens/S_Pass/password_screen.h"
#include "screens/S_Pass/screen_Pass.h"
#include "encoder/encoder_manager.h"
#include "esp_log.h"

static const char *TAG = "SCREEN_NAV";

static screen_type_t current_screen = SCREEN_MAIN_MENU;
static lv_obj_t *main_screen = NULL; // Сохраняем указатель на главный экран

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
            // Показываем главное меню
            main_menu_show();
            encoder_manager_register_callback(screen_navigation_encoder_event_cb);
            break;
            
        case SCREEN_PASSWORD_INPUT:
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
            
        case SCREEN_IN_OUT:
            // TODO: Реализовать переход на экран входов/выходов
            ESP_LOGI(TAG, "Transition to IN_OUT screen not implemented yet");
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
        // Передаем события в главное меню
        main_menu_encoder_event_cb(e);
        
        // Если нажали ENC_CLICK и находимся на первом пункте меню (Открыть доступ)
        if ((e & ENC_CLICK)) {
            extern uint32_t current_cursor_index;
            if (current_cursor_index == 0) {
                screen_navigation_go_to(SCREEN_PASSWORD_INPUT);
            }
        }
    }
}

screen_type_t screen_navigation_get_current_screen(void) {
    return current_screen;
}
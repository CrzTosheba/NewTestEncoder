#include "screen_navigation.h"
#include "menu_layer/main_menu/main_menu.h"
#include "screens/S_Pass/password_screen.h"
#include "screens/S_Pass/screen_Pass.h"
#include "encoder/encoder_manager.h"
#include "menu_layer/In_Out_Menu/In_Out_main_menu.h"
#include "menu_layer/CO_Menu/CO_main_menu.h"
#include "screens/S_In_Out/1_layer/screen_In_Out.h"
#include "screen_container_manager.h"
#include "arc_menu.h" // Добавляем include для arc_menu
#include <inttypes.h>
#include "esp_log.h"

static const char *TAG = "SCREEN_NAV";

static screen_type_t current_screen = SCREEN_MAIN_MENU;
static lv_obj_t *main_screen = NULL;

// Переменные для сохранения позиции курсора (теперь используем menu_state_t)
static menu_state_t saved_menu_state = {0};
static bool menu_state_saved = false;

// Контейнеры для разных экранов
static lv_obj_t *current_content_container = NULL;

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
 * @brief Сохраняет текущее состояние главного меню
 */
void screen_navigation_save_cursor_position(void) {
    menu_state_t *main_menu_state = get_menu_state(MENU_TYPE_MAIN);
    saved_menu_state = *main_menu_state;
    menu_state_saved = true;
    ESP_LOGI(TAG, "Menu state saved: cursor_index=%" PRIu32 ", list_index=%" PRIu32, 
             saved_menu_state.cursor_index, saved_menu_state.list_index);
}

/**
 * @brief Восстанавливает сохраненное состояние главного меню
 */
void screen_navigation_restore_cursor_position(void) {
    if (menu_state_saved) {
        menu_state_t *main_menu_state = get_menu_state(MENU_TYPE_MAIN);
        *main_menu_state = saved_menu_state;
        ESP_LOGI(TAG, "Menu state restored: cursor_index=%" PRIu32 ", list_index=%" PRIu32, 
                 saved_menu_state.cursor_index, saved_menu_state.list_index);
    } else {
        ESP_LOGW(TAG, "No menu state saved, using default");
    }
}

/**
 * @brief Очищает текущий контейнер контента
 */
static void screen_navigation_cleanup_current_container(void) {
    if (current_content_container && lv_obj_is_valid(current_content_container)) {
        // Добавляем задержку для завершения операций LVGL
        vTaskDelay(pdMS_TO_TICKS(50));
        
        // Очищаем подсветки перед удалением контейнера
        if (current_screen == SCREEN_IN_OUT) {
            screen_In_Out_cleanup_highlights();
        }
        
        // Используем безопасное удаление
        lv_obj_t *container_to_delete = current_content_container;
        current_content_container = NULL;
        
        // Удаляем в контексте LVGL
        if (lv_obj_is_valid(container_to_delete)) {
            lv_obj_del_async(container_to_delete);
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void screen_navigation_go_to(screen_type_t screen) {
    ESP_LOGI(TAG, "Navigating to screen: %d", screen);
    
    // Добавляем защиту от рекурсивных вызовов
    static bool in_transition = false;
    if (in_transition) {
        ESP_LOGW(TAG, "Navigation already in transition, skipping");
        return;
    }
    in_transition = true;
    
    // Убираем текущий обработчик энкодера
    encoder_manager_unregister_callback();
    
    // Даем время на завершение текущих операций
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Очищаем текущий контейнер
    screen_navigation_cleanup_current_container();
    
    // Сбрасываем флаг после завершения очистки
    in_transition = false;
    
    switch(screen) {
        case SCREEN_MAIN_MENU:
            // ВАЖНО: Добавляем задержку перед восстановлением главного меню
            vTaskDelay(pdMS_TO_TICKS(50));
            
            // Восстанавливаем главный экран
            if (main_screen && lv_obj_is_valid(main_screen)) {
                lv_scr_load(main_screen);
            }
            
            // Очищаем экран пароля и меню входов/выходов
            password_screen_cleanup();
            input_output_menu_cleanup();
            co_menu_cleanup(); // Очищаем меню отопления
            
            // Пересоздаем главное меню если оно было уничтожено
            extern lv_obj_t *_cont;
            if (_cont == NULL || !lv_obj_is_valid(_cont)) {
                ESP_LOGI(TAG, "Recreating main menu");
                Main_Menu_List();
            }
            
            // Восстанавливаем состояние меню
            screen_navigation_restore_cursor_position();
            
            // Показываем главное меню
            main_menu_show();
            
            // Обновляем отображение
            main_menu_update_display();
            
            encoder_manager_register_callback(screen_navigation_encoder_event_cb);
            break;
            
        case SCREEN_PASSWORD_INPUT:
            // Сохраняем состояние меню перед переходом
            screen_navigation_save_cursor_position();
            
            // Скрываем главное меню
            main_menu_hide();
            
            // Небольшая задержка для завершения операций LVGL
            vTaskDelay(pdMS_TO_TICKS(50));
            
            // Создаем контейнер для экрана пароля
            current_content_container = screen_container_create(CONTAINER_TYPE_PASSWORD);
            
            // Создаем экран пароля
            password_screen();
            encoder_manager_register_callback(password_encoder_event_cb);
            break;
            
        case SCREEN_GVS:
            // TODO: Реализовать переход на экран ГВС
            ESP_LOGI(TAG, "Transition to GVS screen not implemented yet");
            // Временно возвращаемся в главное меню
            screen_navigation_go_to(SCREEN_MAIN_MENU);
            break;
            
        case SCREEN_CO:
            // Переход в меню отопления
            ESP_LOGI(TAG, "Transition to CO menu");
            
            // Проверяем, не происходит ли уже переход
            static bool co_transition_in_progress = false;
            if (co_transition_in_progress) {
                ESP_LOGW(TAG, "CO transition already in progress, skipping");
                return;
            }
            co_transition_in_progress = true;
            
            // Сохраняем состояние меню перед переходом
            screen_navigation_save_cursor_position();
            
            // Скрываем главное меню
            main_menu_hide();
            
            // Даем больше времени на завершение операций
            vTaskDelay(pdMS_TO_TICKS(150));
            
            // Очищаем текущий контейнер
            screen_navigation_cleanup_current_container();
            
            // Создаем контейнер для экрана отопления
            current_content_container = screen_container_create(CONTAINER_TYPE_MAIN_MENU);
            
            // Даем время на создание контейнера
            vTaskDelay(pdMS_TO_TICKS(50));
            
            // СОЗДАЕМ ОБЕРТКУ ДЛЯ КОНТЕНТА
            lv_obj_t *screen_wrapper = screen_content_wrapper_create(current_content_container);
            
            // Создаем интерфейс экрана отопления в обертке (левая часть остается)
            screen_CO_create(screen_wrapper);
            
            // Даем время на создание интерфейса отопления
            vTaskDelay(pdMS_TO_TICKS(50));
            
            // Создаем меню отопления (правая часть меняется)
            CO_Menu_List();
            
            // ДАЕМ ВРЕМЯ НА ПОЛНУЮ ИНИЦИАЛИЗАЦИЮ МЕНЮ
            vTaskDelay(pdMS_TO_TICKS(100));
            
            // ОБНОВЛЯЕМ ДУГОВОЕ МЕНЮ ДЛЯ КОРРЕКТНОГО ОТОБРАЖЕНИЯ
            // Теперь co_cont доступна через CO_main_menu.h
            if (co_cont && lv_obj_is_valid(co_cont)) {
                arc_menu_update_slide(co_cont);
            }
            
            encoder_manager_register_callback(co_menu_encoder_event_cb);
            co_transition_in_progress = false;
            break;
            
        case SCREEN_PODP:
            // TODO: Реализовать переход на экран подпитки
            ESP_LOGI(TAG, "Transition to PODP screen not implemented yet");
            // Временно возвращаемся в главное меню
            screen_navigation_go_to(SCREEN_MAIN_MENU);
            break;
            
        case SCREEN_UV:
            // TODO: Реализовать переход на экран узла ввода
            ESP_LOGI(TAG, "Transition to UV screen not implemented yet");
            // Временно возвращаемся в главное меню
            screen_navigation_go_to(SCREEN_MAIN_MENU);
            break;        

        case SCREEN_ALARMS:
            // TODO: Реализовать переход на экран аварий
            ESP_LOGI(TAG, "Transition to SCREEN_ALARMS screen not implemented yet");
            // Временно возвращаемся в главное меню
            screen_navigation_go_to(SCREEN_MAIN_MENU);
            break;

        case SCREEN_IN_OUT:
            // Сохраняем состояние меню перед переходом
            screen_navigation_save_cursor_position();
            
            // Скрываем главное меню
            main_menu_hide();
            
            // Даем время на завершение операций
            vTaskDelay(pdMS_TO_TICKS(50));
            
            // Очищаем текущий контейнер
            screen_navigation_cleanup_current_container();
            
            // Создаем контейнер для экрана входов/выходов
            current_content_container = screen_container_create(CONTAINER_TYPE_IN_OUT);
            
            // СОЗДАЕМ ОБЕРТКУ ДЛЯ КОНТЕНТА
            lv_obj_t *screen_wrapper_io = screen_content_wrapper_create(current_content_container);
            
            // Создаем интерфейс экрана входов/выходов в обертке
            screen_In_Out_create(screen_wrapper_io);
            
            // Создаем меню входов/выходов
            Input_Output_Menu_List();
            encoder_manager_register_callback(input_output_encoder_event_cb);
            break;

        case SCREEN_SERVICE:
            // TODO: Реализовать переход на экран сервиса
            ESP_LOGI(TAG, "Transition to SCREEN_SERVICE screen not implemented yet");
            // Временно возвращаемся в главное меню
            screen_navigation_go_to(SCREEN_MAIN_MENU);
            break;
            
        default:
            ESP_LOGE(TAG, "Unknown screen type: %d", screen);
            // В случае неизвестного экрана возвращаемся в главное меню
            screen_navigation_go_to(SCREEN_MAIN_MENU);
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
            menu_state_t *menu_state = get_menu_state(MENU_TYPE_MAIN);
            ESP_LOGI(TAG, "Click detected on menu item: %" PRIu32, menu_state->cursor_index);
            
            switch(menu_state->cursor_index) {
                case 0: // "Открыть доступ"
                    ESP_LOGI(TAG, "Navigating to password screen");
                    screen_navigation_go_to(SCREEN_PASSWORD_INPUT);
                    break;
                case 1: // "ГВС"
                    ESP_LOGI(TAG, "Navigating to GVS screen");
                    screen_navigation_go_to(SCREEN_GVS);
                    break;
                case 2: // "Отопление"
                    ESP_LOGI(TAG, "Navigating to CO menu");
                    screen_navigation_go_to(SCREEN_CO);
                    break;
                case 3: // "Аварии"
                    ESP_LOGI(TAG, "Navigating to alarms screen");
                    screen_navigation_go_to(SCREEN_ALARMS);
                    break;
                case 4: // "Входы/выходы"
                    ESP_LOGI(TAG, "Navigating to input/output screen");
                    screen_navigation_go_to(SCREEN_IN_OUT);
                    break;
                case 5: // "Сервис"
                    ESP_LOGI(TAG, "Navigating to service screen");
                    screen_navigation_go_to(SCREEN_SERVICE);
                    break;
                default:
                    ESP_LOGI(TAG, "No action defined for menu item: %" PRIu32, menu_state->cursor_index);
                    break;
            }
        }
    }
}

screen_type_t screen_navigation_get_current_screen(void) {
    return current_screen;
}
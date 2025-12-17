#include "screen_navigation.h"
#include "menu_layer/main_menu/main_menu.h"
#include "screens/S_Pass/password_screen.h"
#include "screens/S_Pass/screen_Pass.h"
#include "screen_logic/access_control.h"
#include "encoder/encoder_manager.h"
#include "menu_layer/In_Out_Menu/In_Out_main_menu.h"
#include "menu_layer/CO_Menu/CO_main_menu.h"
#include "menu_layer/GVS_Menu/GVS_main_menu.h"
#include "menu_layer/GVS_Menu/GVS_general_menu.h"
#include "menu_layer/GVS_Menu/GVS_pumps_menu.h"
#include "menu_layer/GVS_Menu/GVS_valve_menu.h"
#include "menu_layer/GVS_Menu/GVS_manual_menu.h"
#include "menu_layer/GVS_Menu/GVS_schedule_menu.h"
#include "menu_layer/GVS_Menu/GVS_alarms_menu.h"
#include "menu_layer/GVS_Menu/GVS_dry_run_menu.h"
#include "screens/S_In_Out/1_layer/screen_In_Out.h"
#include "screens/S_Co/screen_CO.h"
#include "screens/S_Gvs/screen_Gvs.h"
#include "screen_container_manager.h"
#include "arc_menu.h"
#include <inttypes.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SCREEN_NAV";

static screen_type_t current_screen = SCREEN_MAIN_MENU;
static lv_obj_t *main_screen = NULL;

// Переменные для сохранения позиции курсора
static menu_state_t saved_menu_state = {0};
static bool menu_state_saved = false;

// Контейнеры для разных экранов
static lv_obj_t *current_content_container = NULL;

// Флаг для предотвращения рекурсивных переходов
static bool navigation_in_progress = false;

// Объявления внешних функций
extern void main_menu_show(void);
extern void main_menu_hide(void);
extern void main_menu_cleanup(void);
extern void input_output_menu_show(void);
extern void input_output_menu_hide(void);
extern void co_menu_show(void);
extern void co_menu_hide(void);

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
 * @brief Очищает текущий контейнер контента (без блокирующих задержек)
 */
static void screen_navigation_cleanup_current_container(void) {
    if (current_content_container && lv_obj_is_valid(current_content_container)) {
        ESP_LOGI(TAG, "Cleaning up current container");
        
        // Очищаем подсветки перед удалением контейнера
        if (current_screen == SCREEN_IN_OUT) {
            screen_In_Out_cleanup_highlights();
        }
        
        // Используем безопасное удаление
        lv_obj_t *container_to_delete = current_content_container;
        current_content_container = NULL;
        
        // Удаляем асинхронно без задержек
        if (lv_obj_is_valid(container_to_delete)) {
            lv_obj_del_async(container_to_delete);
        }
        
        ESP_LOGI(TAG, "Container cleanup scheduled");
    }
}

/**
 * @brief Полная очистка всех меню (при выходе из приложения)
 */
void screen_navigation_full_cleanup(void) {
    ESP_LOGI(TAG, "Performing full cleanup of all menus");
    
    input_output_menu_cleanup();
    co_menu_cleanup();
    main_menu_cleanup();
    password_screen_cleanup();
    
    ESP_LOGI(TAG, "Full cleanup completed");
}

/**
 * @brief Безопасный переход между экранами (без блокирующих операций)
 */
void screen_navigation_go_to(screen_type_t screen) {
    // Защита от рекурсивных вызовов
    if (navigation_in_progress) {
        ESP_LOGW(TAG, "Navigation already in progress, skipping");
        return;
    }
    
    navigation_in_progress = true;
    ESP_LOGI(TAG, "Navigating to screen: %d", screen);
    
    // Убираем текущий обработчик энкодера
    encoder_manager_unregister_callback();
    
    // Очищаем текущий контейнер (быстро, без задержек)
    screen_navigation_cleanup_current_container();
    
    switch(screen) {
        case SCREEN_MAIN_MENU:
            ESP_LOGI(TAG, "Returning to main menu");
            
            // ВАЖНО: Сначала восстанавливаем главный экран, потом очищаем экран пароля
            // Это предотвращает обращение к удаленным объектам
            
            // Восстанавливаем главный экран ПЕРЕД очисткой экрана пароля
            if (main_screen && lv_obj_is_valid(main_screen)) {
                lv_scr_load(main_screen);
                // Даем LVGL время на переключение экрана
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            
            // Скрываем все подменю
            input_output_menu_hide();
            co_menu_hide();
            gvs_menu_hide();
            gvs_general_menu_hide();
            gvs_pumps_menu_hide();
            gvs_valve_menu_hide();
            gvs_manual_menu_hide();
            gvs_schedule_menu_hide();
            gvs_schedule_day_menu_hide();
            gvs_alarms_menu_hide();
            gvs_dry_run_menu_hide();
            
            // Теперь безопасно очищаем экран пароля (главный экран уже активен)
            password_screen_cleanup();
            
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
            
            // Обновляем отображение экрана доступа
            screen_Pass_update_display();
            
            encoder_manager_register_callback(screen_navigation_encoder_event_cb);
            break;
            
        case SCREEN_PASSWORD_INPUT:
            ESP_LOGI(TAG, "Going to password screen");
            
            // Сохраняем состояние меню перед переходом
            screen_navigation_save_cursor_position();
            
            // Скрываем главное меню
            main_menu_hide();
            
            // Создаем контейнер для экрана пароля
            current_content_container = screen_container_create(CONTAINER_TYPE_PASSWORD);
            
            // Создаем экран пароля
            password_screen();
            encoder_manager_register_callback(password_encoder_event_cb);
            break;
            
        case SCREEN_GVS: {
            ESP_LOGI(TAG, "Transition to GVS menu");
            
            // Сохраняем состояние меню перед переходом
            screen_navigation_save_cursor_position();
            
            // Скрываем главное меню
            main_menu_hide();
            
            // Создаем контейнер для экрана ГВС
            current_content_container = screen_container_create(CONTAINER_TYPE_MAIN_MENU);
            
            // Создаем обертку для контента
            lv_obj_t *screen_wrapper = screen_content_wrapper_create(current_content_container);
            
            // Создаем интерфейс экрана ГВС
            screen_Gvs_create(screen_wrapper);
            
            // Создаем меню ГВС
            GVS_Menu_List();
            
            // Показываем меню ГВС
            gvs_menu_show();
            
            // Обновляем дуговое меню
            if (gvs_cont && lv_obj_is_valid(gvs_cont)) {
                arc_menu_update_slide(gvs_cont);
            }
            
            encoder_manager_register_callback(gvs_menu_encoder_event_cb);
            break;
        }
            
        case SCREEN_CO: {
            ESP_LOGI(TAG, "Transition to CO menu");
            
            // Сохраняем состояние меню перед переходом
            screen_navigation_save_cursor_position();
            
            // Скрываем главное меню
            main_menu_hide();
            
            // Создаем контейнер для экрана отопления
            current_content_container = screen_container_create(CONTAINER_TYPE_MAIN_MENU);
            
            // Создаем обертку для контента
            lv_obj_t *screen_wrapper = screen_content_wrapper_create(current_content_container);
            
            // Создаем интерфейс экрана отопления
            screen_CO_create(screen_wrapper);
            
            // Создаем меню отопления
            CO_Menu_List();
            
            // Показываем меню отопления
            co_menu_show();
            
            // Обновляем дуговое меню
            if (co_cont && lv_obj_is_valid(co_cont)) {
                arc_menu_update_slide(co_cont);
            }
            
            encoder_manager_register_callback(co_menu_encoder_event_cb);
            break;
        }
            
        case SCREEN_PODP:
            ESP_LOGI(TAG, "Transition to PODP screen not implemented yet");
            screen_navigation_go_to(SCREEN_MAIN_MENU);
            break;
            
        case SCREEN_UV:
            ESP_LOGI(TAG, "Transition to UV screen not implemented yet");
            screen_navigation_go_to(SCREEN_MAIN_MENU);
            break;        

        case SCREEN_ALARMS:
            ESP_LOGI(TAG, "Transition to SCREEN_ALARMS screen not implemented yet");
            screen_navigation_go_to(SCREEN_MAIN_MENU);
            break;

        case SCREEN_IN_OUT:
            ESP_LOGI(TAG, "Transition to input/output screen");
            
            // Сохраняем состояние меню перед переходом
            screen_navigation_save_cursor_position();
            
            // Скрываем главное меню
            main_menu_hide();
            
            // Создаем контейнер для экрана входов/выходов
            current_content_container = screen_container_create(CONTAINER_TYPE_IN_OUT);
            if (!current_content_container || !lv_obj_is_valid(current_content_container)) {
                ESP_LOGE(TAG, "Failed to create content container for In/Out screen");
                screen_navigation_go_to(SCREEN_MAIN_MENU);
                break;
            }
            
            // Создаем обертку для контента
            lv_obj_t *screen_wrapper_io = screen_content_wrapper_create(current_content_container);
            if (!screen_wrapper_io || !lv_obj_is_valid(screen_wrapper_io)) {
                ESP_LOGE(TAG, "Failed to create content wrapper for In/Out screen");
                screen_navigation_go_to(SCREEN_MAIN_MENU);
                break;
            }
            
            // Даем LVGL время на инициализацию контейнеров перед созданием виджетов
            vTaskDelay(pdMS_TO_TICKS(10));
            
            // Создаем интерфейс экрана входов/выходов
            screen_In_Out_create(screen_wrapper_io);
            
            // Создаем меню входов/выходов
            Input_Output_Menu_List();
            
            // Показываем меню входов/выходов
            input_output_menu_show();
            
            encoder_manager_register_callback(input_output_encoder_event_cb);
            break;

        case SCREEN_SERVICE:
            ESP_LOGI(TAG, "Transition to SCREEN_SERVICE screen not implemented yet");
            screen_navigation_go_to(SCREEN_MAIN_MENU);
            break;
            
        default:
            ESP_LOGE(TAG, "Unknown screen type: %d", screen);
            screen_navigation_go_to(SCREEN_MAIN_MENU);
            break;
    }
    
    current_screen = screen;
    navigation_in_progress = false;
}

void screen_navigation_encoder_event_cb(uint8_t e) {
    // Обновляем таймер активности при любом действии пользователя
    access_control_update_activity_timer();
    
    // Если мы в главном меню
    if (current_screen == SCREEN_MAIN_MENU) {
        // Передаем события в главное меню для обработки движения
        main_menu_encoder_event_cb(e);
        
        // Обработка нажатий в главном меню
        if ((e & ENC_CLICK)) {
            menu_state_t *menu_state = get_menu_state(MENU_TYPE_MAIN);
            ESP_LOGI(TAG, "Click detected on menu item: %" PRIu32, menu_state->cursor_index);
            
            switch(menu_state->cursor_index) {
                case 0: // "Открыть доступ" / "Закрыть доступ"
                    {
                        if (access_control_is_unlocked()) {
                            // Если доступ открыт - закрываем его
                            ESP_LOGI(TAG, "Closing access");
                            access_control_lock();
                            // Обновляем отображение главного меню и экрана доступа
                            main_menu_update_access_display();
                            screen_Pass_update_display();
                        } else {
                            // Если доступ закрыт - открываем экран пароля
                            ESP_LOGI(TAG, "Navigating to password screen");
                            screen_navigation_go_to(SCREEN_PASSWORD_INPUT);
                        }
                    }
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

/**
 * @brief Скрывает контейнер контента экрана CO
 */
void screen_navigation_hide_co_content_container(void) {
    if (current_content_container && lv_obj_is_valid(current_content_container)) {
        screen_container_hide(current_content_container);
    }
}

/**
 * @brief Показывает контейнер контента экрана CO
 */
void screen_navigation_show_co_content_container(void) {
    if (current_content_container && lv_obj_is_valid(current_content_container)) {
        screen_container_show(current_content_container);
    }
}

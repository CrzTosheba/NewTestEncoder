
#include "screen_Pass.h"
#include "my_widgets/w_lock_big_close.h"
#include "my_widgets/w_lock_big_open.h"
#include "screen_logic/access_control.h"
#include <string.h>

// Глобальные указатели на объекты экрана для обновления
static lv_obj_t *lock_image = NULL;
static lv_obj_t *access_label = NULL;
static lv_obj_t *parent_container = NULL;

/**
 * @brief Поиск метки доступа среди дочерних элементов
 */
static lv_obj_t* find_access_label(lv_obj_t *parent) {
    if (parent == NULL || !lv_obj_is_valid(parent)) {
        return NULL;
    }
    
    uint32_t child_cnt = lv_obj_get_child_cnt(parent);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(parent, i);
        if (child && lv_obj_is_valid(child)) {
            // Проверяем, является ли это меткой
            if (lv_obj_check_type(child, &lv_label_class)) {
                const char *text = lv_label_get_text(child);
                if (text != NULL && (strcmp(text, "ДОСТУП ЗАКРЫТ") == 0 || strcmp(text, "ДОСТУП ОТКРЫТ") == 0)) {
                    return child;
                }
            }
        }
    }
    return NULL;
}

/**
 * @brief Обновление отображения экрана доступа в зависимости от состояния
 */
void screen_Pass_update_display(void) {
    if (parent_container == NULL || !lv_obj_is_valid(parent_container)) {
        return;
    }
    
    bool is_unlocked = access_control_is_unlocked();
    
    // Удаляем все дочерние элементы (изображение и метку), чтобы избежать дублирования
    // Функции lock_big_* создают и изображение, и метку, поэтому нужно удалить всё
    uint32_t child_cnt = lv_obj_get_child_cnt(parent_container);
    for (int32_t i = child_cnt - 1; i >= 0; i--) {
        lv_obj_t *child = lv_obj_get_child(parent_container, i);
        if (child && lv_obj_is_valid(child)) {
            lv_obj_del(child);
        }
    }
    
    // Сбрасываем указатели
    lock_image = NULL;
    access_label = NULL;
    
    // Создаем новые объекты в зависимости от состояния
    // Функции lock_big_* сами создадут и изображение, и метку
    if (is_unlocked) {
        lock_image = lock_big_open(parent_container);
    } else {
        lock_image = lock_big_close(parent_container);
    }
    
    // Находим созданную метку для сохранения указателя (опционально)
    access_label = find_access_label(parent_container);
}

void screen_Pass_create(lv_obj_t *parent) {
    parent_container = parent;
    
    // Изначально показываем состояние доступа
    screen_Pass_update_display();
}
#ifndef PARAM_MENU_COMMON_H
#define PARAM_MENU_COMMON_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Объявления шрифтов
LV_FONT_DECLARE(Roboto_bold_24);
LV_IMG_DECLARE(lv_im_arrow_right);

// Стандартные настройки отображения параметров в меню
// (унифицированы на основе CO_pumps_menu.c)

// Выравнивание названия параметра (смещение от левого края)
#define PARAM_LABEL_ALIGN_X -5

// Позиция контейнера значения параметра (x, y относительно родителя)
#define PARAM_VALUE_CONTAINER_X 240
#define PARAM_VALUE_CONTAINER_Y -23

// Размер контейнера значения параметра
#define PARAM_VALUE_CONTAINER_WIDTH 83
#define PARAM_VALUE_CONTAINER_HEIGHT 40

// Выравнивание текста значения параметра внутри контейнера
#define PARAM_VALUE_LABEL_ALIGN LV_ALIGN_TOP_RIGHT

// Цвета для параметров
#define PARAM_BG_COLOR_DEFAULT 0x2B3639
#define PARAM_BG_COLOR_SELECTED 0xFFCC00
#define PARAM_BG_COLOR_EDITING 0xE9EBEB
#define PARAM_TEXT_COLOR_DEFAULT 0xFFFFFF
#define PARAM_TEXT_COLOR_SELECTED 0x000000
#define PARAM_TEXT_COLOR_EDITING 0x101315

/**
 * @brief Создает label для названия параметра с унифицированными настройками
 * @param parent Родительский контейнер (box)
 * @param text Текст названия параметра
 * @return Указатель на созданный label или NULL при ошибке
 */
lv_obj_t* param_menu_create_label(lv_obj_t *parent, const char *text);

/**
 * @brief Создает контейнер и label для значения параметра с унифицированными настройками
 * @param parent Родительский контейнер (box)
 * @param value_label_ptr Указатель на указатель для сохранения label значения (может быть NULL)
 * @return Указатель на созданный контейнер значения или NULL при ошибке
 */
lv_obj_t* param_menu_create_value_container(lv_obj_t *parent, lv_obj_t **value_label_ptr);

#ifdef __cplusplus
}
#endif

#endif // PARAM_MENU_COMMON_H


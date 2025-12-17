#include "co_heating_graph_display.h"
#include "CO_heating_graph_params.h"
#include "CO_heating_graph_menu.h"
#include "esp_log.h"
#include <stdbool.h>

// Используем функции API для работы с данными серий вместо прямого доступа к полям

// Объявление шрифта
LV_FONT_DECLARE(Roboto_bold_24);

static const char *TAG = "CO_HEATING_GRAPH_DISPLAY";

// Статические объекты графика
static lv_obj_t *chart = NULL;
static lv_chart_series_t *ser_points = NULL;
static lv_obj_t *markers[6] = {NULL};
static lv_obj_t *x_label = NULL;
static lv_obj_t *y_label = NULL;
static lv_obj_t *x_tick_labels[6] = {NULL};  // Метки для цифр на делениях оси X (максимум 6)
static lv_obj_t *y_tick_labels[6] = {NULL};  // Метки для цифр на делениях оси Y (максимум 6)
static lv_obj_t *dashed_line_x = NULL;
static lv_obj_t *dashed_line_y = NULL;
static lv_obj_t *right_border_line = NULL;  // Правая граница графика (серая)
static lv_chart_series_t *ser_red_line_left = NULL;
static lv_chart_series_t *ser_red_line_right = NULL;
static lv_obj_t *title_container = NULL;

// Состояние отображения
static int current_selected_param_index = -1;  // -1 означает "не выбрано"
static bool current_is_editing = false;
static int current_editing_param_index = -1;

/**
 * @brief Проверяет, является ли объект валидным
 */
static bool is_obj_valid(lv_obj_t *obj) {
    return obj != NULL && lv_obj_is_valid(obj);
}

/**
 * @brief Получает координаты точки по номеру точки (0-5)
 */
static void get_point_coords(int point_idx, float *x, float *y) {
    switch(point_idx) {
        case 0: *x = C1_T0_1; *y = C1_T1_Desired_1; break;
        case 1: *x = C1_T0_2; *y = C1_T1_Desired_2; break;
        case 2: *x = C1_T0_3; *y = C1_T1_Desired_3; break;
        case 3: *x = C1_T0_4; *y = C1_T1_Desired_4; break;
        case 4: *x = C1_T0_5; *y = C1_T1_Desired_5; break;
        case 5: *x = C1_T0_6; *y = C1_T1_Desired_6; break;
        default: *x = 0; *y = 0; break;
    }
}

/**
 * @brief Получает координаты точки в режиме редактирования
 */
static void get_editing_point_coords(int point_idx, int axis, float *x, float *y, float editing_value) {
    // Сначала получаем текущие координаты
    get_point_coords(point_idx, x, y);
    
    // Затем обновляем редактируемую координату
    if (axis == 0) {  // X axis (Тнв)
        *x = editing_value;
    } else {  // Y axis (Тпод_CO)
        *y = editing_value;
    }
}

/**
 * @brief Преобразует param_index в номер точки и ось
 * param_index 3 -> point 0, axis 0 (X)
 * param_index 4 -> point 0, axis 1 (Y)
 * param_index 5 -> point 1, axis 0 (X)
 * и т.д.
 */
static void param_index_to_point_axis(int param_index, int *point_idx, int *axis) {
    if (param_index >= 3 && param_index <= 14) {
        *point_idx = (param_index - 3) / 2;
        *axis = (param_index - 3) % 2;
    } else {
        *point_idx = -1;
        *axis = -1;
    }
}

/**
 * @brief Обновляет внешний вид маркера
 */
static void update_marker_appearance(int point_idx) {
    if (point_idx < 0 || point_idx >= 6 || !markers[point_idx]) return;
    
    lv_obj_t *img = lv_obj_get_child(markers[point_idx], 0);
    if (!img) return;
    
    // Получаем label для номера точки
    lv_obj_t *label = lv_obj_get_child(markers[point_idx], 1);
    
    // Проверяем, выбрана ли эта точка и редактируется ли она
    bool is_editing_this = false;
    int selected_axis = 0;
    
    if (current_selected_param_index >= 3 && current_selected_param_index <= 14) {
        int selected_point_idx, axis;
        param_index_to_point_axis(current_selected_param_index, &selected_point_idx, &axis);
        if (selected_point_idx == point_idx) {
            selected_axis = axis;
            is_editing_this = (current_is_editing && current_editing_param_index == current_selected_param_index);
        }
    }
    
    // Цвет маркера меняется ТОЛЬКО в режиме редактирования
    if (is_editing_this) {
        // Режим редактирования - желтый маркер с иконкой изменения
        lv_img_set_src(img, selected_axis == 0 ? 
            &lv_im_point_yellow_change_x : 
            &lv_im_point_yellow_change_y);
    } else {
        // ВСЕГДА белый маркер (и когда выбрано, и когда не выбрано)
        lv_img_set_src(img, &lv_im_point_white);
    }
    
    // Цифры ВСЕГДА черные и всегда видны
    if (label) {
        lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);
        // Убеждаемся, что label не скрыт
        lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Обновляет пунктирные линии
 */
static void update_dashed_lines(void) {
    if (!chart) return;
    
    // Перемещение линий на передний план
    if (dashed_line_x && lv_obj_is_valid(dashed_line_x)) {
        lv_obj_move_foreground(dashed_line_x);
    }
    if (dashed_line_y && lv_obj_is_valid(dashed_line_y)) {
        lv_obj_move_foreground(dashed_line_y);
    }
    
    // Определение текущего цвета
    uint32_t current_color = current_is_editing ? GRAPH_DASH_LINE_COLOR_EDIT : GRAPH_DASH_LINE_COLOR_NORMAL;
    
    // Проверка наличия выбранной точки
    if (current_selected_param_index >= 3 && current_selected_param_index <= 14) {
        int point_idx, axis;
        param_index_to_point_axis(current_selected_param_index, &point_idx, &axis);
        
        if (point_idx >= 0 && point_idx < C1_Number) {
            // Получаем позицию точки на графике (координаты уже обновлены в update_chart_points)
            lv_point_t p;
            lv_chart_get_point_pos_by_id(chart, ser_points, point_idx, &p);
            
            if (axis == 0) {
                // Горизонтальная линия (X axis - Тнв)
                if (dashed_line_x) {
                    lv_coord_t y_pos = p.y + GRAPH_DASH_LINE_Y_OFFSET;
                    lv_coord_t chart_w = lv_obj_get_width(chart);
                    static lv_point_precise_t points_x[2];
                    points_x[0].x = 0.0f;
                    points_x[0].y = (float)y_pos;
                    points_x[1].x = (float)chart_w;
                    points_x[1].y = (float)y_pos;
                    
                    lv_line_set_points(dashed_line_x, points_x, 2);
                    lv_obj_set_style_line_color(dashed_line_x, lv_color_hex(current_color), 0);
                    lv_obj_set_style_line_dash_width(dashed_line_x, GRAPH_DASH_LINE_WIDTH, 0);
                    lv_obj_set_style_line_dash_gap(dashed_line_x, GRAPH_DASH_GAP, 0);
                    lv_obj_set_style_line_width(dashed_line_x, 2, 0);
                    
                    lv_obj_clear_flag(dashed_line_x, LV_OBJ_FLAG_HIDDEN);
                }
                if (dashed_line_y) {
                    lv_obj_add_flag(dashed_line_y, LV_OBJ_FLAG_HIDDEN);
                }
            } else {
                // Вертикальная линия (Y axis - Тпод_CO)
                if (dashed_line_y) {
                    lv_coord_t x_pos = p.x + GRAPH_DASH_LINE_X_OFFSET;
                    lv_coord_t chart_h = lv_obj_get_height(chart);
                    static lv_point_precise_t points_y[2];
                    points_y[0].x = (float)x_pos;
                    points_y[0].y = 0.0f;
                    points_y[1].x = (float)x_pos;
                    points_y[1].y = (float)chart_h;
                    
                    lv_line_set_points(dashed_line_y, points_y, 2);
                    lv_obj_set_style_line_color(dashed_line_y, lv_color_hex(current_color), 0);
                    lv_obj_set_style_line_dash_width(dashed_line_y, GRAPH_DASH_LINE_WIDTH, 0);
                    lv_obj_set_style_line_dash_gap(dashed_line_y, GRAPH_DASH_GAP, 0);
                    lv_obj_set_style_line_width(dashed_line_y, 2, 0);
                    
                    lv_obj_clear_flag(dashed_line_y, LV_OBJ_FLAG_HIDDEN);
                }
                if (dashed_line_x) {
                    lv_obj_add_flag(dashed_line_x, LV_OBJ_FLAG_HIDDEN);
                }
            }
        } else {
            // Скрываем обе линии если точка вне диапазона
            if (dashed_line_x) lv_obj_add_flag(dashed_line_x, LV_OBJ_FLAG_HIDDEN);
            if (dashed_line_y) lv_obj_add_flag(dashed_line_y, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        // Скрываем обе линии если нет выбора
        if (dashed_line_x) lv_obj_add_flag(dashed_line_x, LV_OBJ_FLAG_HIDDEN);
        if (dashed_line_y) lv_obj_add_flag(dashed_line_y, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Создает оси графика и подписи
 */
static void create_chart_axes(void) {
    if (!chart) return;
    
    // Настройка осей
    lv_obj_set_style_line_color(chart, lv_color_hex(GRAPH_CHART_LINE_COLOR), LV_PART_MAIN);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, GRAPH_CHART_MIN_X, GRAPH_CHART_MAX_X);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, GRAPH_CHART_MIN_Y, GRAPH_CHART_MAX_Y);
    // Проблема: количество линий сетки должно совпадать с количеством промежуточных меток
    // Для оси X: 5 меток всего (-50, -30, -10, 10, 30), 3 промежуточные (-30, -10, 10) -> нужно 3 вертикальные линии
    // Для оси Y: 6 меток всего (0, 20, 40, 60, 80, 100), 4 промежуточные (20, 40, 60, 80) -> нужно 4 горизонтальные линии
    // LVGL рисует линии по формуле: позиция = (размер * i) / (count - 1), где i от i_start до i_end-1
    // LVGL пропускает первую линию (i=0) если есть border слева/сверху и padding=0
    // LVGL пропускает последнюю линию (i=count-1) если есть border справа/снизу и padding=0
    // Чтобы получить нужное количество линий на промежуточных метках, нужно установить count = количество_промежуточных_меток + 2
    // Но промежуточные метки = общее_количество - 2 (без граничных)
    // Поэтому: count = (общее_количество - 2) + 2 = общее_количество
    // Но тогда получим линии на всех метках включая граничные, которые будут пропущены -> останутся только на промежуточных
    // Для оси X: vdiv_cnt = 5 (3 промежуточные линии после пропуска граничных)
    // Для оси Y: hdiv_cnt = 6 (4 промежуточные линии после пропуска граничных)
    // НЕТ, это не работает правильно. Попробуем другой подход:
    // Для оси X: нужно 3 линии на -30, -10, 10. Если vdiv_cnt=4, линии будут на: 0, w/3, 2w/3, w
    //   После пропуска первой (border слева): w/3, 2w/3, w - НЕ СОВПАДАЕТ с метками
    // Если vdiv_cnt=5, линии будут на: 0, w/4, w/2, 3w/4, w
    //   После пропуска первой: w/4, w/2, 3w/4, w - НЕ СОВПАДАЕТ (метки на w/4, w/2, 3w/4, но линия на w лишняя)
    // Для совпадения количества линий с промежуточными метками:
    // Ось X: 3 промежуточные метки (-30, -10, 10) -> используем vdiv_cnt = количество меток (5)
    //   LVGL пропустит первую линию (border слева), останется 4 линии на позициях w/4, w/2, 3w/4, w
    //   Но метки на позициях w/4, w/2, 3w/4, w - значит линия на w лишняя!
    //   Используем vdiv_cnt = 4, линии на: 0, w/3, 2w/3, w -> после пропуска первой: w/3, 2w/3, w - НЕ СОВПАДАЕТ
    // Правильное решение: использовать vdiv_cnt = количество меток, но тогда будет лишняя линия на правой границе
    // Ось Y: 4 промежуточные метки (20, 40, 60, 80) -> используем hdiv_cnt = количество меток (6)
    //   LVGL пропустит последнюю линию (border снизу), останется 5 линий на позициях 0, h/5, 2h/5, 3h/5, 4h/5
    //   Но метки на позициях h, 4h/5, 3h/5, 2h/5, h/5, 0 - значит линия на 0 (верхняя граница) лишняя!
    // Используем количество меток для правильного позиционирования, лишние линии на границах будут пропущены
    lv_chart_set_div_line_count(chart, GRAPH_AXIS_TICK_COUNT_X, GRAPH_AXIS_TICK_COUNT_Y);
    
    // Удаление старых меток если есть
    if (x_label && is_obj_valid(x_label)) {
        lv_obj_del(x_label);
        x_label = NULL;
    }
    if (y_label && is_obj_valid(y_label)) {
        lv_obj_del(y_label);
        y_label = NULL;
    }
    
    // Удаление старых меток делений
    // Используем максимальное значение из двух осей для безопасного удаления всех возможных меток
    int max_tick_count = (GRAPH_AXIS_TICK_COUNT_X > GRAPH_AXIS_TICK_COUNT_Y) ? GRAPH_AXIS_TICK_COUNT_X : GRAPH_AXIS_TICK_COUNT_Y;
    for (int i = 0; i < max_tick_count && i < 6; i++) {
        if (x_tick_labels[i] && is_obj_valid(x_tick_labels[i])) {
            lv_obj_del(x_tick_labels[i]);
            x_tick_labels[i] = NULL;
        }
        if (y_tick_labels[i] && is_obj_valid(y_tick_labels[i])) {
            lv_obj_del(y_tick_labels[i]);
            y_tick_labels[i] = NULL;
        }
    }
    
    // Создание метки оси X (Тнв) - справа внизу графика
    // Создаем на экране, а не как дочерний элемент графика, чтобы не обрезалось границами
    x_label = lv_label_create(lv_scr_act());
    lv_label_set_text(x_label, "Тнв °C");
    // Позиционируем справа внизу графика
    // Вычисляем позицию: правый край графика минус ширина текста минус отступ
    lv_obj_set_pos(x_label, 
                   GRAPH_CHART_X_POS + GRAPH_CHART_WIDTH - GRAPH_X_AXIS_LABEL_OFFSET_FROM_RIGHT,
                   GRAPH_CHART_Y_POS + GRAPH_CHART_HEIGHT + GRAPH_X_AXIS_LABEL_OFFSET_Y);
    lv_obj_set_style_text_color(x_label, lv_color_hex(GRAPH_AXIS_LABEL_COLOR), 0);
    lv_obj_set_style_text_font(x_label, &Roboto_bold_18, 0);
    
    // Создание метки оси Y (Тпод) - слева вверху графика
    // Создаем на экране, а не как дочерний элемент графика, чтобы не обрезалось границами
    y_label = lv_label_create(lv_scr_act());
    lv_label_set_text(y_label, "Тпод °C");
    // Позиционируем слева вверху графика
    lv_obj_set_pos(y_label, 
                   GRAPH_CHART_X_POS + GRAPH_Y_AXIS_LABEL_OFFSET_X,
                   GRAPH_CHART_Y_POS + GRAPH_Y_AXIS_LABEL_OFFSET_Y);
    lv_obj_set_style_text_color(y_label, lv_color_hex(GRAPH_AXIS_LABEL_COLOR), 0);
    lv_obj_set_style_text_font(y_label, &Roboto_bold_18, 0);
    
    // Создание меток для цифр на делениях оси X
    int32_t x_range = GRAPH_CHART_MAX_X - GRAPH_CHART_MIN_X;
    for (int i = 0; i < GRAPH_AXIS_TICK_COUNT_X; i++) {
        int32_t tick_value = GRAPH_CHART_MIN_X + (x_range * i) / (GRAPH_AXIS_TICK_COUNT_X - 1);
        x_tick_labels[i] = lv_label_create(lv_scr_act());
        lv_label_set_text_fmt(x_tick_labels[i], "%d", (int)tick_value);
        // Позиционируем под графиком, равномерно распределяя по ширине
        // Для последней метки не вычитаем CENTER_OFFSET полностью, чтобы она не перекрывалась
        int calculated_x = (GRAPH_CHART_WIDTH * i) / (GRAPH_AXIS_TICK_COUNT_X - 1);
        int center_offset = (i == GRAPH_AXIS_TICK_COUNT_X - 1) ? GRAPH_X_TICK_LABEL_CENTER_OFFSET / 2 : GRAPH_X_TICK_LABEL_CENTER_OFFSET;
        int x_pos = GRAPH_CHART_X_POS + calculated_x - center_offset;
        int y_pos = GRAPH_CHART_Y_POS + GRAPH_CHART_HEIGHT + GRAPH_X_TICK_LABEL_OFFSET_Y;
        lv_obj_set_pos(x_tick_labels[i], x_pos, y_pos);
        lv_obj_set_style_text_color(x_tick_labels[i], lv_color_hex(GRAPH_AXIS_TICK_COLOR), 0);
        lv_obj_set_style_text_font(x_tick_labels[i], &Roboto_bold_18, 0);
    }
    
    // Создание меток для цифр на делениях оси Y
    int32_t y_range = GRAPH_CHART_MAX_Y - GRAPH_CHART_MIN_Y;
    for (int i = 0; i < GRAPH_AXIS_TICK_COUNT_Y; i++) {
        int32_t tick_value = GRAPH_CHART_MIN_Y + (y_range * i) / (GRAPH_AXIS_TICK_COUNT_Y - 1);
        y_tick_labels[i] = lv_label_create(lv_scr_act());
        lv_label_set_text_fmt(y_tick_labels[i], "%d", (int)tick_value);
        // Позиционируем слева от графика, равномерно распределяя по высоте
        int x_pos = GRAPH_CHART_X_POS + GRAPH_Y_TICK_LABEL_OFFSET_X;
        int calculated_y_in_chart = (GRAPH_CHART_HEIGHT * (GRAPH_AXIS_TICK_COUNT_Y - 1 - i)) / (GRAPH_AXIS_TICK_COUNT_Y - 1);
        int y_pos = GRAPH_CHART_Y_POS + calculated_y_in_chart - GRAPH_Y_TICK_LABEL_CENTER_OFFSET;
        lv_obj_set_pos(y_tick_labels[i], x_pos, y_pos);
        lv_obj_set_style_text_color(y_tick_labels[i], lv_color_hex(GRAPH_AXIS_TICK_COLOR), 0);
        lv_obj_set_style_text_font(y_tick_labels[i], &Roboto_bold_18, 0);
    }
}

/**
 * @brief Обновляет позиции маркеров и их внешний вид
 */
static void update_markers(void) {
    if (!chart || !ser_points) return;
    
    // Обход всех маркеров до C1_Number
    for (int i = 0; i < C1_Number && i < 6; i++) {
        if (!markers[i]) continue;
        
        // Получаем координаты точки
        float x, y;
        if (current_is_editing && current_editing_param_index >= 3 && current_editing_param_index <= 14) {
            int edit_point_idx, edit_axis;
            param_index_to_point_axis(current_editing_param_index, &edit_point_idx, &edit_axis);
            if (edit_point_idx == i) {
                if (co_heating_graph_menu_get_edit_mode()) {
                    float editing_value = co_heating_graph_menu_get_editing_float_value();
                    get_editing_point_coords(i, edit_axis, &x, &y, editing_value);
                } else {
                    get_point_coords(i, &x, &y);
                }
            } else {
                get_point_coords(i, &x, &y);
            }
        } else {
            get_point_coords(i, &x, &y);
        }
        
        // Обновляем координаты точки на графике через API
        lv_chart_set_series_value_by_id2(chart, ser_points, i, (int32_t)x, (int32_t)y);
        
        // Получаем позицию точки
        lv_point_t p;
        lv_chart_get_point_pos_by_id(chart, ser_points, i, &p);
        
        // Позиционирование маркера
        lv_obj_set_pos(markers[i], 
            lv_obj_get_x(chart) + p.x - GRAPH_MARKER_X_OFFSET, 
            lv_obj_get_y(chart) + p.y - GRAPH_MARKER_Y_OFFSET);
        
        // Обновление внешнего вида
        update_marker_appearance(i);
    }
    
    // Скрываем неиспользуемые маркеры
    for (int i = C1_Number; i < 6; i++) {
        if (markers[i]) {
            lv_obj_add_flag(markers[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    update_dashed_lines();
}

/**
 * @brief Обновляет данные точек на графике
 */
static void update_chart_points(void) {
    if (!chart || !ser_points) return;
    
    // Устанавливаем количество видимых точек на графике в соответствии с C1_Number
    lv_chart_set_point_count(chart, C1_Number);
    
    // Обновляем координаты точек
    for (int i = 0; i < C1_Number && i < 6; i++) {
        float x, y;
        if (current_is_editing && current_editing_param_index >= 3 && current_editing_param_index <= 14) {
            int edit_point_idx, edit_axis;
            param_index_to_point_axis(current_editing_param_index, &edit_point_idx, &edit_axis);
            if (edit_point_idx == i) {
                if (co_heating_graph_menu_get_edit_mode()) {
                    float editing_value = co_heating_graph_menu_get_editing_float_value();
                    get_editing_point_coords(i, edit_axis, &x, &y, editing_value);
                } else {
                    get_point_coords(i, &x, &y);
                }
            } else {
                get_point_coords(i, &x, &y);
            }
        } else {
            get_point_coords(i, &x, &y);
        }
        
        // Обновляем координаты точки на графике через API
        lv_chart_set_series_value_by_id2(chart, ser_points, i, (int32_t)x, (int32_t)y);
    }
    
    // Обновляем красные линии (горизонтальные продолжения от первой и до последней точки)
    if (ser_red_line_left && C1_Number > 0) {
        float first_x, first_y;
        get_point_coords(0, &first_x, &first_y);
        lv_chart_set_series_value_by_id2(chart, ser_red_line_left, 0, GRAPH_CHART_MIN_X, (int32_t)first_y);
        lv_chart_set_series_value_by_id2(chart, ser_red_line_left, 1, (int32_t)first_x, (int32_t)first_y);
    }
    
    if (ser_red_line_right && C1_Number > 0) {
        int last_idx = C1_Number - 1;
        float last_x, last_y;
        get_point_coords(last_idx, &last_x, &last_y);
        lv_chart_set_series_value_by_id2(chart, ser_red_line_right, 0, (int32_t)last_x, (int32_t)last_y);
        lv_chart_set_series_value_by_id2(chart, ser_red_line_right, 1, GRAPH_CHART_MAX_X, (int32_t)last_y);
    }
    
    lv_chart_refresh(chart);
    update_markers();
}

/**
 * @brief Инициализирует график отопления
 */
void co_heating_graph_display_init(void) {
    // Проверяем не только наличие указателя, но и валидность объекта
    if (chart) {
        if (is_obj_valid(chart)) {
            // График уже создан и валиден, просто обновляем точки
            update_chart_points();
            return;
        } else {
            // Chart существует, но невалиден - сбрасываем указатель
            chart = NULL;
        }
    }
    
    ESP_LOGI(TAG, "Initializing heating graph display");
    
    // Создание графика
    chart = lv_chart_create(lv_scr_act());
    lv_obj_set_size(chart, GRAPH_CHART_WIDTH, GRAPH_CHART_HEIGHT);
    lv_obj_set_pos(chart, GRAPH_CHART_X_POS, GRAPH_CHART_Y_POS);
    
    // Настройка цветов и стилей графика
    lv_obj_set_style_bg_color(chart, lv_color_hex(GRAPH_CHART_BG_COLOR), 0);
    lv_obj_set_style_radius(chart, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(chart, 0, LV_PART_ITEMS);
    lv_obj_set_style_radius(chart, 0, LV_PART_INDICATOR);
    
    lv_chart_set_type(chart, LV_CHART_TYPE_SCATTER);
    // Устанавливаем количество линий равное количеству меток
    // LVGL автоматически пропустит линии на границах (где есть border)
    lv_chart_set_div_line_count(chart, GRAPH_AXIS_TICK_COUNT_X, GRAPH_AXIS_TICK_COUNT_Y);
    
    lv_obj_set_style_border_color(chart, lv_color_hex(GRAPH_CHART_BORDER_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(chart, GRAPH_CHART_BORDER_SIDE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(chart, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(chart, GRAPH_CHART_PADDING, LV_PART_MAIN);
    
    // Создание правой границы графика (серая линия)
    right_border_line = lv_line_create(chart);
    static lv_point_precise_t right_border_points[2];
    right_border_points[0].x = (float)GRAPH_CHART_WIDTH - 1.0f;  // Правая граница графика
    right_border_points[0].y = 0.0f;
    right_border_points[1].x = (float)GRAPH_CHART_WIDTH - 1.0f;
    right_border_points[1].y = (float)GRAPH_CHART_HEIGHT;
    lv_line_set_points(right_border_line, right_border_points, 2);
    lv_obj_set_style_line_width(right_border_line, GRAPH_CHART_RIGHT_BORDER_WIDTH, 0);
    lv_obj_set_style_line_color(right_border_line, lv_color_hex(GRAPH_CHART_RIGHT_BORDER_COLOR), 0);
    
    // Создание горизонтальной пунктирной линии
    dashed_line_x = lv_line_create(chart);
    lv_obj_set_style_line_dash_width(dashed_line_x, GRAPH_DASH_LINE_WIDTH, 0);
    lv_obj_set_style_line_dash_gap(dashed_line_x, GRAPH_DASH_GAP, 0);
    lv_obj_set_style_line_color(dashed_line_x, lv_color_hex(GRAPH_DASH_LINE_COLOR_NORMAL), 0);
    lv_obj_set_style_line_width(dashed_line_x, 2, 0);
    lv_obj_add_flag(dashed_line_x, LV_OBJ_FLAG_HIDDEN);
    
    // Создание вертикальной пунктирной линии
    dashed_line_y = lv_line_create(chart);
    lv_obj_set_style_line_dash_width(dashed_line_y, GRAPH_DASH_LINE_WIDTH, 0);
    lv_obj_set_style_line_dash_gap(dashed_line_y, GRAPH_DASH_GAP, 0);
    lv_obj_set_style_line_color(dashed_line_y, lv_color_hex(GRAPH_DASH_LINE_COLOR_NORMAL), 0);
    lv_obj_set_style_line_width(dashed_line_y, 2, 0);
    lv_obj_add_flag(dashed_line_y, LV_OBJ_FLAG_HIDDEN);
    
    // Создание контейнера для заголовка
    title_container = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(title_container);
    lv_obj_set_size(title_container, 500, 30);
    lv_obj_align_to(title_container, chart, LV_ALIGN_OUT_TOP_MID, 0, -8);
    lv_obj_set_flex_flow(title_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(title_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Левая декоративная линия заголовка
    lv_obj_t *line_left = lv_line_create(title_container);
    static lv_point_precise_t line_points_left[] = { {100, 0}, {0, 0} };
    lv_line_set_points(line_left, line_points_left, 2);
    lv_obj_set_style_line_width(line_left, 2, 0);
    lv_obj_set_style_line_color(line_left, lv_color_hex(0x565B5D), 0);
    
    // Текст заголовка
    lv_obj_t *title = lv_label_create(title_container);
    lv_label_set_text(title, " ГРАФИК ОТОПЛЕНИЯ ");
    lv_obj_set_style_text_font(title, &Roboto_bold_24, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    
    // Правая декоративная линия заголовка
    lv_obj_t *line_right = lv_line_create(title_container);
    static lv_point_precise_t line_points_right[] = { {0, 0}, {100, 0} };
    lv_line_set_points(line_right, line_points_right, 2);
    lv_obj_set_style_line_width(line_right, 2, 0);
    lv_obj_set_style_line_color(line_right, lv_color_hex(0x565B5D), 0);
    
    // Создание осей графика
    create_chart_axes();
    
    // Устанавливаем количество точек графика в соответствии с C1_Number
    lv_chart_set_point_count(chart, C1_Number);
    
    // Добавление линии данных
    ser_points = lv_chart_add_series(chart, lv_color_hex(GRAPH_CHART_DATA_COLOR), LV_CHART_AXIS_SECONDARY_Y);
    lv_obj_set_style_line_width(chart, GRAPH_CHART_DATA_WIDTH, LV_PART_ITEMS);
    
    // Линия слева от первой точки (продолжение линии данных)
    ser_red_line_left = lv_chart_add_series(chart, lv_color_hex(GRAPH_CHART_DATA_COLOR), LV_CHART_AXIS_PRIMARY_Y);
    
    // Линия справа от последней точки (продолжение линии данных)
    ser_red_line_right = lv_chart_add_series(chart, lv_color_hex(GRAPH_CHART_DATA_COLOR), LV_CHART_AXIS_PRIMARY_Y);
    
    // Создание маркеров для всех точек
    for (int i = 0; i < 6; i++) {
        markers[i] = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(markers[i]);
        lv_obj_set_size(markers[i], 40, 50);
        
        // Добавление изображения маркера
        lv_obj_t *img = lv_img_create(markers[i]);
        lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
        
        // Добавление номера точки (черный цвет, всегда виден)
        lv_obj_t *label = lv_label_create(markers[i]);
        lv_label_set_text_fmt(label, "%d", i + 1);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_font(label, &Roboto_bold_18, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);  // Черный цвет для цифр
    }
    
    // Обновляем точки и маркеры
    update_chart_points();
    
    ESP_LOGI(TAG, "Heating graph display initialized");
}

/**
 * @brief Очищает график отопления
 */
void co_heating_graph_display_cleanup(void) {
    ESP_LOGI(TAG, "Cleaning up heating graph display");
    
    // Удаление маркеров
    for (int i = 0; i < 6; i++) {
        if (markers[i] && is_obj_valid(markers[i])) {
            lv_obj_del(markers[i]);
            markers[i] = NULL;
        }
    }
    
    // Удаление линий
    if (dashed_line_x && is_obj_valid(dashed_line_x)) {
        lv_obj_del(dashed_line_x);
        dashed_line_x = NULL;
    }
    if (dashed_line_y && is_obj_valid(dashed_line_y)) {
        lv_obj_del(dashed_line_y);
        dashed_line_y = NULL;
    }
    if (right_border_line && is_obj_valid(right_border_line)) {
        lv_obj_del(right_border_line);
        right_border_line = NULL;
    }
    
    // Удаление контейнера заголовка
    if (title_container && is_obj_valid(title_container)) {
        lv_obj_del(title_container);
        title_container = NULL;
    }
    
    // Удаление меток осей
    if (x_label && is_obj_valid(x_label)) {
        lv_obj_del(x_label);
        x_label = NULL;
    }
    if (y_label && is_obj_valid(y_label)) {
        lv_obj_del(y_label);
        y_label = NULL;
    }
    
    // Удаление меток делений осей
    // Используем максимальное значение из двух осей для безопасного удаления всех возможных меток
    int max_tick_count = (GRAPH_AXIS_TICK_COUNT_X > GRAPH_AXIS_TICK_COUNT_Y) ? GRAPH_AXIS_TICK_COUNT_X : GRAPH_AXIS_TICK_COUNT_Y;
    for (int i = 0; i < max_tick_count && i < 6; i++) {
        if (x_tick_labels[i] && is_obj_valid(x_tick_labels[i])) {
            lv_obj_del(x_tick_labels[i]);
            x_tick_labels[i] = NULL;
        }
        if (y_tick_labels[i] && is_obj_valid(y_tick_labels[i])) {
            lv_obj_del(y_tick_labels[i]);
            y_tick_labels[i] = NULL;
        }
    }
    
    // Удаление графика (должно быть последним, так как линии являются дочерними и удалятся автоматически)
    // Серии данных также удалятся автоматически вместе с графиком
    if (chart && is_obj_valid(chart)) {
        lv_obj_del(chart);
    }
    
    // Сброс всех указателей после удаления графика
    chart = NULL;
    ser_points = NULL;
    ser_red_line_left = NULL;
    ser_red_line_right = NULL;
    right_border_line = NULL;  // Эта линия создается как дочерний элемент chart, удалится автоматически
    
    current_selected_param_index = -1;
    current_is_editing = false;
    current_editing_param_index = -1;
}

/**
 * @brief Показывает график отопления
 */
void co_heating_graph_display_show(void) {
    if (chart && is_obj_valid(chart)) {
        lv_obj_clear_flag(chart, LV_OBJ_FLAG_HIDDEN);
    }
    if (title_container && is_obj_valid(title_container)) {
        lv_obj_clear_flag(title_container, LV_OBJ_FLAG_HIDDEN);
    }
    for (int i = 0; i < 6; i++) {
        if (markers[i] && is_obj_valid(markers[i])) {
            if (i < C1_Number) {
                lv_obj_clear_flag(markers[i], LV_OBJ_FLAG_HIDDEN);
                // Убеждаемся, что label с цифрой тоже не скрыт
                lv_obj_t *label = lv_obj_get_child(markers[i], 1);
                if (label) {
                    lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
                }
            }
        }
    }
}

/**
 * @brief Скрывает график отопления
 */
void co_heating_graph_display_hide(void) {
    if (chart && is_obj_valid(chart)) {
        lv_obj_add_flag(chart, LV_OBJ_FLAG_HIDDEN);
    }
    if (title_container && is_obj_valid(title_container)) {
        lv_obj_add_flag(title_container, LV_OBJ_FLAG_HIDDEN);
    }
    for (int i = 0; i < 6; i++) {
        if (markers[i] && is_obj_valid(markers[i])) {
            lv_obj_add_flag(markers[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (dashed_line_x && is_obj_valid(dashed_line_x)) {
        lv_obj_add_flag(dashed_line_x, LV_OBJ_FLAG_HIDDEN);
    }
    if (dashed_line_y && is_obj_valid(dashed_line_y)) {
        lv_obj_add_flag(dashed_line_y, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief Обновляет отображение в зависимости от курсора и режима редактирования
 */
void co_heating_graph_display_update_cursor(int cursor_index, bool is_editing, int editing_param_index) {
    // Определяем param_index по cursor_index
    // cursor_index 0 = "Назад" (param_index -1)
    // cursor_index 1 = "Способ задания" (param_index 0)
    // cursor_index 2 = "Угол наклона" (param_index 1)
    // cursor_index 3 = "Количество точек" (param_index 2)
    // cursor_index 4 = "Точка 1. Тнв" (param_index 3)
    // cursor_index 5 = "Точка 1. Тпод_CO" (param_index 4)
    // и т.д.
    
    static const int cursor_to_param_map[] = {
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
    };
    
    int param_index = -1;
    if (cursor_index >= 0 && cursor_index < sizeof(cursor_to_param_map) / sizeof(cursor_to_param_map[0])) {
        param_index = cursor_to_param_map[cursor_index];
    }
    
    // Обновляем состояние только если это параметр точки (3-14)
    if (param_index >= 3 && param_index <= 14) {
        current_selected_param_index = param_index;
    } else {
        current_selected_param_index = -1;
    }
    
    current_is_editing = is_editing;
    current_editing_param_index = editing_param_index;
    
    // Обновляем отображение
    // Важно: сначала обновляем координаты точек, потом маркеры и линии
    update_chart_points();
    update_markers();
}

/**
 * @brief Обновляет позиции точек на графике
 */
void co_heating_graph_display_update_points(void) {
    update_chart_points();
}


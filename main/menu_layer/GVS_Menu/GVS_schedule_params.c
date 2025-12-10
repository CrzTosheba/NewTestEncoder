#include "GVS_schedule_params.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "GVS_SCHEDULE_PARAMS";

// Глобальные переменные для параметров расписания ГВС
// Понедельник
int GVS_MonHoursFrom1 = 8;   // По умолчанию 8:00
int GVS_MonHoursTo1 = 12;   // По умолчанию 12:00
int GVS_MonMinFrom1 = 0;
int GVS_MonMinTo1 = 0;
int GVS_MonHoursFrom2 = 18;  // По умолчанию 18:00
int GVS_MonHoursTo2 = 22;    // По умолчанию 22:00
int GVS_MonMinFrom2 = 0;
int GVS_MonMinTo2 = 0;

// Вторник
int GVS_TueHoursFrom1 = 8;
int GVS_TueHoursTo1 = 12;
int GVS_TueMinFrom1 = 0;
int GVS_TueMinTo1 = 0;
int GVS_TueHoursFrom2 = 18;
int GVS_TueHoursTo2 = 22;
int GVS_TueMinFrom2 = 0;
int GVS_TueMinTo2 = 0;

// Среда
int GVS_WedHoursFrom1 = 8;
int GVS_WedHoursTo1 = 12;
int GVS_WedMinFrom1 = 0;
int GVS_WedMinTo1 = 0;
int GVS_WedHoursFrom2 = 18;
int GVS_WedHoursTo2 = 22;
int GVS_WedMinFrom2 = 0;
int GVS_WedMinTo2 = 0;

// Четверг
int GVS_ThuHoursFrom1 = 8;
int GVS_ThuHoursTo1 = 12;
int GVS_ThuMinFrom1 = 0;
int GVS_ThuMinTo1 = 0;
int GVS_ThuHoursFrom2 = 18;
int GVS_ThuHoursTo2 = 22;
int GVS_ThuMinFrom2 = 0;
int GVS_ThuMinTo2 = 0;

// Пятница
int GVS_FriHoursFrom1 = 8;
int GVS_FriHoursTo1 = 12;
int GVS_FriMinFrom1 = 0;
int GVS_FriMinTo1 = 0;
int GVS_FriHoursFrom2 = 18;
int GVS_FriHoursTo2 = 22;
int GVS_FriMinFrom2 = 0;
int GVS_FriMinTo2 = 0;

// Суббота
int GVS_SatHoursFrom1 = 8;
int GVS_SatHoursTo1 = 12;
int GVS_SatMinFrom1 = 0;
int GVS_SatMinTo1 = 0;
int GVS_SatHoursFrom2 = 18;
int GVS_SatHoursTo2 = 22;
int GVS_SatMinFrom2 = 0;
int GVS_SatMinTo2 = 0;

// Воскресенье
int GVS_SunHoursFrom1 = 8;
int GVS_SunHoursTo1 = 12;
int GVS_SunMinFrom1 = 0;
int GVS_SunMinTo1 = 0;
int GVS_SunHoursFrom2 = 18;
int GVS_SunHoursTo2 = 22;
int GVS_SunMinFrom2 = 0;
int GVS_SunMinTo2 = 0;

// Функция инициализации параметров
void gvs_schedule_params_init(void) {
    // НЕ загружаем параметры из NVS (как в ГВС)
    // Используем значения по умолчанию
    ESP_LOGI(TAG, "GVS schedule parameters initialized with default values");
}


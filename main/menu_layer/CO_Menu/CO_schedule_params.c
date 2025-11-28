#include "CO_schedule_params.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <stddef.h>

static const char *TAG = "CO_SCHEDULE_PARAMS";
static const char *NVS_NAMESPACE = "co_schedule";  // Максимум 15 символов для NVS

// Глобальные переменные для параметров расписания
// Понедельник
int MonHoursFrom1 = 8;   // По умолчанию 8:00
int MonHoursTo1 = 12;   // По умолчанию 12:00
int MonMinFrom1 = 0;
int MonMinTo1 = 0;
int MonHoursFrom2 = 18;  // По умолчанию 18:00
int MonHoursTo2 = 22;    // По умолчанию 22:00
int MonMinFrom2 = 0;
int MonMinTo2 = 0;

// Вторник
int TueHoursFrom1 = 8;
int TueHoursTo1 = 12;
int TueMinFrom1 = 0;
int TueMinTo1 = 0;
int TueHoursFrom2 = 18;
int TueHoursTo2 = 22;
int TueMinFrom2 = 0;
int TueMinTo2 = 0;

// Среда
int WedHoursFrom1 = 8;
int WedHoursTo1 = 12;
int WedMinFrom1 = 0;
int WedMinTo1 = 0;
int WedHoursFrom2 = 18;
int WedHoursTo2 = 22;
int WedMinFrom2 = 0;
int WedMinTo2 = 0;

// Четверг
int ThuHoursFrom1 = 8;
int ThuHoursTo1 = 12;
int ThuMinFrom1 = 0;
int ThuMinTo1 = 0;
int ThuHoursFrom2 = 18;
int ThuHoursTo2 = 22;
int ThuMinFrom2 = 0;
int ThuMinTo2 = 0;

// Пятница
int FriHoursFrom1 = 8;
int FriHoursTo1 = 12;
int FriMinFrom1 = 0;
int FriMinTo1 = 0;
int FriHoursFrom2 = 18;
int FriHoursTo2 = 22;
int FriMinFrom2 = 0;
int FriMinTo2 = 0;

// Суббота
int SatHoursFrom1 = 8;
int SatHoursTo1 = 12;
int SatMinFrom1 = 0;
int SatMinTo1 = 0;
int SatHoursFrom2 = 18;
int SatHoursTo2 = 22;
int SatMinFrom2 = 0;
int SatMinTo2 = 0;

// Воскресенье
int SunHoursFrom1 = 8;
int SunHoursTo1 = 12;
int SunMinFrom1 = 0;
int SunMinTo1 = 0;
int SunHoursFrom2 = 18;
int SunHoursTo2 = 22;
int SunMinFrom2 = 0;
int SunMinTo2 = 0;

/**
 * @brief Сохраняет параметры в NVS
 */
void co_schedule_params_save(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to save parameters to NVS namespace: %s", NVS_NAMESPACE);
    
    // Открываем NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return;
    }
    
    // Сохраняем параметры понедельника
    err = nvs_set_i32(nvs_handle, "MonHFrom1", MonHoursFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MonHFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "MonHTo1", MonHoursTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MonHTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "MonMFrom1", MonMinFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MonMFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "MonMTo1", MonMinTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MonMTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "MonHFrom2", MonHoursFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MonHFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "MonHTo2", MonHoursTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MonHTo2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "MonMFrom2", MonMinFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MonMFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "MonMTo2", MonMinTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving MonMTo2: %s", esp_err_to_name(err));
    
    // Сохраняем параметры вторника
    err = nvs_set_i32(nvs_handle, "TueHFrom1", TueHoursFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving TueHFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "TueHTo1", TueHoursTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving TueHTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "TueMFrom1", TueMinFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving TueMFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "TueMTo1", TueMinTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving TueMTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "TueHFrom2", TueHoursFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving TueHFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "TueHTo2", TueHoursTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving TueHTo2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "TueMFrom2", TueMinFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving TueMFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "TueMTo2", TueMinTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving TueMTo2: %s", esp_err_to_name(err));
    
    // Сохраняем параметры среды
    err = nvs_set_i32(nvs_handle, "WedHFrom1", WedHoursFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving WedHFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "WedHTo1", WedHoursTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving WedHTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "WedMFrom1", WedMinFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving WedMFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "WedMTo1", WedMinTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving WedMTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "WedHFrom2", WedHoursFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving WedHFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "WedHTo2", WedHoursTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving WedHTo2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "WedMFrom2", WedMinFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving WedMFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "WedMTo2", WedMinTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving WedMTo2: %s", esp_err_to_name(err));
    
    // Сохраняем параметры четверга
    err = nvs_set_i32(nvs_handle, "ThuHFrom1", ThuHoursFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving ThuHFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "ThuHTo1", ThuHoursTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving ThuHTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "ThuMFrom1", ThuMinFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving ThuMFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "ThuMTo1", ThuMinTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving ThuMTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "ThuHFrom2", ThuHoursFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving ThuHFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "ThuHTo2", ThuHoursTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving ThuHTo2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "ThuMFrom2", ThuMinFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving ThuMFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "ThuMTo2", ThuMinTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving ThuMTo2: %s", esp_err_to_name(err));
    
    // Сохраняем параметры пятницы
    err = nvs_set_i32(nvs_handle, "FriHFrom1", FriHoursFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving FriHFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "FriHTo1", FriHoursTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving FriHTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "FriMFrom1", FriMinFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving FriMFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "FriMTo1", FriMinTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving FriMTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "FriHFrom2", FriHoursFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving FriHFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "FriHTo2", FriHoursTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving FriHTo2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "FriMFrom2", FriMinFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving FriMFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "FriMTo2", FriMinTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving FriMTo2: %s", esp_err_to_name(err));
    
    // Сохраняем параметры субботы
    err = nvs_set_i32(nvs_handle, "SatHFrom1", SatHoursFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SatHFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SatHTo1", SatHoursTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SatHTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SatMFrom1", SatMinFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SatMFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SatMTo1", SatMinTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SatMTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SatHFrom2", SatHoursFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SatHFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SatHTo2", SatHoursTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SatHTo2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SatMFrom2", SatMinFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SatMFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SatMTo2", SatMinTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SatMTo2: %s", esp_err_to_name(err));
    
    // Сохраняем параметры воскресенья
    err = nvs_set_i32(nvs_handle, "SunHFrom1", SunHoursFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SunHFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SunHTo1", SunHoursTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SunHTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SunMFrom1", SunMinFrom1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SunMFrom1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SunMTo1", SunMinTo1);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SunMTo1: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SunHFrom2", SunHoursFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SunHFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SunHTo2", SunHoursTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SunHTo2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SunMFrom2", SunMinFrom2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SunMFrom2: %s", esp_err_to_name(err));
    err = nvs_set_i32(nvs_handle, "SunMTo2", SunMinTo2);
    if (err != ESP_OK) ESP_LOGE(TAG, "Error saving SunMTo2: %s", esp_err_to_name(err));
    
    // Коммитим изменения
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error committing NVS: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Parameters saved to NVS successfully");
    }
    
    nvs_close(nvs_handle);
}

/**
 * @brief Загружает параметры из NVS
 */
void co_schedule_params_load(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    ESP_LOGI(TAG, "Attempting to load parameters from NVS namespace: %s", NVS_NAMESPACE);
    
    // Открываем NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Error opening NVS handle (namespace may not exist): %s", esp_err_to_name(err));
        return;
    }
    
    // Загружаем параметры понедельника
    int32_t int_val;
    err = nvs_get_i32(nvs_handle, "MonHFrom1", &int_val);
    if (err == ESP_OK) MonHoursFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "MonHTo1", &int_val);
    if (err == ESP_OK) MonHoursTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "MonMFrom1", &int_val);
    if (err == ESP_OK) MonMinFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "MonMTo1", &int_val);
    if (err == ESP_OK) MonMinTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "MonHFrom2", &int_val);
    if (err == ESP_OK) MonHoursFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "MonHTo2", &int_val);
    if (err == ESP_OK) MonHoursTo2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "MonMFrom2", &int_val);
    if (err == ESP_OK) MonMinFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "MonMTo2", &int_val);
    if (err == ESP_OK) MonMinTo2 = (int)int_val;
    
    // Загружаем параметры вторника
    err = nvs_get_i32(nvs_handle, "TueHFrom1", &int_val);
    if (err == ESP_OK) TueHoursFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "TueHTo1", &int_val);
    if (err == ESP_OK) TueHoursTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "TueMFrom1", &int_val);
    if (err == ESP_OK) TueMinFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "TueMTo1", &int_val);
    if (err == ESP_OK) TueMinTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "TueHFrom2", &int_val);
    if (err == ESP_OK) TueHoursFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "TueHTo2", &int_val);
    if (err == ESP_OK) TueHoursTo2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "TueMFrom2", &int_val);
    if (err == ESP_OK) TueMinFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "TueMTo2", &int_val);
    if (err == ESP_OK) TueMinTo2 = (int)int_val;
    
    // Загружаем параметры среды
    err = nvs_get_i32(nvs_handle, "WedHFrom1", &int_val);
    if (err == ESP_OK) WedHoursFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "WedHTo1", &int_val);
    if (err == ESP_OK) WedHoursTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "WedMFrom1", &int_val);
    if (err == ESP_OK) WedMinFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "WedMTo1", &int_val);
    if (err == ESP_OK) WedMinTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "WedHFrom2", &int_val);
    if (err == ESP_OK) WedHoursFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "WedHTo2", &int_val);
    if (err == ESP_OK) WedHoursTo2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "WedMFrom2", &int_val);
    if (err == ESP_OK) WedMinFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "WedMTo2", &int_val);
    if (err == ESP_OK) WedMinTo2 = (int)int_val;
    
    // Загружаем параметры четверга
    err = nvs_get_i32(nvs_handle, "ThuHFrom1", &int_val);
    if (err == ESP_OK) ThuHoursFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "ThuHTo1", &int_val);
    if (err == ESP_OK) ThuHoursTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "ThuMFrom1", &int_val);
    if (err == ESP_OK) ThuMinFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "ThuMTo1", &int_val);
    if (err == ESP_OK) ThuMinTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "ThuHFrom2", &int_val);
    if (err == ESP_OK) ThuHoursFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "ThuHTo2", &int_val);
    if (err == ESP_OK) ThuHoursTo2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "ThuMFrom2", &int_val);
    if (err == ESP_OK) ThuMinFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "ThuMTo2", &int_val);
    if (err == ESP_OK) ThuMinTo2 = (int)int_val;
    
    // Загружаем параметры пятницы
    err = nvs_get_i32(nvs_handle, "FriHFrom1", &int_val);
    if (err == ESP_OK) FriHoursFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "FriHTo1", &int_val);
    if (err == ESP_OK) FriHoursTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "FriMFrom1", &int_val);
    if (err == ESP_OK) FriMinFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "FriMTo1", &int_val);
    if (err == ESP_OK) FriMinTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "FriHFrom2", &int_val);
    if (err == ESP_OK) FriHoursFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "FriHTo2", &int_val);
    if (err == ESP_OK) FriHoursTo2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "FriMFrom2", &int_val);
    if (err == ESP_OK) FriMinFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "FriMTo2", &int_val);
    if (err == ESP_OK) FriMinTo2 = (int)int_val;
    
    // Загружаем параметры субботы
    err = nvs_get_i32(nvs_handle, "SatHFrom1", &int_val);
    if (err == ESP_OK) SatHoursFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SatHTo1", &int_val);
    if (err == ESP_OK) SatHoursTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SatMFrom1", &int_val);
    if (err == ESP_OK) SatMinFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SatMTo1", &int_val);
    if (err == ESP_OK) SatMinTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SatHFrom2", &int_val);
    if (err == ESP_OK) SatHoursFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SatHTo2", &int_val);
    if (err == ESP_OK) SatHoursTo2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SatMFrom2", &int_val);
    if (err == ESP_OK) SatMinFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SatMTo2", &int_val);
    if (err == ESP_OK) SatMinTo2 = (int)int_val;
    
    // Загружаем параметры воскресенья
    err = nvs_get_i32(nvs_handle, "SunHFrom1", &int_val);
    if (err == ESP_OK) SunHoursFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SunHTo1", &int_val);
    if (err == ESP_OK) SunHoursTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SunMFrom1", &int_val);
    if (err == ESP_OK) SunMinFrom1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SunMTo1", &int_val);
    if (err == ESP_OK) SunMinTo1 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SunHFrom2", &int_val);
    if (err == ESP_OK) SunHoursFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SunHTo2", &int_val);
    if (err == ESP_OK) SunHoursTo2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SunMFrom2", &int_val);
    if (err == ESP_OK) SunMinFrom2 = (int)int_val;
    err = nvs_get_i32(nvs_handle, "SunMTo2", &int_val);
    if (err == ESP_OK) SunMinTo2 = (int)int_val;
    
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Parameters loaded from NVS successfully");
}

/**
 * @brief Инициализирует параметры расписания (загружает из NVS или использует значения по умолчанию)
 */
void co_schedule_params_init(void) {
    ESP_LOGI(TAG, "Initializing CO schedule parameters");
    co_schedule_params_load();
}


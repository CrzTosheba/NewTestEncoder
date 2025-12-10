#ifndef GVS_SCHEDULE_PARAMS_H
#define GVS_SCHEDULE_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные для параметров расписания ГВС
// Понедельник
extern int GVS_MonHoursFrom1;
extern int GVS_MonHoursTo1;
extern int GVS_MonMinFrom1;
extern int GVS_MonMinTo1;
extern int GVS_MonHoursFrom2;
extern int GVS_MonHoursTo2;
extern int GVS_MonMinFrom2;
extern int GVS_MonMinTo2;

// Вторник
extern int GVS_TueHoursFrom1;
extern int GVS_TueHoursTo1;
extern int GVS_TueMinFrom1;
extern int GVS_TueMinTo1;
extern int GVS_TueHoursFrom2;
extern int GVS_TueHoursTo2;
extern int GVS_TueMinFrom2;
extern int GVS_TueMinTo2;

// Среда
extern int GVS_WedHoursFrom1;
extern int GVS_WedHoursTo1;
extern int GVS_WedMinFrom1;
extern int GVS_WedMinTo1;
extern int GVS_WedHoursFrom2;
extern int GVS_WedHoursTo2;
extern int GVS_WedMinFrom2;
extern int GVS_WedMinTo2;

// Четверг
extern int GVS_ThuHoursFrom1;
extern int GVS_ThuHoursTo1;
extern int GVS_ThuMinFrom1;
extern int GVS_ThuMinTo1;
extern int GVS_ThuHoursFrom2;
extern int GVS_ThuHoursTo2;
extern int GVS_ThuMinFrom2;
extern int GVS_ThuMinTo2;

// Пятница
extern int GVS_FriHoursFrom1;
extern int GVS_FriHoursTo1;
extern int GVS_FriMinFrom1;
extern int GVS_FriMinTo1;
extern int GVS_FriHoursFrom2;
extern int GVS_FriHoursTo2;
extern int GVS_FriMinFrom2;
extern int GVS_FriMinTo2;

// Суббота
extern int GVS_SatHoursFrom1;
extern int GVS_SatHoursTo1;
extern int GVS_SatMinFrom1;
extern int GVS_SatMinTo1;
extern int GVS_SatHoursFrom2;
extern int GVS_SatHoursTo2;
extern int GVS_SatMinFrom2;
extern int GVS_SatMinTo2;

// Воскресенье
extern int GVS_SunHoursFrom1;
extern int GVS_SunHoursTo1;
extern int GVS_SunMinFrom1;
extern int GVS_SunMinTo1;
extern int GVS_SunHoursFrom2;
extern int GVS_SunHoursTo2;
extern int GVS_SunMinFrom2;
extern int GVS_SunMinTo2;

// Функции для работы с параметрами (без сохранения в NVS)
void gvs_schedule_params_init(void);

#ifdef __cplusplus
}
#endif

#endif // GVS_SCHEDULE_PARAMS_H


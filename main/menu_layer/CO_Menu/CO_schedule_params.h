#ifndef CO_SCHEDULE_PARAMS_H
#define CO_SCHEDULE_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные для параметров расписания
// Понедельник
extern int MonHoursFrom1;
extern int MonHoursTo1;
extern int MonMinFrom1;
extern int MonMinTo1;
extern int MonHoursFrom2;
extern int MonHoursTo2;
extern int MonMinFrom2;
extern int MonMinTo2;

// Вторник
extern int TueHoursFrom1;
extern int TueHoursTo1;
extern int TueMinFrom1;
extern int TueMinTo1;
extern int TueHoursFrom2;
extern int TueHoursTo2;
extern int TueMinFrom2;
extern int TueMinTo2;

// Среда
extern int WedHoursFrom1;
extern int WedHoursTo1;
extern int WedMinFrom1;
extern int WedMinTo1;
extern int WedHoursFrom2;
extern int WedHoursTo2;
extern int WedMinFrom2;
extern int WedMinTo2;

// Четверг
extern int ThuHoursFrom1;
extern int ThuHoursTo1;
extern int ThuMinFrom1;
extern int ThuMinTo1;
extern int ThuHoursFrom2;
extern int ThuHoursTo2;
extern int ThuMinFrom2;
extern int ThuMinTo2;

// Пятница
extern int FriHoursFrom1;
extern int FriHoursTo1;
extern int FriMinFrom1;
extern int FriMinTo1;
extern int FriHoursFrom2;
extern int FriHoursTo2;
extern int FriMinFrom2;
extern int FriMinTo2;

// Суббота
extern int SatHoursFrom1;
extern int SatHoursTo1;
extern int SatMinFrom1;
extern int SatMinTo1;
extern int SatHoursFrom2;
extern int SatHoursTo2;
extern int SatMinFrom2;
extern int SatMinTo2;

// Воскресенье (для будущего использования)
extern int SunHoursFrom1;
extern int SunHoursTo1;
extern int SunMinFrom1;
extern int SunMinTo1;
extern int SunHoursFrom2;
extern int SunHoursTo2;
extern int SunMinFrom2;
extern int SunMinTo2;

// Функции для работы с параметрами
void co_schedule_params_init(void);
void co_schedule_params_save(void);
void co_schedule_params_load(void);

#ifdef __cplusplus
}
#endif

#endif // CO_SCHEDULE_PARAMS_H


#ifndef ACCESS_CONTROL_H
#define ACCESS_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Конфигурация доступа (можно менять в коде)
#define ACCESS_PASSWORD 000          // Пароль для доступа
#define ACCESS_TIMEOUT_MINUTES 1     // Таймаут неактивности в минутах

// Функции управления доступом
bool access_control_is_unlocked(void);
void access_control_unlock(void);
void access_control_lock(void);

// Функции для работы с таймером активности
void access_control_reset_activity_timer(void);
void access_control_update_activity_timer(void);  // Вызывать при активности пользователя

// Инициализация модуля
void access_control_init(void);

#ifdef __cplusplus
}
#endif

#endif // ACCESS_CONTROL_H


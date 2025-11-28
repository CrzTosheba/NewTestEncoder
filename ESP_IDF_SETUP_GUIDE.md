# Руководство по установке ESP-IDF на Windows

## Способ 1: ESP-IDF Tools Installer (РЕКОМЕНДУЕТСЯ)

### Шаг 1: Скачать установщик
1. Перейдите на страницу: https://dl.espressif.com/dl/esp-idf/
2. Скачайте **ESP-IDF Tools Installer** для Windows (например, `esp-idf-tools-setup-online.exe`)
3. Выберите версию ESP-IDF (рекомендуется v5.1 или v5.2)

### Шаг 2: Установка
1. Запустите установщик от имени администратора
2. Выберите:
   - Версию ESP-IDF (v5.1 или v5.2)
   - Python (установщик скачает нужную версию)
   - Git (если не установлен)
   - Путь установки (по умолчанию: `C:\Espressif`)
3. Дождитесь завершения установки

### Шаг 3: Активация окружения
После установки у вас появится ярлык **"ESP-IDF Command Prompt"** или **"ESP-IDF PowerShell"**.

**ВАЖНО:** Всегда используйте этот терминал для работы с ESP-IDF!

Или активируйте вручную в обычном PowerShell:
```powershell
# Путь может отличаться, проверьте в установщике
C:\Espressif\frameworks\esp-idf-v5.1\export.ps1
```

---

## Способ 2: Ручная установка

### Требования:
- Python 3.8 или выше
- Git
- CMake 3.16 или выше

### Шаги:

1. **Установите Python** (если нет):
   - Скачайте с https://www.python.org/downloads/
   - При установке отметьте "Add Python to PATH"

2. **Установите Git** (если нет):
   - Скачайте с https://git-scm.com/download/win

3. **Клонируйте ESP-IDF**:
   ```powershell
   cd C:\
   git clone --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   git checkout v5.1  # или v5.2
   git submodule update --init --recursive
   ```

4. **Установите инструменты ESP-IDF**:
   ```powershell
   .\install.bat esp32s3
   ```

5. **Активируйте окружение**:
   ```powershell
   .\export.ps1
   ```

---

## Проверка установки

После активации окружения выполните:
```powershell
idf.py --version
```

Должна отобразиться версия ESP-IDF.

---

## Использование в вашем проекте

1. **Откройте ESP-IDF Command Prompt** (или активируйте окружение)

2. **Перейдите в директорию проекта**:
   ```powershell
   cd C:\Users\fugu\OneDrive\Desktop\esp32s3_lcd_i80_ssd1963
   ```

3. **Настройте проект** (если нужно):
   ```powershell
   idf.py menuconfig
   ```

4. **Соберите проект**:
   ```powershell
   idf.py build
   ```

5. **Прошейте на устройство**:
   ```powershell
   idf.py -p COM3 flash  # замените COM3 на ваш порт
   ```

6. **Мониторинг**:
   ```powershell
   idf.py -p COM3 monitor
   ```

---

## Постоянная активация в PowerShell (опционально)

Если хотите, чтобы ESP-IDF был доступен всегда, добавьте в профиль PowerShell:

```powershell
# Откройте профиль
notepad $PROFILE

# Добавьте строку (измените путь на ваш):
C:\Espressif\frameworks\esp-idf-v5.1\export.ps1
```

---

## Решение проблем

### Проблема: "idf.py не найден"
**Решение:** Убедитесь, что вы используете ESP-IDF Command Prompt или активировали окружение через `export.ps1`

### Проблема: "Python не найден"
**Решение:** Установите Python и добавьте его в PATH, или используйте ESP-IDF Tools Installer

### Проблема: Ошибки при сборке
**Решение:** 
- Проверьте версию ESP-IDF (должна быть v5.0+)
- Убедитесь, что все подмодули установлены: `git submodule update --init --recursive`

---

## Полезные ссылки

- Официальная документация: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/
- Форум: https://esp32.com/
- GitHub: https://github.com/espressif/esp-idf


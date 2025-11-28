# Скрипт для загрузки прошивки и мониторинга ESP32-S3
# Использование: .\flash_and_monitor.ps1 [COM_PORT]

param(
    [string]$Port = "COM3"  # Порт по умолчанию из настроек VS Code
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ESP32-S3 Flash and Monitor Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Проверяем наличие ESP-IDF окружения
if (-not $env:IDF_PATH) {
    Write-Host "ОШИБКА: ESP-IDF окружение не активировано!" -ForegroundColor Red
    Write-Host "Активируйте ESP-IDF через:" -ForegroundColor Yellow
    Write-Host "  C:\Users\fugu\esp\v5.5.1\esp-idf\export.ps1" -ForegroundColor Yellow
    exit 1
}

Write-Host "ESP-IDF Path: $env:IDF_PATH" -ForegroundColor Green
Write-Host "Target Port: $Port" -ForegroundColor Green
Write-Host ""

# Шаг 1: Сборка проекта
Write-Host "[1/3] Сборка проекта..." -ForegroundColor Yellow
idf.py build
if ($LASTEXITCODE -ne 0) {
    Write-Host "ОШИБКА: Сборка не удалась!" -ForegroundColor Red
    exit 1
}
Write-Host "Сборка завершена успешно!" -ForegroundColor Green
Write-Host ""

# Шаг 2: Загрузка прошивки
Write-Host "[2/3] Загрузка прошивки на $Port..." -ForegroundColor Yellow
idf.py -p $Port flash
if ($LASTEXITCODE -ne 0) {
    Write-Host "ОШИБКА: Загрузка не удалась!" -ForegroundColor Red
    Write-Host "Проверьте:" -ForegroundColor Yellow
    Write-Host "  - Подключен ли контроллер" -ForegroundColor Yellow
    Write-Host "  - Правильный ли COM порт ($Port)" -ForegroundColor Yellow
    Write-Host "  - Нажат ли кнопка BOOT при загрузке (если требуется)" -ForegroundColor Yellow
    exit 1
}
Write-Host "Загрузка завершена успешно!" -ForegroundColor Green
Write-Host ""

# Шаг 3: Мониторинг
Write-Host "[3/3] Запуск мониторинга логов..." -ForegroundColor Yellow
Write-Host "Для выхода из мониторинга нажмите Ctrl+]" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

idf.py -p $Port monitor


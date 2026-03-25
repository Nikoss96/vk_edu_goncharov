# Скрипт для быстрой сборки и запуска проекта

# Создание директории для сборки
if (-Not (Test-Path -Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
    Write-Host "Создана директория build/" -ForegroundColor Green
}

# Переход в директорию build
Set-Location build

Write-Host "`n=== Конфигурация CMake ===" -ForegroundColor Cyan
cmake ..

if ($LASTEXITCODE -ne 0) {
    Write-Host "Ошибка конфигурации CMake!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Write-Host "`n=== Сборка проекта ===" -ForegroundColor Cyan
cmake --build . --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "Ошибка сборки!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Write-Host "`n=== Запуск тестов ===" -ForegroundColor Cyan
.\Release\tests.exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "Тесты не прошли!" -ForegroundColor Red
} else {
    Write-Host "`nВсе тесты пройдены успешно!" -ForegroundColor Green
}

Write-Host "`n=== Запуск бенчмарков ===" -ForegroundColor Cyan
Write-Host "Для запуска бенчмарков выполните: .\Release\benchmarks.exe" -ForegroundColor Yellow

Set-Location ..

Write-Host "`nГотово! Проект собран и протестирован." -ForegroundColor Green
Write-Host "Для запуска бенчмарков: cd build; .\Release\benchmarks.exe" -ForegroundColor Yellow

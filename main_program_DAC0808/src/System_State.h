#pragma once

#include <Arduino.h>

namespace SystemState {
    // Типы команд для проверки
    enum class CommandType {
        ALWAYS_ALLOWED,     // Всегда разрешены (stop, status)
        ALLOWED_WHEN_BUSY,  // Разрешены даже когда занято
        BLOCKED_WHEN_BUSY   // Блокируются когда занято
    };
    
    void occupy(const char* deviceName); // Функция для занятия системы устройством
    void free(); // Функция для освобождение системы от устройства
    bool isSystemBusy(); // Опрос занятости системы
    String getActiveDevice(); // Сообщает название устройства, занимающего систему
    bool canAccessDevice(const char* deviceName); // Запрос на доступ к устройству
    
    // Новая функция: проверка команды с учетом типа
    bool canExecuteCommand(const char* deviceName, const char* command, CommandType type);
}
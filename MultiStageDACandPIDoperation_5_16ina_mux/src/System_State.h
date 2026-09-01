#pragma once

#include <Arduino.h>

namespace SystemState {
    // Типы команд для проверки
    enum class CommandType {
        ALWAYS_ALLOWED,     // Всегда разрешены (stop, status)
        ALLOWED_WHEN_BUSY,  // Разрешены даже когда занято
        BLOCKED_WHEN_BUSY   // Блокируются когда занято
    };
    
    void occupy(const char* deviceName);
    void free();
    bool isSystemBusy();
    String getActiveDevice();
    bool canAccessDevice(const char* deviceName);
    
    // Новая функция: проверка команды с учетом типа
    bool canExecuteCommand(const char* deviceName, const char* command, CommandType type);
}
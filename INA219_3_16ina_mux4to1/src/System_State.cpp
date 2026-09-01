#include "System_State.h"

namespace SystemState {
    bool isBusy = false;
    String activeDevice = "";
    
    void occupy(const char* deviceName) {
        isBusy = true;
        activeDevice = deviceName;
        Serial.print("Система занята устройством: ");
        Serial.println(deviceName);
    }
    
    void free() {
        isBusy = false;
        activeDevice = "";
        Serial.println("Система свободна");
    }
    
    bool isSystemBusy() {
        return isBusy;
    }
    
    String getActiveDevice() {
        return activeDevice;
    }
    
    bool canAccessDevice(const char* deviceName) {
        if (!isBusy) return true;
        if (activeDevice == deviceName) return true;
        
        Serial.print("Ошибка: Система занята устройством '");
        Serial.print(activeDevice);
        Serial.println("'!");
        Serial.print("Сначала остановите '");
        Serial.print(activeDevice);
        Serial.print("' командой '");
        Serial.print(activeDevice);
        Serial.println(":stop;'");
        return false;
    }
    
    bool canExecuteCommand(const char* deviceName, const char* command, CommandType type) {
        // Если команда всегда разрешена
        if (type == CommandType::ALWAYS_ALLOWED) {
            return true;
        }
        
        // Если система свободна - все команды разрешены
        if (!isBusy) {
            return true;
        }
        
        // Система занята
        // Проверяем, что запрос от того же устройства
        if (activeDevice != deviceName) {
            Serial.print("Ошибка: Система занята устройством '");
            Serial.print(activeDevice);
            Serial.println("'!");
            return false;
        }
        
        // Если команда разрешена даже когда занято
        if (type == CommandType::ALLOWED_WHEN_BUSY) {
            return true;
        }
        
        // Команда заблокирована
        Serial.print("Ошибка: Команда '");
        Serial.print(command);
        Serial.println("' недоступна во время измерений!");
        Serial.println("Доступны команды: stop, ...");
        return false;
    }
}
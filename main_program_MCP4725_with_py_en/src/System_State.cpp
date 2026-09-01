#include "System_State.h"

namespace SystemState {
    bool isBusy = false;
    String activeDevice = "";
    
    // Функция для занятия системы устройством
    void occupy(const char* deviceName) {
        isBusy = true;
        activeDevice = deviceName;
        //Serial.print("Система занята устройством: ");
        //Serial.println(deviceName);
    }
    // Функция для освобождение системы от устройства
    void free() {
        isBusy = false;
        activeDevice = "";
        //Serial.println("Система свободна");
    }

    // Опрос занятости системы
    bool isSystemBusy() {
        return isBusy;
    }
    // Сообщает название устройства, занимающего систему
    String getActiveDevice() {
        return activeDevice;
    }

    // Запрос на доступ к устройству (если система занята одним устройством, то остальные не получают к ней доступа)
    bool canAccessDevice(const char* deviceName) {
        if (!isBusy) return true;
        if (activeDevice == deviceName) return true;
        
        Serial.print("Error: The system is busy with the device'");
        Serial.print(activeDevice);
        Serial.println("'!");
        Serial.print("First stop '");
        Serial.print(activeDevice);
        Serial.print("' with the command '");
        Serial.print(activeDevice);
        Serial.println(":stop;'");
        return false;
    }
    // Запрос на исполнения команды
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
            Serial.print("Error: The system is busy with the device '");
            Serial.print(activeDevice);
            Serial.println("'!");
            return false;
        }
        
        // Если команда разрешена даже когда занято
        if (type == CommandType::ALLOWED_WHEN_BUSY) {
            return true;
        }
        
        // Команда заблокирована
        Serial.print("Error: Command '");
        Serial.print(command);
        Serial.println("' is not available during measurements!");
        Serial.println("Available commands: stop, ...");
        return false;
    }
}
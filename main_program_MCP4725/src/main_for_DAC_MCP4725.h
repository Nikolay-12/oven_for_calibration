#pragma once

#include "project_config.h"
#include "MultiStageDAC.h"
#include "Measurement_Processing.h"


extern MultiStageDACcontroller DACcontroller;

namespace DAC_MCP4725
{
    void clear();
    void init();
    
    //  Команды для обновления/удаления стадий по напряжению ЦАПа
    void updateStages(const char* stages); // ручная устанавка стадий по напряжению
    void setVoltagePoint(const float new_voltPoint); // ручная установка одной стадии по напряжению (установка после start)
    void resetStages(); // сброс всех стадий
    
    //  Команды для изменения настроек ЦАП (V)
    void updateSettings(const float* param); // Изменение следующих настроек ЦАП: Vmin, Vmax, deltaV, holding time
    void updateTimeSettings(const float new_hold_time); // Изменение временных настроек ЦАП: holding time
    void updateMeasurementSettings(const int cycles_num, const float period_s); // Изменение настроек ЦАП для этапа измерений: // Изменение временных настроек ЦАП: number of cycles и period (sec)
    void changeMode(const int mode);

    //  Команды для включения, выключения и работы ЦАП
    void inProgress(); // Основная функция ЦАП во время работы по сценарию (этапы выдержки и измерений)
    void on(); // Включение ЦАП
    void off(); // Выключение ЦАП

    //  Команды для получения данных о ЦАПе (текущий уровень, настроек, состоянии, режиме работы)
    int getLevel();
    void getData();
    bool getState();
    uint8_t getMode();

    //  Функции для непосредственной работы с ЦАПом
    //void setVoltage(float voltage); // конвертирует напряжение в необходимый уровень сигнала для ЦАП
    void setOutputLevel(size_t level, bool save_to_eeprom); // устанавливает уровень сигнала ЦАП
};
#pragma once

#include <Arduino.h>

// ==================================================
//  КЛАСС ПИД-РЕГУЛЯТОРА
// ==================================================
class PIDController {
private:
    double kp, ki, kd;
    double setpoint;
    double integral;
    double prev_error;
    double prev_output;
    double dt;
    double output_min;
    double output_max;
    double integral_limit;
    
public:
    PIDController(double kp, double ki, double kd, double dt,
                  int16_t output_min = 0, int16_t output_max = 255)
        : kp(kp), ki(ki), kd(kd), dt(dt),
          output_min(output_min), output_max(output_max) {
        reset();
        setIntegralLimit(output_max * 1.0);
    }
    
    void reset() {
        integral = 0.0;
        prev_error = 0.0;
        prev_output = 0.0;
    }

    void updateSettings(double new_Kp, double new_Ki, double new_Kd, double new_dt){
        kp = new_Kp;
        ki = new_Ki;
        kd = new_Kd;
        dt = new_dt;
    }
    
    void setSetpoint(double sp) {
        setpoint = sp;
    }
    
    double getSetpoint() const { return setpoint; }
    
    void setIntegralLimit(double limit) { integral_limit = limit; }
    
    double update(double measured_value) {
        double error = setpoint - measured_value;
        
        double p = kp * error;
        
        integral += error * dt;
        if (integral > integral_limit) integral = integral_limit;
        if (integral < -integral_limit) integral = -integral_limit;
        double i = ki * integral;
        
        double derivative = (error - prev_error) / dt;
        double d = kd * derivative;
        
        double output = p + i + d;
        // Ограничение управляющего сигнала в диапазоне [0, 255] (обычно 8-битный ШИМ-канал)
        if (output > output_max) output = output_max;
        if (output < output_min) output = output_min;
        
        // Anti-windup
        if ((output >= output_max && error > 0) || (output <= output_min && error < 0)) {
            integral -= error * dt;
        }
        
        prev_error = error;
        prev_output = output;
        return output;
    }
};
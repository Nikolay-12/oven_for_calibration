# Настройка параметров для ПИД-регулятора
DEFAULT_KP = 10.0      # Дефолтное значение для коэффициента Kp
DEFAULT_KI = 0.3      # Дефолтное значение для коэффициента Ki
DEFAULT_KD = 0.8      # Дефолтное значение для коэффициента Kd
DEFAULT_DT = 0.1      # Шаг дискретизации 100 мс
DEFAULT_stabilization_time = 5.0      # Время, которое необходимо для подтверждения стабилизации температуры (с)
DEFAULT_holding_time = 10.0      # Время удержания данной температуры (в это время идет сбор всех данных с датчиков) (с)
DEFAULT_temp_tolerance = 1.0      # Температурная погрешность
DEFAULT_max_stab_time = 30.0      # максимальное время на одной стадии
DEFAULT_TEMPERATURE_STEP = 10.0      # 
DEFAULT_TEMPERATURE_MIN = 30.0      # 
DEFAULT_TEMPERATURE_MAX = 100.0      # 
VOLTAGE_MIN_FOR_IT_MODE = 10      #

# Настройка параметров для многостадийного ЦАП
DEFAULT_VOLTAGE_STEP = 0.1      # 
DEFAULT_VOLTAGE_MIN = 0.0      # 
DEFAULT_VOLTAGE_MAX = 1.0      #

# Настройка параметров для измерений
DEFAULT_MEASUREMENT_CYCLES = 10      #
DEFAULT_MEASUREMENT_PERIOD = 2.0      #
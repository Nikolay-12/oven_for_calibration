from abc import abstractmethod


class Event:
    def is_async(self) -> bool:
        return False

class ComportSettingsEvent(Event):
    def __init__(self, comport_name, baudrate):
        self.comport_name = comport_name
        self.baudrate = baudrate
class ComportInitializedEvent(Event):
   def __str__(self):
      return "Serial port was initialized"

class SetRTParametersEvent(Event):
   def __init__(self, kp, ki, kd, dt, stab_time, hold_time, tol, max_stab_time, Tmin, Tmax, deltaT, stages, mode):
      self.kp = kp
      self.ki = ki
      self.kd = kd
      self.dt = dt
      self.stab_time = stab_time
      self.hold_time = hold_time
      self.tol = tol
      self.max_stab_time = max_stab_time
      self.Tmin = Tmin
      self.Tmax = Tmax
      self.deltaT = deltaT
      self.stages = stages
      self.mode = mode
class RTParametersWereSetEvent(Event):
   def __str__(self):
      return "RT parameters were set"
class SetRVParametersEvent(Event):
   def __init__(self, hold_time, Vmin, Vmax, deltaV, stages, mode):
      self.hold_time = hold_time
      self.Vmin = Vmin
      self.Vmax = Vmax
      self.deltaV = deltaV
      self.stages = stages
      self.mode = mode
class RVParametersWereSetEvent(Event):
   def __str__(self):
      return "RV parameters were set"

class SetMeasurementParametersEvent:
   def __init__(self, measurement_cycles, measurement_period):
      self.measurement_cycles = measurement_cycles
      self.measurement_period = measurement_period
class MeasurementParametersWereSetEvent:
   def __str__(self):
      return "Measurement parameters were set"
class StartReadingFromINA219:
   def __init__(self, mode):
      self.mode = mode
class ReadingFromINA219Started:
   def __str__(self):
      return "Current measuring from INA219 started"
class StopReadingFromINA219:
   pass
class ReadingFromINA219Stoped:
   def __str__(self):
      return "Current measuring were stoped"

class StartScenarioEvent(Event):
   def __init__(self, scenario_counter, scenario_commands):
      self.scenario_counter = scenario_counter
      self.scenario_commands = scenario_commands
class ScenarioStartedEvent(Event):
   def __str__(self):
      return "measurement in scenario mode started (button doesnt complete)"
class StopScenarioEvent(Event):
   pass
class ScenarioStopedEvent(Event):
   def __str__(self):
      return "measurement in scenario mode stoped (button doesnt complete)"
class RestartScenarioEvent(Event):
   def __init__(self, remaining_script_counter, remaining_script_commands):
      self.remaining_script_counter = remaining_script_counter
      self.remaining_script_commands = remaining_script_commands
class ScenarioRestartedEvent(Event):
   def __str__(self):
      return "measurement in scenario mode restarted (button doesnt complete)"
class EndScenarioEvent(Event):
   pass
class ScenarioEndedEvent(Event):
   def __str__(self):
      return "measurement in scenario mode ended (button doesnt complete)"

class StartManualTempEvent(Event):
   def __init__(self, T):
      self.T = T
class StartManualVoltEvent(Event):
   def __init__(self, V):
      self.V = V
class ManualStartedEvent(Event):
   def __str__(self):
      return "measurement in manual mode started"
class StopManualEvent(Event):
   pass
class ManualStopedEvent(Event):
   pass

class SelectingDevices(Event):
   def __init__(self, list_of_used_devices):
      self.selected_devices = list_of_used_devices
class DevicesWereSelected(Event):
   def __str__(self):
      return "devices were chosen"

class EndlessReadingEvent(Event):
    def is_async(self) -> bool:
        return True

class EndlessReadingStartedEvent(Event):
   def __str__(self):
      return "reading was started"
class StopReadingEvent(Event):
   pass
class ReadingStopedEvent(Event):
   def __str__(self):
      return "reading was stoped"

class ScenarioGenerationEvent(Event):
   def __init__(self, file_path, Tmin, Tmax, delta_T, delta_t, num_of_val_for_averaging, data_reading_frequency):
      self.file_path = file_path
      self.Tmin = Tmin
      self.Tmax = Tmax
      self.delta_T = delta_T
      self.delta_t = delta_t
      self.num_of_val_for_averaging = num_of_val_for_averaging
      self.data_reading_frequency = data_reading_frequency
class ScenarioWasGeneratedEvent(Event):
   def __str__(self):
      return "Scenario was created"

class UpdateStatusBar(Event):
    def __init__(self, message: str) -> None:
       super().__init__()
       self.message = message
class UpdateMonitoringTabInfoFromUsedDevices(Event):
    def __init__(self, data_array: list[float]) -> None:
       super().__init__()
       self.data_array = data_array
class UpdateMonitoringTabRealTimePlotting(Event):
    def __init__(self, data_array: list[float]) -> None:
       super().__init__()
       self.data_array = data_array

from queue import Queue
import threading
from tkinter.messagebox import showerror, showinfo
from numpy import log
import serial
import logging
logger = logging.getLogger(__name__)

from events import ComportSettingsEvent, ComportInitializedEvent, SetRTParametersEvent, RTParametersWereSetEvent, \
    SetRVParametersEvent, RVParametersWereSetEvent, \
    SetMeasurementParametersEvent, MeasurementParametersWereSetEvent, StartReadingFromINA219, ReadingFromINA219Started, \
    StopReadingFromINA219, ReadingFromINA219Stoped, \
    StartScenarioEvent, ScenarioStartedEvent, StopScenarioEvent, ScenarioStopedEvent, EndlessReadingEvent, \
    EndlessReadingStartedEvent, StopReadingEvent, ReadingStopedEvent, RestartScenarioEvent, ScenarioRestartedEvent, \
    SelectingDevices, DevicesWereSelected, ScenarioGenerationEvent, ScenarioWasGeneratedEvent, StartManualTempEvent, \
    ManualStartedEvent, StartManualVoltEvent, StopManualEvent, ManualStopedEvent
from data_exchange import DataExchange
from scenario_generator import ScenarioGenerator


class FunctionForTaskProcessing:
    def __init__(self, parent, update_status_bar):
        self.ser = None
        self.remaining_script_data = None
        self.lines_for_reading = 0
        self.method_to_update_status_bar = update_status_bar
        self.data_exchange = DataExchange(parent)

    def func_for_task_processing(self, task, task_queue, task_done_queue):
        try:
            logger.info(f"Received task: {task}")
            # Обработка событий настройки параметров процессов
            if isinstance(task, ComportSettingsEvent):
                self.update_comport_settings(task.comport_name, task.baudrate)
                return ComportInitializedEvent()
            if isinstance(task, SetRTParametersEvent):
                self.update_RT_settings(task.kp, task.ki, task.kd, task.dt, task.stab_time, task.hold_time, task.tol,
                                        task.max_stab_time, task.Tmin, task.Tmax, task.deltaT, task.stages, task.mode)
                return RTParametersWereSetEvent()
            if isinstance(task, SetRVParametersEvent):
                self.update_RV_settings(task.hold_time, task.Vmin, task.Vmax, task.deltaV, task.stages, task.mode)
                return RVParametersWereSetEvent()
            if isinstance(task, SetMeasurementParametersEvent):
                self.update_measurement_settings(task.measurement_cycles, task.measurement_period)
                return MeasurementParametersWereSetEvent()

            # Обработка событий, связанных с измерениями с INA219
            if isinstance(task, StartReadingFromINA219):
                self.start_measuring_from_INA219(task.mode, task_queue = task_queue)
                task_done_queue.put(ReadingFromINA219Started)
                #return ReadingFromINA219Started()
            #if isinstance(task, StopReadingFromINA219):
            #    self.stop_measuring_from_INA219()
            #    return ReadingFromINA219Stoped()

            # Обработка событий, связанных с измерениями с работой по сценарию
            if isinstance(task, StartScenarioEvent):
                self.start_scenario(task.scenario_counter, task.scenario_commands)
                task_done_queue.put(ScenarioStartedEvent)
            if isinstance(task, StopScenarioEvent):
                self.remaining_script_data = self.stop_scenario()
                logger.info(f"Часть сценария, которая осталась невыполненной: {self.remaining_script_data}")
                return ScenarioStopedEvent()
            if isinstance(task, RestartScenarioEvent):
                self.restart_scenario()
                return ScenarioRestartedEvent()

            # Обработка событий, связанных с измерениями с работой в ручном режиме
            if isinstance(task, StartManualTempEvent):
                self.start_measurement_in_manual_mode_with_temp(task.T)
                return ManualStartedEvent()
            if isinstance(task, StartManualVoltEvent):
                self.start_measurement_in_manual_mode_with_temp(task.V)
                return ManualStartedEvent()
            if isinstance(task, StopManualEvent):
                self.manual_stop()
                return ManualStopedEvent()

            # Обработка событий, связанных с обработкой полученных данных (выбор показываемых данных, сохранение, построение, аппроксимация)
            if isinstance(task, SelectingDevices):
                logger.info(f"Selected devices:{task.selected_devices}")
                return DevicesWereSelected()
            if task == "save_input_data":
                pass
            if task == "read_and_plot":
                pass
            if task == "approximate":
                pass



            if isinstance(task, StopReadingEvent):
                #self.data_exchange.send_line(self.ser, command="stop_reading;")
                return ReadingStopedEvent()
            if isinstance(task, ScenarioGenerationEvent):
                ScenarioGenerator.scenario_creation(task.file_path, task.Tmin, task.Tmax, task.delta_T,
                                                    task.delta_t, task.num_of_val_for_averaging,
                                                    task.data_reading_frequency)
                return ScenarioWasGeneratedEvent()
            return None

        except serial.SerialException as se:
            logger.error("Serial port error:", str(se))
            return None

        except KeyboardInterrupt:
            return None


    def func_for_async_task(self, task, task_done_queue) -> threading.Event:
        stop_event = threading.Event()
        logger.info(f"Received async task: {task}")
        if isinstance(task, EndlessReadingEvent):
            self.data_exchange.send_and_endless_receive(self.ser, "read;", stop_event, False, task_done_queue)
        return stop_event

    # Функции-обработчики событий
    def update_comport_settings(self, comport_name, baudrate):
        if self.ser is not None:
            self.ser.close()
        self.ser = serial.Serial(port=comport_name, baudrate=baudrate, timeout=3, bytesize=serial.EIGHTBITS,
                                 xonxoff=False, stopbits=1, parity=serial.PARITY_NONE, dsrdtr=False)

    def update_RT_settings(self, kp, ki, kd, dt, stab_time, hold_time, tol, max_stab_time, Tmin, Tmax, deltaT, stages, mode):
        self.data_exchange.send_and_receive_n_lines(serial_port=self.ser,
                                                    command=f"thermreg:PIDsettings:{kp}_{ki}_{kd}_{dt};",
                                                    n = 2, to_process = False)
        self.data_exchange.send_and_receive_n_lines(serial_port=self.ser,
                                                    command=f"thermreg:time&tolSettings:{stab_time}_{hold_time}_{tol}_{max_stab_time};",
                                                    n = 2, to_process = False)
        self.data_exchange.send_and_receive_n_lines(serial_port=self.ser,
                                                    command=f"thermreg:tempSettings:{Tmin}_{Tmax}_{deltaT};",
                                                    n = 2, to_process = False)
        self.data_exchange.send_and_receive_n_lines(serial_port=self.ser,
                                                    command=f"thermreg:tempStages:{stages};",
                                                    n = 3, to_process = False)
        self.data_exchange.send_and_receive_n_lines(serial_port=self.ser,
                                                    command=f"thermreg:changeMode:{mode};",
                                                    n = 1, to_process = False)
    def update_RV_settings(self, hold_time, Vmin, Vmax, deltaV, stages, mode):
        self.data_exchange.send_and_receive_n_lines(serial_port=self.ser,
                                                    command=f"DAC:settings:{Vmin}_{Vmax}_{deltaV}_{hold_time};",
                                                    n = 3, to_process = False)
        self.data_exchange.send_and_receive_n_lines(serial_port=self.ser,
                                                    command=f"DAC:voltageSettings:{stages};",
                                                    n = 3, to_process = False)
        self.data_exchange.send_and_receive_n_lines(serial_port=self.ser,
                                                    command=f"DAC:changeMode:{mode};",
                                                    n = 3, to_process = False)

    def update_measurement_settings(self, measurement_cycles, measurement_period):
        self.lines_for_reading = measurement_cycles
        self.data_exchange.send_and_receive_n_lines(serial_port=self.ser,
                                                    command=f"INA219:measurementSettings:{measurement_cycles}_{measurement_period};",
                                                    n = 3, to_process = False)
    def start_measuring_from_INA219(self, measurement_mode, task_queue):
        if measurement_mode == 1:
            self.data_exchange.send_and_receive_n_lines(serial_port=self.ser,
                                                        command="INA219:readNtimes;",
                                                        n = int(self.lines_for_reading + 2), to_process = False)
        if measurement_mode == 2:
            self.data_exchange.send_and_endless_receive(serial_port=self.ser,
                                                        command=f"INA219:reading;",
                                                        task_queue=task_queue, to_process = False)
    #def stop_measuring_from_INA219(self):
        """
        self.data_exchange.measuring_active = False
        self.data_exchange.send_and_receive_n_lines(serial_port=self.ser,
                                                    command="INA219:stop;",
                                                    n = 1, to_process = False)
        """


    def start_scenario(self, scenario_counter, scenario_commands):
        command = f"scenario;{scenario_counter};{scenario_commands}"
        logger.debug(command)
        answer = self.data_exchange.send_line_and_get_response(self.ser, command)
        logger.info(answer)

    def stop_scenario(self):
        answer = self.data_exchange.send_line_and_get_response(serial_port=self.ser, command="stop_scenario;")
        remaining_scenario_counter, remaining_scenario_commands = self.data_exchange.get_remaining_scenario_data(answer)
        return  remaining_scenario_counter, remaining_scenario_commands

    def restart_scenario(self):
        if self.remaining_script_data is not None:
            command = f"scenario:{self.remaining_script_data[0]};{self.remaining_script_data[1]}"
            self.data_exchange.send_and_receive_n_lines(serial_port=self.ser, command=command, n = 2, to_process = False)
        else:
            showerror(title='Ошибка', message='Сценарий не был запущен!')
            logger.error("Сценарий не был запущен!")

    def start_measurement_in_manual_mode_with_temp(self, T):
        answer = self.data_exchange.send_line_and_get_response(serial_port=self.ser, command=f"T;{T}")
        logger.info(answer)
    def start_measurement_in_manual_mode_with_volt(self, V):
        answer = self.data_exchange.send_line_and_get_response(serial_port=self.ser, command=f"V;{V}")
        logger.info(answer)
    def manual_stop(self):
        answer = self.data_exchange.send_line_and_get_response(serial_port=self.ser, command=f"manual_stop;")
        logger.info(answer)

    def endless_reading(self, task_queue):
        self.data_exchange.send_and_endless_receive(serial_port=self.ser, command="read;", task_queue=task_queue, to_process = False)

'''       
        finally:
            if ser.is_open:
                ser.close()
                print("Serial connection closed.")
'''

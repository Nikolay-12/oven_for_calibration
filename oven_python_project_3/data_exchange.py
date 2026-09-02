import serial

from data_processing import string_to_float_array, string_to_float_array_without_last_symbol
from stopwatch import get_current_time, elapsed_time_in_sec
from events import StopReadingFromINA219, UpdateMonitoringTabInfoFromUsedDevices, UpdateMonitoringTabRealTimePlotting, UpdateStatusBar
import time
import queue
import threading
import logging
logger = logging.getLogger(__name__)

class DataExchange:
    def __init__(self, parent):
        self.parent = parent
        self.ser = None
        self.reading_thread = None
        self.data_array = None
        self.start_time = None
        self.measuring_active = False

    @staticmethod
    def get_remaining_scenario_data(input_line):
        remaining_scenario_counter = input_line[:input_line.find(';')] if ';' in input_line else input_line
        remaining_scenario_commands = input_line[input_line.find(';')+1:] if ';' in input_line else input_line
        logger.info(f"remaining_scenario_counter: {remaining_scenario_counter}, "
              f"remaining_scenario_commands: {remaining_scenario_commands}")
        return remaining_scenario_counter, remaining_scenario_commands



    def send_and_endless_receive(self, serial_port, command, stop_event, to_process, task_done_queue):
        self.ser = serial_port
        # self.checking_command_content(command)
        encoded_command = command.encode()
        self.ser.write(encoded_command)

        self.ser.timeout = 0.5

        self.reading_thread = threading.Thread(
            target=self._read_loop,
            args=(to_process, stop_event, task_done_queue),
            daemon=True
        )
        self.reading_thread.start()

    def _read_loop(self, to_process, stop_event, task_done_queue):
        # Поток для чтения данных
        buffer = ""

        while not stop_event.is_set():
            try:
                if self.ser and self.ser.in_waiting > 0:
                    data = self.ser.read(self.ser.in_waiting)
                    try:
                        decoded_data = data.decode("utf-8", errors="ignore")
                        buffer += decoded_data

                        lines = buffer.split('\r\n')
                        buffer = lines[-1] if lines else ""

                        for line in lines[:-1]:
                            if line.strip():
                                self._process_line(line, to_process, task_done_queue)
                    except Exception as e:
                        logger.error(f"Ошибка декодирования: {e}")

                time.sleep(0.001)

            except Exception as e:
                logger.error(f"Ошибка в потоке чтения: {e}")
                break

    def _process_line(self, line, to_process, task_done_queue):
        # Обработка строки данных
        try:
            decoded_response = line.rstrip('\r\n')
            logger.info(decoded_response)
            task_done_queue.put(UpdateStatusBar(f"Данные {decoded_response}"))

            if to_process:
                data_array = string_to_float_array_without_last_symbol(decoded_response)
                task_done_queue.put(UpdateMonitoringTabInfoFromUsedDevices(data_array))
                current_time = get_current_time()
                logger.info(f"Time:{current_time} Data:{data_array}")
                elapsed_time = elapsed_time_in_sec(self.start_time, current_time)
                data_array.insert(0, float(elapsed_time))
                task_done_queue.put(UpdateMonitoringTabRealTimePlotting(data_array))

        except Exception as e:
            logger.error(f"Ошибка обработки: {e}")


        """
        def send_and_endless_receive(self, serial_port, command, task_queue, to_process):
        self.ser = serial_port
        self.checking_command_content(command)
        encoded_command = command.encode()
        self.ser.write(encoded_command)

        self.measuring_active = True
        self.ser.timeout = 0.01

        buffer = ""  # Буфер для накопления данных

        while self.measuring_active:
            if not task_queue.empty():
                try:
                    task = task_queue.get_nowait()
                    if isinstance(task, StopReadingFromINA219):
                        logger.info("Получена команда остановки")
                        self.measuring_active = False
                        break
                except queue.Empty:
                    pass

            try:
                response = self.ser.readline()
            except serial.SerialException as e:
                logger.error(f"Ошибка чтения из порта: {e}")
                break

            if response:
                try:
                    decoded_part = response.decode("utf-8")
                    buffer += decoded_part
                except UnicodeDecodeError as e:
                    decoded_part = response.decode("utf-8", errors="replace")
                    buffer += decoded_part
                    logger.warning(f"Ошибка декодирования: {e}, данные заменены")
                    
            if '\n' in buffer or '\r\n' in buffer:
                lines = buffer.split('\r\n')
                buffer = lines[-1] if lines else ""

                for line in lines[:-1]:
                    if line.strip():
                        decoded_response = line.rstrip('\r\n')
                        logger.info(decoded_response)

                        if to_process:
                            try:
                                self.data_array = string_to_float_array_without_last_symbol(decoded_response)
                                self.parent.monitoring_tab.info_from_used_devices.update_data(self.data_array)
                                current_time = get_current_time()
                                logger.info(f"Time:{current_time} Data:{self.data_array}")
                                elapsed_time = elapsed_time_in_sec(self.start_time, current_time)
                                self.data_array.insert(0, float(elapsed_time))
                                self.parent.monitoring_tab.real_time_plotting.update_plot(self.data_array)
                            except ValueError as e:
                                logger.error(f"Не удалось конвертировать данные: {e}")

            time.sleep(0.001)

        self.ser.write("INA219:stop;".encode())
        logger.info("Команда остановки отправлена на Arduino")
        """

        """
        def send_and_endless_receive(self, serial_port, command, task_queue, to_process):
        self.ser = serial_port
        self.checking_command_content(command)
        encoded_command = command.encode()
        self.ser.write(encoded_command)

        self.measuring_active = True
        self.ser.timeout = 0.01

        while self.measuring_active:
            if not task_queue.empty():
                try:
                    task = task_queue.get_nowait()
                    if isinstance(task, StopReadingFromINA219):
                        logger.info("Получена команда остановки")
                        self.measuring_active = False
                        break
                except queue.Empty:
                    pass

            response = self.ser.readline()
            if not response:
                continue

            decoded_response = response.decode("utf-8").rstrip('\r\n')
            logger.info(decoded_response)

            if to_process:
                try:
                    self.data_array = string_to_float_array_without_last_symbol(decoded_response)
                    self.parent.monitoring_tab.info_from_used_devices.update_data(self.data_array)
                    current_time = get_current_time()
                    logger.info(f"Time:{current_time} Data:{self.data_array}")
                    elapsed_time = elapsed_time_in_sec(self.start_time, current_time)
                    self.data_array.insert(0, float(elapsed_time))
                    self.parent.monitoring_tab.real_time_plotting.update_plot(self.data_array)
                except ValueError:
                    logger.error("Не удалось конвертировать данные из com-порта в массив данных типа float")

            time.sleep(0.001)
        self.ser.write("INA219:stop;".encode())
        logger.info("Команда остановки отправлена на Arduino")
        """
        """
        while task_queue.empty():
            response = self.ser.readline()
            decoded_response = response.decode("utf-8").rstrip('\r\n')
            logger.info(decoded_response)
            if to_process:
                try:
                    self.data_array = string_to_float_array_without_last_symbol(decoded_response)
                    self.parent.monitoring_tab.info_from_used_devices.update_data(self.data_array)
                    current_time = get_current_time()
                    logger.info(f"Time:{current_time} Data:{self.data_array}")
                    elapsed_time = elapsed_time_in_sec(self.start_time, current_time)
                    self.data_array.insert(0, float(elapsed_time))
                    self.parent.monitoring_tab.real_time_plotting.update_plot(self.data_array) #data_array_with_time
                except ValueError:
                    logger.error("Не удалось конвертировать данные из com-порта в массив данных типа float")
        """


    def send_and_receive_n_lines(self, serial_port, command, n, to_process):
        self.ser = serial_port
        # self.checking_command_content(command)
        encoded_command = command.encode()
        self.ser.write(encoded_command)
        for i in range(n):
            response = self.ser.readline()
            decoded_response = response.decode("utf-8").rstrip('\r\n')
            logger.info(decoded_response)
            if to_process:
                try:
                    self.data_array = string_to_float_array_without_last_symbol(decoded_response)
                    # self.parent.monitoring_tab.info_from_used_devices.update_data(self.data_array)
                    current_time = get_current_time()
                    logger.info(f"Time:{current_time} Data:{self.data_array}")
                    elapsed_time = elapsed_time_in_sec(self.start_time, current_time)
                    data_array_with_time = self.data_array.insert(0, float(elapsed_time))
                    # self.parent.monitoring_tab.real_time_plotting.update_plot(data_array_with_time)
                except ValueError:
                    logger.error("Не удалось конвертировать данные из com-порта в массив данных типа float")

    def send_line(self, serial_port, command):
        self.ser = serial_port
        encoded_command = command.encode()
        self.ser.write(encoded_command)
        logger.info(f"Command '{command}' was sent")

    def send_line_and_get_response(self, serial_port, command):
        self.ser = serial_port
        encoded_command = command.encode()
        self.ser.write(encoded_command)
        logger.info(f"Command '{command}' was sent")
        response = self.ser.readline()
        decoded_response = response.decode("utf-8").rstrip('\r\n')
        logger.info(decoded_response)
        return  decoded_response

    def checking_command_content(self, command):
        if command == "read;":
            self.start_time = get_current_time()
        if command == "stop_reading;":
            self.start_time = None

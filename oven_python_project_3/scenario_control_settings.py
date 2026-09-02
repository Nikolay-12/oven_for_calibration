import tkinter as tk
from tkinter import StringVar
from tkinter.messagebox import showerror
from tkinter import filedialog
import logging
logger = logging.getLogger(__name__)

from events import StartScenarioEvent, StopScenarioEvent, EndScenarioEvent, RestartScenarioEvent

common_button_ui_params = {"font": "TkDefaultFont", "bd": 3}
common_grid_ui_params = dict(padx=10, pady=5)

class ScenarioControlSettings:
    def __init__(self, parent, floating_point_validation_function, task_queue, method_to_update_status_bar) -> None:
        self.parent = parent
        self.floating_point_validation_function = floating_point_validation_function
        self.task_queue = task_queue
        self.method_to_update_status_bar = method_to_update_status_bar
        self._scenario_file_address = StringVar(value='')
        self._command_line = ''
        self._line_counter = 0
        self.scenario_flag = False

    def ui_draw(self):
        self.scenario_control_settings_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Управление по сценарию")
        self.scenario_control_settings_frame.grid(padx=10, pady=5, row=1, column=3, sticky=tk.NSEW)
        tk.Label(self.scenario_control_settings_frame, text="Сценарий:").grid(row=1, column=1)
        self.scenario_selection_button = tk.Button(
            self.scenario_control_settings_frame,
            text="Выбрать",
            command=self.choose_mode_file,
            bd=3)
        self.scenario_selection_button.grid(row=1, column=2, pady=3, sticky=tk.W)

        tk.Label(self.scenario_control_settings_frame, text="Адрес файла:").grid(row=2, column=1)
        tk.Entry(
            self.scenario_control_settings_frame,
            state='disabled',
            textvariable=self._scenario_file_address).grid(row=2, column=2)  # , sticky=tk.NSEW
        self.scenario_start_button = tk.Button(
            self.scenario_control_settings_frame,
            text="Запустить\n сценарий",
            bg="Green",
            command=self.start_measurement_in_scenario_mode,
            **common_button_ui_params)
        self.scenario_start_button.grid(row=3, column=1, **common_grid_ui_params)
        self.scenario_stop_button = tk.Button(
            self.scenario_control_settings_frame,
            text="Приостановить\n сценарий",
            bg="Red",
            command=self.stop_measurement_in_scenario_mode,
            **common_button_ui_params)
        self.scenario_stop_button.grid(row=3, column=2, **common_grid_ui_params)

    def choose_mode_file(self):
        file_path = filedialog.askopenfilename()
        self._scenario_file_address.set(file_path)
        #logger.info(f"Selected file: {file_path}")
        self.method_to_update_status_bar(f"Selected file: {file_path}")

    def lock_scenario_mode(self):
        self.scenario_control_settings_frame.config(font="TkDefaultFont")
        self.scenario_selection_button.config(state="disabled", disabledforeground="black")
        self.scenario_start_button.config(state="disabled", disabledforeground="black")
        self.scenario_stop_button.config(state="disabled", disabledforeground="black")

    def unlock_scenario_mode(self):
        self.scenario_control_settings_frame.config(font=("DejaVu Sans", 9, "bold"))
        self.scenario_selection_button.config(state="normal")
        self.scenario_start_button.config(state="normal")
        self.scenario_stop_button.config(state="normal")

    def start_measurement_in_scenario_mode(self):
        if not self.scenario_flag:
            try:
                file_path = self.scenario_file_address
                self._command_line = ''
                self._line_counter = 0
                with open(file_path, "r") as file:
                    for line in file:
                        data_line = line.replace('\n', ' ')
                        self._command_line += data_line
                        self._line_counter += 1
                logger.info(f"Data from file {self.scenario_file_address}: {self.command_line} "
                      f"\nNumber of commands:{self.line_counter}")
                self.method_to_update_status_bar(f"Data from file {self.scenario_file_address}: {self.command_line} "
                      f"\nNumber of commands:{self.line_counter}")
                self.task_queue.put(StartScenarioEvent(self.line_counter, self.command_line))
                self.scenario_flag = True
                #self.parent.working_with_devices_by_COM_port.read_data_from_devices_and_write_it()
                #self.parent.working_with_devices_by_COM_port.start_working_on_the_script()
            except FileNotFoundError:
                showerror(title='Ошибка', message='Файл не выбран!')
                logger.error("Не выбран файл-сценарий")

        if self.scenario_flag:
            self.task_queue.put(RestartScenarioEvent(self.line_counter, self.command_line))


    def stop_measurement_in_scenario_mode(self):
        self.task_queue.put(StopScenarioEvent())

    def end_measurement_in_scenario_mode(self):
        self.task_queue.put(EndScenarioEvent())
        self.scenario_flag = False


    @property
    def all_scenario_settings(self):
        return self._scenario_file_address.get(), self._command_line, self._line_counter

    @property
    def scenario_file_address(self):
        return self._scenario_file_address.get()

    @property
    def command_line(self):
        return self._command_line

    @property
    def line_counter(self):
        return self._line_counter
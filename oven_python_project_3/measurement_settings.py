from tkinter import IntVar, DoubleVar, StringVar
import tkinter as tk
from tkinter import ttk
from tkinter.messagebox import showerror
import logging
import basic_default_settings as settings
from utils.input_validation import is_valid_non_zero_integer
logger = logging.getLogger(__name__)

from events import SetMeasurementParametersEvent, StartReadingFromINA219, StopReadingFromINA219

common_button_ui_params = {"font": "TkDefaultFont", "bd": 3}
common_grid_ui_params = dict(pady=8)

class MeasurementSettings:
    def __init__(self, parent, floating_point_validation_function, task_queue, method_to_update_status_bar) -> None:
        self.parent = parent
        self.floating_point_validation_function = floating_point_validation_function
        self.task_queue = task_queue
        self.method_to_update_status_bar = method_to_update_status_bar
        self._measurement_cycles = DoubleVar(value=settings.DEFAULT_MEASUREMENT_CYCLES)
        self._measurement_period = DoubleVar(value=settings.DEFAULT_MEASUREMENT_PERIOD)
        self._measurement_mode = 0

        self.check_integer = (self.parent.register(is_valid_non_zero_integer), "%P")

    def ui_draw(self):
        measurement_settings_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Настройки измерений")
        measurement_settings_frame.grid(padx=10, pady=5, row=4, column=1, sticky=tk.NSEW)
        tk.Label(measurement_settings_frame, text="Measure\ncycles:").grid(row=1, column=1)
        tk.Label(measurement_settings_frame, text="Measure\nperiod:").grid(row=1, column=3)

        entry_cycles_number = tk.Entry(
            measurement_settings_frame,
            validate="key",
            validatecommand=self.check_integer,
            textvariable=self._measurement_cycles,
            width=4,
        )
        entry_cycles_number.grid(padx=[2,6], row=1, column=2)
        entry_measurement_period = tk.Entry(
            measurement_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._measurement_period,
            width=4,
        )
        entry_measurement_period.grid(padx=[2,6], row=1, column=4)

        tk.Label(measurement_settings_frame, text="modes:").grid(row=2, column=1)
        # Словарь соответствия
        # RT_modes_options = {"Вариант 1": 0, "Вариант 2": 1, "Вариант 3": 2}
        self.measurement_modes_combobox = ttk.Combobox(measurement_settings_frame,
                                              values=["Idle mode",
                                                      "Reading currents several times",
                                                      "Reading currents"],
                                              state="readonly")
        self.measurement_modes_combobox.current(0)
        self.measurement_modes_combobox.grid(padx=[2, 0], row=2, column=2, columnspan=4)
        self.measurement_modes_combobox.bind("<<ComboboxSelected>>", self.measurement_mode_selected)

        self.apply_button = tk.Button(
            measurement_settings_frame,
            text="Применить",
            bg="yellow",
            command=self.change_measurement_settings,
            bd=3)
        self.apply_button.grid(row=1, column=7, columnspan=2, rowspan=2, sticky=tk.E, **common_grid_ui_params)
        self.start_button = tk.Button(
            measurement_settings_frame,
            text="Начать",
            bg="green",
            command=self.start_measurement,
            bd=3)
        self.start_button.grid(row=3, column=1, columnspan=3, **common_grid_ui_params)
        self.stop_button = tk.Button(
            measurement_settings_frame,
            text="Остановить",
            bg="red",
            command=self.stop_measurement,
            bd=3)
        self.stop_button.grid(row=3, column=4, columnspan=3, **common_grid_ui_params)

    def change_measurement_settings(self):
        try:
            self.apply_button.config(bg="yellow")
            logger.info(f"Measurement parameters: {self.measurement_cycles}, {self.measurement_period}")
            self.method_to_update_status_bar(f"Measurement parameters: {self.measurement_cycles}, {self.measurement_period}")
            self.task_queue.put(SetMeasurementParametersEvent(self.measurement_cycles, self.measurement_period))
        except tk.TclError:
            showerror(title='Error', message='Some measurement settings are not specified; please double-check them!')
            logger.error("Some measurement settings are not specified.")

    def start_measurement(self):
        if self.measurement_mode == 0:
            showerror(title='Error', message='Measurement mode was not specified!')
            logger.error("Measurement mode was not specified.")
        else:
            self.task_queue.put(StartReadingFromINA219(self.measurement_mode))
    def stop_measurement(self):
        self.task_queue.put(StopReadingFromINA219)

    def update_button_color(self):
        self.apply_button.config(bg="light green")

    def measurement_mode_selected(self, event):
        self._measurement_mode = self.measurement_modes_combobox.current()  # 0, 1, 2


    @property
    def measurement_cycles(self):
        return self._measurement_cycles.get()

    @property
    def measurement_period(self):
        return self._measurement_period.get()

    @property
    def measurement_mode(self):
        return self._measurement_mode
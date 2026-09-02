from tkinter import IntVar, DoubleVar, StringVar
import tkinter as tk
from tkinter import ttk
from tkinter.messagebox import showerror
import logging
import basic_default_settings as settings
from utils.input_validation import is_valid_stages
logger = logging.getLogger(__name__)

from events import SetRVParametersEvent

common_button_ui_params = {"font": "TkDefaultFont", "bd": 3}
common_grid_ui_params = dict(pady=8)

class RVSettings:
    def __init__(self, parent, floating_point_validation_function, task_queue, method_to_update_status_bar) -> None:
        self.parent = parent
        self.floating_point_validation_function = floating_point_validation_function
        self.task_queue = task_queue
        self.method_to_update_status_bar = method_to_update_status_bar
        self._holding_time = DoubleVar(value=settings.DEFAULT_holding_time)
        self._Vmin = DoubleVar(value=settings.DEFAULT_VOLTAGE_MIN)
        self._Vmax = DoubleVar(value=settings.DEFAULT_VOLTAGE_MAX)
        self._deltaV = DoubleVar(value=settings.DEFAULT_VOLTAGE_STEP)
        self._voltage_stages = StringVar(value="")
        self._RV_mode = 0

        self.check_stages_string = (self.parent.register(is_valid_stages), "%P")

    def ui_draw(self):
        RV_settings_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Настройки режима R(V)")
        RV_settings_frame.grid(padx=10, pady=5, row=3, column=1, sticky=tk.NSEW)
        tk.Label(RV_settings_frame, text="Основные настройки:").grid(row=1, column=1, columnspan=4, sticky=tk.W)
        tk.Label(RV_settings_frame, text="hold\ntime:").grid(row=2, column=1)
        tk.Label(RV_settings_frame, text="Vmin:").grid(row=2, column=3)
        tk.Label(RV_settings_frame, text="Vmax:").grid(row=2, column=5)
        tk.Label(RV_settings_frame, text="deltaV:").grid(row=2, column=7)

        entry_hold_time = tk.Entry(
            RV_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._holding_time,
            width=4,
        )
        entry_hold_time.grid(padx=[2, 6], row=2, column=2)
        entry_Vmin = tk.Entry(
            RV_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._Vmin,
            width=4,
        )
        entry_Vmin.grid(padx=[2, 6], row=2, column=4)
        entry_Vmax = tk.Entry(
            RV_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._Vmax,
            width=4,
        )
        entry_Vmax.grid(padx=[2, 6], row=2, column=6)
        entry_deltaV = tk.Entry(
            RV_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._deltaV,
            width=4,
        )
        entry_deltaV.grid(padx=[2, 0], row=2, column=8)

        tk.Label(RV_settings_frame, text="stages:").grid(row=3, column=1)
        entry_volt_stages = tk.Entry(
            RV_settings_frame,
            validate="key",
            validatecommand=self.check_stages_string,
            textvariable=self._voltage_stages,
            width=4,
        )
        entry_volt_stages.grid(padx=[2, 0], row=3, column=2, columnspan=4, sticky=tk.EW)

        tk.Label(RV_settings_frame, text="modes:").grid(row=4, column=1)
        # Словарь соответствия
        # RT_modes_options = {"Вариант 1": 0, "Вариант 2": 1, "Вариант 3": 2}
        self.RV_modes_combobox = ttk.Combobox(RV_settings_frame,
                                              values=["Idle mode", "Сustom stages mode",
                                                      "Gradual increasing mode", "Gradual decreasing mode"],
                                              state="readonly")
        self.RV_modes_combobox.current(0)
        self.RV_modes_combobox.grid(padx=[2, 0], row=4, column=2, columnspan=4)
        self.RV_modes_combobox.bind("<<ComboboxSelected>>", self.RV_mode_selected)

        self.apply_button = tk.Button(
            RV_settings_frame,
            text="Изменить",
            bg="yellow",
            command=self.change_RT_settings,
            bd=3)
        self.apply_button.grid(row=4, column=6, columnspan=3, sticky=tk.E, **common_grid_ui_params)

    def change_RT_settings(self):
        try:
            self.apply_button.config(bg="yellow")
            logger.info(f"RV parameters: {self.hold_time}, {self.Vmin}, {self.Vmax}, {self.deltaV} ... ")
            self.method_to_update_status_bar(f"RV parameters: {self.hold_time}, {self.Vmin}, {self.Vmax}, {self.deltaV} ... ")
            self.task_queue.put(SetRVParametersEvent(self.hold_time, self.Vmin, self.Vmax,
                                                     self.deltaV, self.volt_stages, self.RV_mode))
        except tk.TclError:
            showerror(title='Ошибка', message='Некоторые настройки R(V) режима не заданы, перепроверьте их!')
            logger.error("Некоторые настройки R(V) режима не заданы")

    def update_button_color(self):
        self.apply_button.config(bg="light green")

    def RV_mode_selected(self, event):
        self._RV_mode = self.RV_modes_combobox.current()  # 0, 1, 2

    @property
    def hold_time(self):
        return self._holding_time.get()

    @property
    def Vmin(self):
        return self._Vmin.get()

    @property
    def Vmax(self):
        return self._Vmax.get()

    @property
    def deltaV(self):
        return self._deltaV.get()

    @property
    def volt_stages(self):
        return self._voltage_stages.get()

    @property
    def RV_mode(self):
        return self._RV_mode


"""
@property
    def RT_mode(self):
        return self._RT_mode.get()
values = ["Idle mode", "Сustom stages mode",
                  "Gradual increasing mode", "Gradual decreasing mode",
                  "Reading average temperature", "Reading all temperatures"])
"""

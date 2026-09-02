from tkinter import IntVar, DoubleVar, StringVar
import tkinter as tk
from tkinter import ttk
from tkinter.messagebox import showerror
import logging
import basic_default_settings as settings
from utils.input_validation import is_valid_stages
logger = logging.getLogger(__name__)

from events import SetRTParametersEvent

common_button_ui_params = {"font": "TkDefaultFont", "bd": 3}
common_grid_ui_params = dict(pady=8)

class RTSettings:
    def __init__(self, parent, floating_point_validation_function, task_queue, method_to_update_status_bar) -> None:
        self.parent = parent
        self.floating_point_validation_function = floating_point_validation_function
        self.task_queue = task_queue
        self.method_to_update_status_bar = method_to_update_status_bar
        self._kp = DoubleVar(value=settings.DEFAULT_KP)
        self._ki = DoubleVar(value=settings.DEFAULT_KI)
        self._kd = DoubleVar(value=settings.DEFAULT_KD)
        self._dt = DoubleVar(value=settings.DEFAULT_DT)
        self._stabilization_time = DoubleVar(value=settings.DEFAULT_stabilization_time)
        self._holding_time = DoubleVar(value=settings.DEFAULT_holding_time)
        self._tolerance = DoubleVar(value=settings.DEFAULT_temp_tolerance)
        self._max_stabilization_time = DoubleVar(value=settings.DEFAULT_max_stab_time)
        self._Tmin = DoubleVar(value=settings.DEFAULT_TEMPERATURE_MIN)
        self._Tmax = DoubleVar(value=settings.DEFAULT_TEMPERATURE_MAX)
        self._deltaT = DoubleVar(value=settings.DEFAULT_TEMPERATURE_STEP)
        self._temp_stages = StringVar(value="")
        self._RT_mode = 0

        self.check_stages_string = (self.parent.register(is_valid_stages), "%P")

    def ui_draw(self):
        RT_settings_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Настройки режима R(T)")
        RT_settings_frame.grid(padx=10, pady=5, row=2, column=1, sticky=tk.NSEW)
        tk.Label(RT_settings_frame, text="ПИД-настройки:").grid(row=1, column=1, columnspan=4, sticky=tk.W)
        tk.Label(RT_settings_frame, text="Kp:").grid(row=2, column=1)
        tk.Label(RT_settings_frame, text="Ki:").grid(row=2, column=3)
        tk.Label(RT_settings_frame, text="Kd:").grid(row=2, column=5)
        tk.Label(RT_settings_frame, text="dt:").grid(row=2, column=7)

        entry_Kp = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._kp,
            width=4,
        )
        entry_Kp.grid(padx=[2,6], row=2, column=2)
        entry_Ki = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._ki,
            width=4,
        )
        entry_Ki.grid(padx=[2,6], row=2, column=4)
        entry_Kd = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._kd,
            width=4,
        )
        entry_Kd.grid(padx=[2,0], row=2, column=6)
        entry_dt = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._dt,
            width=4,
        )
        entry_dt.grid(padx=[2, 0], row=2, column=8)

        tk.Label(RT_settings_frame, text="stab\ntime:").grid(row=3, column=1)
        tk.Label(RT_settings_frame, text="hold\ntime:").grid(row=3, column=3)
        tk.Label(RT_settings_frame, text="tol:").grid(row=3, column=5)
        tk.Label(RT_settings_frame, text="max stab\ntime:").grid(row=3, column=7)

        entry_stab_time = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._stabilization_time,
            width=4,
        )
        entry_stab_time.grid(padx=[2, 6], row=3, column=2)
        entry_hold_time = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._holding_time,
            width=4,
        )
        entry_hold_time.grid(padx=[2, 6], row=3, column=4)
        entry_tolerance = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._tolerance,
            width=4,
        )
        entry_tolerance.grid(padx=[2, 0], row=3, column=6)
        entry_max_stab_time = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._max_stabilization_time,
            width=4,
        )
        entry_max_stab_time.grid(padx=[2, 0], row=3, column=8)

        tk.Label(RT_settings_frame, text="Tmin:").grid(row=4, column=1)
        tk.Label(RT_settings_frame, text="Tmax:").grid(row=4, column=3)
        tk.Label(RT_settings_frame, text="deltaT:").grid(row=4, column=5)

        entry_Tmin = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._Tmin,
            width=4,
        )
        entry_Tmin.grid(padx=[2, 6], row=4, column=2)
        entry_Tmax = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._Tmax,
            width=4,
        )
        entry_Tmax.grid(padx=[2, 6], row=4, column=4)
        entry_deltaT = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._deltaT,
            width=4,
        )
        entry_deltaT.grid(padx=[2, 0], row=4, column=6)

        tk.Label(RT_settings_frame, text="stages:").grid(row=5, column=1)
        entry_temp_stages = tk.Entry(
            RT_settings_frame,
            validate="key",
            validatecommand=self.check_stages_string,
            textvariable=self._temp_stages,
            width=4,
        )
        entry_temp_stages.grid(padx=[2, 0], row=5, column=2, columnspan=4, sticky=tk.EW)

        tk.Label(RT_settings_frame, text="modes:").grid(row=6, column=1)
        # Словарь соответствия
        # RT_modes_options = {"Вариант 1": 0, "Вариант 2": 1, "Вариант 3": 2}
        self.RT_modes_combobox = ttk.Combobox(RT_settings_frame,
                                              values=["Idle mode", "Сustom stages mode",
                                                      "Gradual increasing mode", "Gradual decreasing mode",
                                                      "Reading average temperature", "Reading all temperatures"],
                                              state="readonly")
        self.RT_modes_combobox.current(0)
        self.RT_modes_combobox.grid(padx=[2, 0], row=6, column=2, columnspan=4)
        self.RT_modes_combobox.bind("<<ComboboxSelected>>", self.RT_mode_selected)

        self.apply_button = tk.Button(
            RT_settings_frame,
            text="Изменить",
            bg="yellow",
            command=self.change_RT_settings,
            bd=3)
        self.apply_button.grid(row=6, column=6, columnspan=3, sticky=tk.E, **common_grid_ui_params)

    def change_RT_settings(self):
        try:
            self.apply_button.config(bg="yellow")
            logger.info(f"RT parameters: {self.kp}, {self.ki}, {self.kd} ... ")
            self.method_to_update_status_bar(f"RT parameters: {self.kp}, {self.ki}, {self.kd} ... ")
            self.task_queue.put(SetRTParametersEvent(self.kp, self.ki, self.kd, self.dt, self.stab_time, self.hold_time,
                                                     self.temp_tolerance, self.max_stab_time, self.Tmin, self.Tmax,
                                                     self.deltaT, self.temp_stages, self.RT_mode))
        except tk.TclError:
            showerror(title='Ошибка', message='Некоторые настройки R(T) режима не заданы, перепроверьте их!')
            logger.error("Некоторые настройки R(T) режима не заданы")

    def update_button_color(self):
        self.apply_button.config(bg="light green")

    def RT_mode_selected(self, event):
        self._RT_mode = self.RT_modes_combobox.current()  # 0, 1, 2

    @property
    def pid_parameters(self):
        return self._kp.get(), self._ki.get(), self._kd.get()

    @property
    def kp(self):
        return self._kp.get()

    @property
    def ki(self):
        return self._ki.get()

    @property
    def kd(self):
        return self._kd.get()

    @property
    def dt(self):
        return self._dt.get()

    @property
    def stab_time(self):
        return self._stabilization_time.get()

    @property
    def hold_time(self):
        return self._holding_time.get()

    @property
    def temp_tolerance(self):
        return self._tolerance.get()

    @property
    def max_stab_time(self):
        return self._max_stabilization_time.get()

    @property
    def Tmin(self):
        return self._Tmin.get()

    @property
    def Tmax(self):
        return self._Tmax.get()

    @property
    def deltaT(self):
        return self._deltaT.get()

    @property
    def temp_stages(self):
        return self._temp_stages.get()

    @property
    def RT_mode(self):
        return self._RT_mode


"""
@property
    def RT_mode(self):
        return self._RT_mode.get()
values = ["Idle mode", "Сustom stages mode",
                  "Gradual increasing mode", "Gradual decreasing mode",
                  "Reading average temperature", "Reading all temperatures"])
"""

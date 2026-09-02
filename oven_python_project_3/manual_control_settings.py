from tkinter import StringVar, DoubleVar, BooleanVar
import tkinter as tk
from tkinter import ttk
from tkinter.messagebox import showerror
import logging

from PIL.ImageOps import expand

logger = logging.getLogger(__name__)

from events import StartManualTempEvent, StartManualVoltEvent, StopManualEvent

common_button_ui_params = {"font": "TkDefaultFont", "bd": 3}
common_grid_ui_params = dict(padx=10, pady=2)

class ManualControlSettings:
    def __init__(self, parent, floating_point_validation_function, task_queue, method_to_update_status_bar) -> None:
        self.parent = parent
        self.floating_point_validation_function = floating_point_validation_function
        self.task_queue = task_queue
        self.method_to_update_status_bar = method_to_update_status_bar
        self.selected_mode = StringVar(value="")
        self._T= DoubleVar(value=0)
        self._V = DoubleVar(value=0)

    def ui_draw(self):
        self.manual_control_settings_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Ручное управление")
        self.manual_control_settings_frame.grid(padx=10, pady=5, row=2, column=3, sticky=tk.NSEW)

        tk.Label(self.manual_control_settings_frame, text="T").grid(row=1, column=1)
        tk.Label(self.manual_control_settings_frame, text="V").grid(row=1, column=2)
        self.temp_radiobutton = ttk.Radiobutton(self.manual_control_settings_frame,
                                                value="T",
                                                variable=self.selected_mode)
        self.temp_radiobutton.grid(padx=[10,4], row=2, column=1)
        self.volt_radiobutton = ttk.Radiobutton(self.manual_control_settings_frame,
                                                value="V",
                                                variable=self.selected_mode)
        self.volt_radiobutton.grid(padx=[10,4], row=2, column=2)

        entry_T = tk.Entry(
            self.manual_control_settings_frame,
            justify="center",
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._T,
            width=5
        )
        entry_T.grid(padx=[2,2], row=3, column=1)
        entry_V = tk.Entry(
            self.manual_control_settings_frame,
            justify="center",
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._V,
            width=5
        )
        entry_V.grid(padx=[2,2], row=3, column=2)

        self.manual_start_button = tk.Button(
            self.manual_control_settings_frame,
            text="Запустить",
            bg="Green",
            command=self.start_measurement_in_manual_mode,
            **common_button_ui_params)
        self.manual_start_button.grid(row=2, column=3, **common_grid_ui_params)
        self.manual_stop_button = tk.Button(
            self.manual_control_settings_frame,
            text="Приостановить",
            bg="Red",
            command=self.stop_measurement_in_manual_mode,
            **common_button_ui_params)
        self.manual_stop_button.grid(row=3, column=3, **common_grid_ui_params)

    def lock_manual_mode(self):
        self.manual_control_settings_frame.config(font="TkDefaultFont")
        self.temp_radiobutton.config(state="disabled")
        self.volt_radiobutton.config(state="disabled")
        self.manual_start_button.config(state="disabled", disabledforeground="black")
        self.manual_stop_button.config(state="disabled", disabledforeground="black")

    def unlock_manual_mode(self):
        self.manual_control_settings_frame.config(font=("DejaVu Sans", 9, "bold"))
        self.temp_radiobutton.config(state="normal")
        self.volt_radiobutton.config(state="normal")
        self.manual_start_button.config(state="normal")
        self.manual_stop_button.config(state="normal")

    def start_measurement_in_manual_mode(self):
        try:
            if self.selected_mode.get() == "T":
                logger.info(f"Настройки ручного режима управления: T = {self.T}")
                self.task_queue.put(StartManualTempEvent(self.T))
            if self.selected_mode.get() == "V":
                logger.info(f"Настройки ручного режима управления: V ={self.V}")
                self.task_queue.put(StartManualVoltEvent(self.T))
            if self.selected_mode.get() == "":
                showerror(title='Ошибка', message='Не выбран режим съемки (T/V)')
                logger.error("Не выбран режим съемки (T/V)")
        except tk.TclError:
            showerror(title='Ошибка', message='Некоторые настройки ручного режима управления не заданы, перепроверьте их!')
            logger.error("Некоторые настройки ручного режима управления не заданы")

    def stop_measurement_in_manual_mode(self):
            self.task_queue.put(StopManualEvent())



    @property
    def all_manual_settings(self):
        return self._T.get(), self._V.get()

    @property
    def T(self):
        return self._T.get()

    @property
    def V(self):
        return self._V.get()

import tkinter as tk
from tkinter import ttk
from tkinter import StringVar

import logging
logger = logging.getLogger(__name__)

class RadiobuttonsForModes:
    def __init__(self, parent):
        self.parent = parent
        self.selected_measurement_mode = StringVar(value="")

    def ui_draw(self):
        mode_selection_frame = tk.Frame(self.parent)
        mode_selection_frame.grid(padx=10, pady=5, row=1, column=2, rowspan=2)

        ttk.Radiobutton(mode_selection_frame,
                        text="Scenario\nmode",
                        value="Scenario_mode",
                        variable=self.selected_measurement_mode,
                        command=self.measurement_mode_selection).grid(row=1, column=1)
        ttk.Radiobutton(mode_selection_frame,
                        text="Manual\nmode",
                        value="Manual_mode",
                        variable=self.selected_measurement_mode,
                        command=self.measurement_mode_selection).grid(row=2, column=1)

    def measurement_mode_selection(self):
        if self.selected_measurement_mode.get() == "Scenario_mode":
            logger.info("Выбран режим управления по сценарию")
            self.parent.scenario_control_settings.unlock_scenario_mode()
            self.parent.manual_control_settings.lock_manual_mode()
        if self.selected_measurement_mode.get() == "Manual_mode":
            logger.info("Выбран ручной режим управления")
            self.parent.scenario_control_settings.lock_scenario_mode()
            self.parent.manual_control_settings.unlock_manual_mode()

class RadiobuttonsForPlotting:
    def __init__(self, parent):
        self.parent = parent
        self.selected_plotting_mode = StringVar(value="T_t")

    def ui_draw(self, frame):
        plotting_modes_frame = tk.Frame(frame)
        plotting_modes_frame.grid(padx=10, pady=5, row=1, column=1, columnspan=2, sticky=tk.NSEW)

        tk.Label(plotting_modes_frame, text="Режимы построения графика:").grid(row=1, column=1, columnspan=4, sticky="w")
        ttk.Radiobutton(plotting_modes_frame,
                        text="T(t)",
                        value="T_t",
                        variable=self.selected_plotting_mode,
                        command=self.plotting_mode_selection).grid(row=2, column=1)
        ttk.Radiobutton(plotting_modes_frame,
                        text="R(t)",
                        value="R_t",
                        variable=self.selected_plotting_mode,
                        command=self.plotting_mode_selection).grid(row=2, column=2)
        ttk.Radiobutton(plotting_modes_frame,
                        text="R(T)",
                        value="R_T",
                        variable=self.selected_plotting_mode,
                        command=self.plotting_mode_selection).grid(row=2, column=3)
        ttk.Radiobutton(plotting_modes_frame,
                        text="R(V)",
                        value="R_V",
                        variable=self.selected_plotting_mode,
                        command=self.plotting_mode_selection).grid(row=2, column=4)

    def plotting_mode_selection(self):
        logger.info(f"Выбран {self.selected_plotting_mode.get()} режим построения графика")
        self.parent.update_axes(self.selected_plotting_mode.get())

    def get_current_plotting_mode(self):
        return self.selected_plotting_mode.get()
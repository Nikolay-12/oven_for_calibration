import  tkinter as tk
import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
import numpy as np
from data_processing import R_T_data_creation
from radiobuttons import RadiobuttonsForPlotting
from palette import MAIN_COLORS_WITH_DARK_COLORS

NUMBER_OF_THERMOCOUPLES = 25
NUMBER_OF_SENSORS = 16
common_label_ui_params = {"font": ('Arial', 12, 'bold'), "bd": 4}
common_button_ui_params = {"font": ('Arial', 10, 'bold'), "bd": 10}
common_graph_label_ui_params = {"fontsize": 8} #, fontweight="bold"
common_grid_ui_params = dict(padx=30, pady=5)

class RealTimePlotting:
    def __init__(self, parent):
        self.parent = parent
        self.connected_devices = None
        #self.fig, self.ax = plt.subplots()
        self.fig = Figure(figsize=(3, 3), dpi=100, layout="tight")
        self.ax = self.fig.add_subplot(1, 1, 1)
        self.canvas = None
        self.all_data = np.empty((0, 9), dtype=float)

    def ui_draw(self):
        real_time_plotting_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Построение графиков")
        real_time_plotting_frame.grid(padx=10, pady=5, row=2, column=2, sticky=tk.NSEW)

        self.radiobuttons = RadiobuttonsForPlotting(self)
        self.radiobuttons.ui_draw(real_time_plotting_frame)

        self.ax.set_xlabel('Время (с)', **common_graph_label_ui_params)
        self.ax.set_ylabel('Температура (\u00b0С)', **common_graph_label_ui_params)
        self.ax.set_title("График T(t) из COM-порта", fontsize=8, fontweight='bold')

        self.canvas = FigureCanvasTkAgg(self.fig, master=real_time_plotting_frame)
        self.canvas.draw()
        self.canvas.get_tk_widget().grid(row=3, column=1, columnspan=2, sticky=tk.EW)
        toolbar = NavigationToolbar2Tk(self.canvas, real_time_plotting_frame, pack_toolbar=False)
        toolbar.update()
        toolbar.grid(row=4, column=1, columnspan=2, sticky=tk.EW)

    def clear_lines(self):
        for line in self.ax.lines:
            line.remove()

    def update_plot(self, data_array):
        self.connected_devices = self.parent.list_of_used_devices.connected_devices
        self.current_plotting_mode = self.radiobuttons.get_current_plotting_mode()
        self.clear_lines()
        self.all_data = np.append(self.all_data, [data_array], axis=0)
        if self.current_plotting_mode == "T_t":
            for i in range(1, NUMBER_OF_THERMOCOUPLES + 1):
                for device_name in self.connected_devices:
                    if device_name == f"T{i}":
                        self.ax.plot(self.all_data[:, 0],
                                     self.all_data[:, i],
                                     lw=2,
                                     c=MAIN_COLORS_WITH_DARK_COLORS[i - 1])
        if self.current_plotting_mode == "R_t":
            for i in range(1, NUMBER_OF_SENSORS + 1):
                for device_name in self.connected_devices:
                    if device_name == f"R{i}":
                        self.ax.plot(self.all_data[:, 0],
                                     self.all_data[:, i + NUMBER_OF_THERMOCOUPLES],
                                     lw=2,
                                     c=MAIN_COLORS_WITH_DARK_COLORS[i - 1])
        if self.current_plotting_mode == "R_T":
            new_data = R_T_data_creation(self.all_data)
            for i in range(1, NUMBER_OF_SENSORS + 1):
                for device_name in self.connected_devices:
                    if device_name == f"R{i}":
                        self.ax.plot(new_data[:, i + NUMBER_OF_THERMOCOUPLES + NUMBER_OF_SENSORS],
                                     new_data[:, i + NUMBER_OF_THERMOCOUPLES],
                                     lw=2,
                                     c=MAIN_COLORS_WITH_DARK_COLORS[i - 1])
        if self.current_plotting_mode == "R_V":
            pass
        self.ax.relim()
        self.ax.autoscale_view()
        self.fig.canvas.draw()
        self.fig.canvas.flush_events()

    def update_axes(self, plotting_mode):
        if plotting_mode == "T_t":
            self.ax.set_xlabel('Время (с)', **common_graph_label_ui_params)
            self.ax.set_ylabel('Температура (\u00b0С)', **common_graph_label_ui_params)
            self.ax.set_title("График T(t) из COM-порта", fontsize=8, fontweight='bold')
            self.fig.canvas.draw()
        if plotting_mode == "R_t":
            self.ax.set_xlabel('Время (с)', **common_graph_label_ui_params)
            self.ax.set_ylabel('Сопротивление (Ом)', **common_graph_label_ui_params)
            self.ax.set_title("График R(t) из COM-порта", fontsize=8, fontweight='bold')
            self.fig.canvas.draw()
        if plotting_mode == "R_T":
            self.ax.set_xlabel('Температура (\u00b0С)', **common_graph_label_ui_params)
            self.ax.set_ylabel('Сопротивление (Ом)', **common_graph_label_ui_params)
            self.ax.set_title("График R(T) из COM-порта", fontsize=8, fontweight='bold')
            self.fig.canvas.draw()
        if plotting_mode == "R_V":
            self.ax.set_xlabel('Напряжение (В)', **common_graph_label_ui_params)
            self.ax.set_ylabel('Сопротивление (Ом)', **common_graph_label_ui_params)
            self.ax.set_title("График R(V) из COM-порта", fontsize=8, fontweight='bold')
            self.fig.canvas.draw()
import tkinter as tk
from tkinter import StringVar, IntVar
from tkinter.messagebox import showerror
from tkinter import filedialog
import logging
logger = logging.getLogger(__name__)

import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
import re
import numpy as np
from numpy.polynomial import Polynomial

class Plotting:
    def __init__(self, parent, floating_point_validation_function, task_queue, method_to_update_status_bar):
        self.parent = parent
        self.floating_point_validation_function = floating_point_validation_function
        self.task_queue = task_queue
        self.method_to_update_status_bar = method_to_update_status_bar
        self.ax = None
        self.canvas = None
        self.x_data = []
        self.y_data = []
        self._file_address_for_plotting = StringVar(value='')
        self._command_line = StringVar(value='')
        self._line_counter = IntVar(value=0)
        self._numbers = []
        self._polynomial_degree = IntVar(value=1)

    def ui_draw(self):
        plotting_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Аппроксимация данных")
        plotting_frame.grid(padx=10, pady=5, row=1, column=2, rowspan=3, sticky=tk.NSEW)

        tk.Label(plotting_frame, text="Построить\nграфик:").grid(row=1, column=1)
        tk.Button(
            plotting_frame,
            text="Построить",
            command=self.read_and_plot,
            bd=3).grid(padx=[2,6], row=1, column=2)

        tk.Label(plotting_frame, text="Степень\nмногочлена:").grid(row=1, column=3)
        tk.Entry(
            plotting_frame,
            justify="center",
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._polynomial_degree,
            width=3).grid(padx=[2,6], row=1, column=4)
        tk.Button(
            plotting_frame,
            text="Аппроксимировать",
            command=self.approximate,
            bd=3).grid(padx=[2,0], row=1, column=5)


        tk.Label(plotting_frame, text="Выбранный\nфайл:").grid(row=2, column=1)
        tk.Entry(
            plotting_frame,
            text="Построить",
            state='disabled',
            textvariable=self._file_address_for_plotting).grid(padx=[2,0], row=2, column=2, columnspan=4, sticky=tk.EW)

        fig = Figure(figsize=(3, 3), dpi=100, layout="tight")
        self.ax = fig.add_subplot(1, 1, 1)
        self.ax.set_xlabel('Ось х')
        self.ax.set_ylabel('Ось y')
        self.ax.set_title('График', fontsize = 11)
        self.canvas = FigureCanvasTkAgg(fig, master=plotting_frame)
        self.canvas.get_tk_widget().grid(row=4, column=1, columnspan=5)
        toolbar = NavigationToolbar2Tk(self.canvas, plotting_frame, pack_toolbar=False)
        toolbar.update()
        toolbar.grid(row=5, column=1, columnspan=5, sticky=tk.W)

    def read_and_plot(self):
        self.task_queue.put("read_and_plot")
    def approximate(self):
        self.task_queue.put("approximate")

    def read_and_plot_with_queue(self):
        #self.ax.clear
        file_path = filedialog.askopenfilename()
        self._file_address_for_plotting.set(file_path)
        try:
            f = open(file_path, 'r', encoding='cp1252')
            self.x_data, self.y_data = [], []
            self._command_line.set(value='')
            self._line_counter.set(value=0)
            for line in f:
                self.numbers = re.findall(r'\b\d+\b', line)
                if (len(self.numbers) != 2):
                    showerror(title="Ошибка", message="Выбранный файл не имеет нужный формат (2 числа в каждой строке)")
                    self.x_data, self.y_data = [], []
                    logger.error("Выбранный файл не имеет нужный формат (2 числа в каждой строке)")
                    break
                else:
                    self._command_line.set(self.command_line + line.strip())
                    self._line_counter.set(self.line_counter + 1)
                    self.x_data.append(float(self.numbers[0]))
                    self.y_data.append(float(self.numbers[1]))
            f.close()
            logger.info("Данные из файла", self.file_address_for_plotting, ": ", self.command_line)
            logger.info("Массив x:", self.x_data, "\nМассив y:", self.y_data)
            #print("Данные из файла", self.file_address_for_plotting, ": ", self.command_line, "\nКоличество команд:", self.line_counter)
        except FileNotFoundError:
            showerror(title='Ошибка', message='Файл не выбран!')
            logger.error("Не выбран файл для построения графика")

        self.refresh_figure(self.x_data, self.y_data)

    def refresh_figure(self, X, Y):
        #self.ax.update()
        self.ax.clear()
        self.ax.scatter(X, Y, label="Данные", color="blue", s=10)
        self.ax.set_title("Сырые данные", fontsize=11)
        self.ax.set_xlabel('Ось х')
        self.ax.set_ylabel('Ось y')
        self.canvas.draw()

    def approximate_with_queue(self):
        xh = np.linspace(min(self.x_data), max(self.x_data), 100)
        Color = ['red', 'orange', 'yellow', 'green', 'purple']
        coefficients = np.polyfit(self.x_data, self.y_data, self.polynomial_degree)[::-1]
        approximation = Polynomial(coefficients)
        self.ax.plot(xh, approximation(xh), label=f"n = {self.polynomial_degree}", color=Color[self.polynomial_degree - 1])
        self.ax.set_title("Полиномиальная аппроксимация", fontsize = 11)
        self.ax.legend()
        self.canvas.draw()
        logger.info("Коэффициенты:", coefficients)
        logger.info(f"Аппроксимирующий полином: f(x) = {approximation}")

    @property
    def file_address_for_plotting(self):
        return self._file_address_for_plotting.get()

    @property
    def command_line(self):
        return self._command_line.get()

    @property
    def line_counter(self):
        return self._line_counter.get()

    @property
    def polynomial_degree(self):
        return self._polynomial_degree.get()

import  tkinter as tk
from tkinter.ttk import Checkbutton
COLOURS = ["#0c0786", "#4a02a0", "#6c00a8", "#99149f", "#b83289",
            "#d7566c", "#ea7455", "#f8963f", "#f5e726", "#eff821"]
DEFAULT_COLOUR = "#f0f0f0"

class Thermocouple(tk.Frame):
    def __init__(self, number,  master=None, **kwargs):
        super().__init__(master, **kwargs)
        self.name = "T{}".format(number)
        self.data = tk.StringVar(value='')
        self.label = tk.Label(self, text=self.name)
        self.entry = tk.Entry(self,
                              state='disabled',
                              justify="center",
                              textvariable=self.data,
                              width=5)
        self.label.pack()
        self.entry.pack()

    def set_label_color_for_chosen_thermocouple(self):
        self.label.config(fg='blue', font=('Arial', 9, 'bold'))
    def set_default_label_color_of_thermocouple(self):
        self.label.config(fg='black', font='TkDefaultFont')

    def update_entry_color_from_temp_point(self, temp_point):
        self.entry.config(disabledbackground=COLOURS[int(temp_point // 50)])
    def set_default_entry_color(self):
        self.entry.config(disabledbackground=DEFAULT_COLOUR)


class Sensor(tk.Frame):
    def __init__(self, number,  master=None, **kwargs):
        super().__init__(master, **kwargs)
        self.name = "R{}".format(number)
        self.data = tk.StringVar(value='')
        self.label = tk.Label(self, text=self.name)
        self.entry = tk.Entry(self,
                              state='disabled',
                              justify="center",
                              textvariable=self.data,
                              width=5)
        self.label.pack()
        self.entry.pack()

    def set_label_color_for_chosen_sensor(self):
        self.label.config(fg='blue', font=('Arial', 9, 'bold'))
    def set_default_label_color_of_sensor(self):
        self.label.config(fg='black', font='TkDefaultFont')

class ColorbarElement(tk.Frame):
    def __init__(self, number, master=None, **kwargs):
        super().__init__(master, **kwargs)
        LABELS_FOR_COLORBAR = ["[15,50)", "[50-100)", "[100-150)", "[150-200)", "[200-250)",
                           "[250-300)", "[300-350)", "[350-400)", "[400-450)", "[450-500)"]

        tk.Entry(self, state='disabled',
                 disabledbackground=COLOURS[number], width=5).grid(row=number, column=1, padx=[15,0])
        tk.Label(self, text=LABELS_FOR_COLORBAR[number], width=8).grid(row=number, column=2)

class ThermocoupleCheckButton(tk.Frame):
    def __init__(self, number,  master=None, **kwargs):
        super().__init__(master, **kwargs)
        self.name = "T{}".format(number)
        self.data = tk.BooleanVar(value=False)
        self.label = tk.Label(self, text=self.name)
        self.checkbutton = Checkbutton(self, variable=self.data)
        self.label.pack()
        self.checkbutton.pack()

class SensorCheckButton(tk.Frame):
    def __init__(self, number,  master=None, **kwargs):
        super().__init__(master, **kwargs)
        self.name = "R{}".format(number)
        self.data = tk.BooleanVar(value=False)
        self.label = tk.Label(self, text=self.name)
        self.checkbutton = Checkbutton(self, variable=self.data)
        self.label.pack()
        self.checkbutton.pack()
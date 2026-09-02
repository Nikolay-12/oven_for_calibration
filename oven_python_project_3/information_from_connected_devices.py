import tkinter as tk
from tkinter import StringVar

Number_of_thermocouples = 9
Number_of_sensors = 16

class InformationFromConnectedDevices:
    def __init__(self, parent):
        self.parent = parent
        self.connected_devices = None

        self._selected_sensors_label = dict()
        self._selected_thermocouples_label = dict()
        self._selected_sensors_label_var = dict()
        self. _selected_thermocouples_label_var = dict()
        self._selected_sensors_data = dict()
        self._selected_thermocouples_data = dict()
        for i in range(1, Number_of_sensors + 1):
            self._selected_sensors_label[i] = StringVar(value="R%i" % i)
            self._selected_sensors_data[i] = StringVar(value="")
        for i in range(1, Number_of_thermocouples + 1):
            self._selected_thermocouples_label[i] = StringVar(value="T%i" % i)
            self._selected_thermocouples_data[i] = StringVar(value="")

    def ui_draw(self):
        information_from_serial_port_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Данные с сенсоров и термопар")
        information_from_serial_port_frame.grid(padx=10, pady=5, row=1, column=3, rowspan=4, sticky=tk.NSEW)

        for i in range(1, Number_of_thermocouples + 1):
            self._selected_thermocouples_label_var[i] = tk.Label(
                information_from_serial_port_frame,
                textvariable=self._selected_thermocouples_label[i])
            self._selected_thermocouples_label_var[i].grid(row=4*((i-1)//3)+3, column=2*((i-1)%3)+2)
            tk.Entry(
                information_from_serial_port_frame,
                state='disabled',
                textvariable=self._selected_thermocouples_data[i],
                width=5).grid(row=4*((i-1)//3)+4, column=2*((i-1)%3)+2)
        for i in range(1, Number_of_sensors + 1):
            self._selected_sensors_label_var[i] = tk.Label(
                information_from_serial_port_frame,
                textvariable=self._selected_sensors_label[i])
            self._selected_sensors_label_var[i].grid(row=4*((i-1)//4)+1, column=2*((i-1)%4)+1)
            tk.Entry(
                information_from_serial_port_frame,
                state='disabled',
                textvariable=self._selected_sensors_data[i],
                width=5).grid(row=4*((i-1)//4)+2, column=2*((i-1)%4)+1)

    def update_color(self):
        self.connected_devices = self.parent.list_of_used_devices.connected_devices
        for i in range(1, Number_of_thermocouples + 1):
            self._selected_thermocouples_label_var[i].config(fg='black', font='TkDefaultFont')
            if self._selected_thermocouples_label[i].get() in self.connected_devices:
                self._selected_thermocouples_label_var[i].config(fg='blue', font=('Arial', 9, 'bold'))
        for i in range(1, Number_of_sensors + 1):
            self._selected_sensors_label_var[i].config(fg='black', font='TkDefaultFont')
            if self._selected_sensors_label[i].get() in self.connected_devices:
                self._selected_sensors_label_var[i].config(fg='blue', font=('Arial', 9, 'bold'))

    def update_data(self, data_array):
        self.connected_devices = self.parent.list_of_used_devices.connected_devices
        for i in range(1, Number_of_thermocouples + 1):
            if self._selected_thermocouples_label[i].get() in self.connected_devices:
                self._selected_thermocouples_data[i].set(data_array[i-1])
        for i in range(1, Number_of_sensors + 1):
            if self._selected_sensors_label[i].get() in self.connected_devices:
                self._selected_sensors_data[i].set(data_array[i + Number_of_thermocouples - 1])

    def testing_of_data_updating(self, data_array):
        self._selected_thermocouples_data[1].set(data_array[3])
        self._selected_sensors_data[1].set(data_array[5])

    @property
    def selected_sensors_data(self):
        return self._selected_sensors_data

    @property
    def selected_thermocouples_data(self):
        return self._selected_thermocouples_data
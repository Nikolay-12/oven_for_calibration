import tkinter as tk
from tkinter import StringVar
from devices_frames import Thermocouple, Sensor, ColorbarElement
from dataclasses import dataclass

NUMBER_OF_THERMOCOUPLES = 25
NUMBER_OF_SENSORS = 16

@dataclass()
class Point():
    row: int
    column: int

THERMOCOUPLES_POSITIONS = [
    Point(1, 2),
    Point(1, 4),
    Point(1, 6),
    Point(1, 8),
    Point(2, 1),
    Point(2, 9),
    Point(3, 3),
    Point(3, 5),
    Point(3, 7),
    Point(4, 1),
    Point(4, 9),
    Point(5, 3),
    Point(5, 5),
    Point(5, 7),
    Point(6, 1),
    Point(6, 9),
    Point(7, 3),
    Point(7, 5),
    Point(7, 7),
    Point(8, 1),
    Point(8, 9),
    Point(9, 2),
    Point(9, 4),
    Point(9, 6),
    Point(9, 8),
]

class InformationFromConnectedDevices:
    def __init__(self, parent):
        self.parent = parent
        self.connected_devices = None

    def ui_draw(self):
        information_from_serial_port_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Данные с термопар и сенсоров")
        information_from_serial_port_frame.grid(padx=10, pady=5, row=2, column=1, sticky=tk.NSEW)

        self.draw_grid_frame(information_from_serial_port_frame)
        self.draw_colorbar_frame(information_from_serial_port_frame)

    def draw_grid_frame(self, parent):
        grid_frame = tk.Frame(parent)
        grid_frame.pack(anchor="n", side="left")
        self.thermocouples = [Thermocouple(i, master=grid_frame) for i in range(1, NUMBER_OF_THERMOCOUPLES + 1)]
        self.sensors = [Sensor(i, master=grid_frame) for i in range(1, NUMBER_OF_SENSORS + 1)]
        self.draw_thermocouples()
        self.draw_sensors()

    def draw_thermocouples(self):
        for position, thermocouple in zip(THERMOCOUPLES_POSITIONS, self.thermocouples):
            thermocouple.grid(row = position.row, column = position.column)

    def draw_sensors(self):
        for position, sensor in enumerate(self.sensors):
            sensor.grid(row = 2*(position//4) + 2, column = 2*(position%4) + 2)

    def draw_colorbar_frame(self, parent):
        colorbar_frame = tk.Frame(parent)
        colorbar_frame.pack(anchor="n", side="left")

        tk.Label(colorbar_frame, text="colour").grid(row=1, column=1, padx=[5,0])
        tk.Label(colorbar_frame, text="T (\u00b0С)").grid(row=1, column=2, sticky=tk.W)
        self.colorbar = [ColorbarElement(i, master=colorbar_frame) for i in range(10)]
        self.draw_colorbar()

    def draw_colorbar(self):
        for position, colorbar_element in enumerate(self.colorbar):
            colorbar_element.grid(row = position + 2, column = 1, columnspan=2)

    def update_label_color(self):
        self.connected_devices = self.parent.list_of_used_devices.connected_devices
        for idx, thermocouple in enumerate(self.thermocouples):
            thermocouple.set_default_label_color_of_thermocouple()
            if thermocouple.name in self.connected_devices:
                thermocouple.set_label_color_for_chosen_thermocouple()
        for idx, sensor in enumerate(self.sensors):
            sensor.set_default_label_color_of_sensor()
            if sensor.name in self.connected_devices:
                sensor.set_label_color_for_chosen_sensor()

    def update_data(self, data_array):
        self.connected_devices = self.parent.list_of_used_devices.connected_devices
        for idx, thermocouple in enumerate(self.thermocouples):
            thermocouple.data.set("")
            thermocouple.set_default_entry_color()
            if thermocouple.name in self.connected_devices:
                thermocouple.data.set(data_array[idx])
                if (data_array[idx]!="nan") and (15<=float(data_array[idx])<=500):
                    thermocouple.update_entry_color_from_temp_point(float(data_array[idx]))

        for idx, sensor in enumerate(self.sensors):
            sensor.data.set("")
            if sensor.name in self.connected_devices:
                sensor.data.set(data_array[idx + NUMBER_OF_THERMOCOUPLES - 1])

    def testing_of_data_updating(self, data_array):
        self.thermocouples[1].data.set(data_array[3])
        self.sensors[1].data.set(data_array[5])

    @property
    def selected_thermocouples_data(self):
        self.connected_devices = self.parent.list_of_used_devices.connected_devices
        temperatures = []
        for idx, thermocouple in enumerate(self.thermocouples):
            if thermocouple.name in self.connected_devices:
                temperatures.append(thermocouple.data)
        return temperatures

    @property
    def selected_sensors_data(self):
        self.connected_devices = self.parent.list_of_used_devices.connected_devices
        resistances = []
        for idx, sensor in enumerate(self.sensors):
            if sensor.name in self.connected_devices:
                resistances.append(sensor.data)
        return resistances

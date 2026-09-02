import tkinter as tk
from tkinter.messagebox import showerror

from devices_frames import ThermocoupleCheckButton, SensorCheckButton
from events import SelectingDevices

import logging
logger = logging.getLogger(__name__)

NUMBER_OF_THERMOCOUPLES = 25
NUMBER_OF_SENSORS = 16

class ListOfConnectedDevices:
    def __init__(self, parent, task_queue, method_to_update_status_bar) -> None:
        self.parent = parent
        self.task_queue = task_queue
        self.method_to_update_status_bar = method_to_update_status_bar
        self._connected_devices = []

    def ui_draw(self):
        connected_devices_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Подключенные термопары и сенсоры")
        connected_devices_frame.grid(padx=10, pady=5, row=1, column=1, rowspan=1, sticky=tk.NSEW)

        self.draw_thermocouples_checkbuttons_frame(connected_devices_frame)
        self.draw_sensors_checkbuttons_frame(connected_devices_frame)

    def confirm_selected_devices(self):
        self.apply_button.config(bg="yellow")
        self._connected_devices = []
        for idx, thermocouple in enumerate(self.thermocouples_checkbuttons):
            if thermocouple.data.get():
                self._connected_devices.append(thermocouple.name)
        for idx, sensor in enumerate(self.sensors_checkbuttons):
            if sensor.data.get():
                self._connected_devices.append(sensor.name)
        if self.connected_devices:
            logger.info(f"Connected devices:{self.connected_devices}")
            self.method_to_update_status_bar(f"Connected devices:{self.connected_devices}")
            #self.task_queue.put(SelectingDevices(self.connected_devices))
            self.update_button_color()
            self.parent.info_from_used_devices.update_label_color()
        else:
            showerror(title='Ошибка', message='Не выбраны устройства!')
            logger.error("Не выбраны устройства!")

    def select_all_devices(self):
        for idx, thermocouple in enumerate(self.thermocouples_checkbuttons):
            thermocouple.data.set(True)
        for idx, sensor in enumerate(self.sensors_checkbuttons):
            sensor.data.set(True)
        logger.info("All devices were chosen")
        self.method_to_update_status_bar("All devices were chosen")
        #self.task_queue.put(SelectingDevices(self.connected_devices))
        self.parent.info_from_used_devices.update_label_color()

    def draw_thermocouples_checkbuttons_frame(self, parent):
        thermocouples_checkbuttons_frame = tk.Frame(parent)
        thermocouples_checkbuttons_frame.pack(anchor="n") # .grid(row=1, column=1)
        self.thermocouples_checkbuttons = [ThermocoupleCheckButton(i, master=thermocouples_checkbuttons_frame)
                                           for i in range(1, NUMBER_OF_THERMOCOUPLES + 1)]
        self.draw_thermocouples_checkbuttons()
    def draw_thermocouples_checkbuttons(self):
        for position, thermocouple in enumerate(self.thermocouples_checkbuttons):
            thermocouple.grid(row=1, column=position+1) #row=position//10+1, column=position%10+1

    def draw_sensors_checkbuttons_frame(self, parent):
        sensors_checkbuttons_frame = tk.Frame(parent)
        sensors_checkbuttons_frame.pack(anchor="n") # .grid(row=2, column=1)
        self.sensors_checkbuttons = [SensorCheckButton(i, master=sensors_checkbuttons_frame)
                                     for i in range(1, NUMBER_OF_SENSORS + 1)]
        self.draw_sensors_checkbuttons()
        self.select_all_button = tk.Button(
            sensors_checkbuttons_frame,
            text="Выбрать\nвсех",
            bg="grey",
            command=self.select_all_devices,
            bd=3)
        self.select_all_button.grid(padx=10, pady=[4, 0], row=2, column=17, columnspan=4)
        self.apply_button = tk.Button(
            sensors_checkbuttons_frame,
            text="Подтвердить",
            bg="yellow",
            command=self.confirm_selected_devices,
            bd=3)
        self.apply_button.grid(padx=10, pady=[4,0], row=2, column=22, columnspan=4)

    def draw_sensors_checkbuttons(self):
        for position, sensor in enumerate(self.sensors_checkbuttons):
            sensor.grid(row=2, column=position+1) #row=position//10+4, column=position%10+1

    def update_button_color(self):
        self.apply_button.config(bg="light green")

    @property
    def connected_devices(self):
        return self._connected_devices
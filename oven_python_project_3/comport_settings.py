import tkinter as tk
import tkinter.ttk as ttk

from events import ComportSettingsEvent
from utils.comports import get_comports

import logging
logger = logging.getLogger(__name__)
#logging.basicConfig(level=logging.DEBUG)
DEFAULT_BAUDRATE = 9600

common_button_ui_params = {"font": "TkDefaultFont", "bd": 3}
common_grid_ui_params = dict(padx=10, pady=5)

class ComportException(Exception):
    def __init__(self, *args: object) -> None:
        super().__init__(*args)

class ComportSettings:
    def __init__(self, parent, task_queue, method_to_update_status_bar) -> None:
        self.parent = parent
        self.task_queue = task_queue
        self.method_to_update_status_bar = method_to_update_status_bar
        self._port = None
        self._baudrate = DEFAULT_BAUDRATE

    def ui_draw(self):
        serial_settings_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Настройки COM-порта")
        serial_settings_frame.grid(padx=10, pady=5, row=1, column=1, sticky=tk.NW)

        tk.Label(serial_settings_frame, text="COM-порт:").grid(row=1, column=1)

        self._comports_listbox = tk.Listbox(serial_settings_frame, width=18, height=2, selectmode='SINGLE')
        self.refresh_comports()

        tk.Label(serial_settings_frame, text="Скорость обмена\n данных по порту:").grid(padx=[3,0], row=2, column=1)

        self._baudrate_combobox = ttk.Combobox(serial_settings_frame, width=15, state="readonly")
        self._baudrate_combobox['values'] = (4800, 9600, 19200, 38400)
        self._baudrate_combobox.current(1)
        self._baudrate_combobox.grid(padx=[3,0], row=2, column=2)

        self.apply_button = tk.Button(serial_settings_frame,
                  text="Подтвердить",
                  bg="yellow",
                  command=self.choose_comport_and_baudrate,
                  **common_button_ui_params)

        self.apply_button.grid(row=3, column=1, **common_grid_ui_params)

        tk.Button(serial_settings_frame,
                  text="Обновить",
                  command=self.refresh_comports,
                  **common_button_ui_params).grid(row=3, column=2, **common_grid_ui_params)

    def choose_comport_and_baudrate(self):
        self.apply_button.config(bg="yellow")
        comports_chosen_indexes = self._comports_listbox.curselection()
        if len(comports_chosen_indexes) > 0:
            self._port = self._comports_listbox.get(comports_chosen_indexes[0])
        self._baudrate = self._baudrate_combobox.get()

        if self._port is not None:
            logger.info(f"Serial port settings: port:{self._port}; baudrate:{self._baudrate}")
            self.method_to_update_status_bar(f"Serial port settings: port:{self._port}; baudrate:{self._baudrate}")
            #self.parent.parent.statusbar_info.set(f"Информация по настройке: Порт:{self._port} Baudrate:{self._baudrate}")
            self.task_queue.put(ComportSettingsEvent(self._port, self._baudrate))
        else:
            self.method_to_update_status_bar("Порт не установлен!")
            raise ComportException("Порт не установлен")

    def refresh_comports(self):
        len_of_comports_listbox = self._comports_listbox.size()
        self._comports_listbox.delete(0, len_of_comports_listbox - 1)
        for comport in get_comports():
            self._comports_listbox.insert(tk.END, comport)
        self._comports_listbox.grid(row=1, column=2, sticky=tk.N)

    def update_button_color(self):
        self.apply_button.config(bg="light green")


    @property
    def port_and_baudrate(self):
        return self._port, self._baudrate

    @property
    def port(self):
        if self._port is None:
            raise ComportException("Порт не установлен")
        return self._port
    
    @property
    def baudrate(self):
        if self._baudrate is None:
            raise ComportException("baudrate не установлен")
        return self._baudrate


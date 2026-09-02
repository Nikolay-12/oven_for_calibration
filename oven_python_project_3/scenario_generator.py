from tkinter import DoubleVar, StringVar
import tkinter as tk
from tkinter.messagebox import showerror
from tkinter import filedialog
import logging
logger = logging.getLogger(__name__)

from events import ScenarioGenerationEvent

common_button_ui_params = {"font": "TkDefaultFont", "bd": 3}
common_grid_ui_params = dict(padx=10, pady=5)

class ScenarioGenerator:
    def __init__(self, parent, floating_point_validation_function, task_queue, method_to_update_status_bar) -> None:
        self.parent = parent
        self.floating_point_validation_function = floating_point_validation_function
        self.task_queue = task_queue
        self.method_to_update_status_bar = method_to_update_status_bar
        self._scenario_file_path = StringVar(value='')
        self._Tmin = DoubleVar(value=0)
        self._Tmax = DoubleVar(value=0)
        self._delta_T = DoubleVar(value=0)
        self._delta_t = DoubleVar(value=0)
        self._number_of_values_for_averaging = DoubleVar(value=0)
        self._data_reading_frequency = DoubleVar(value=0)

    def ui_draw(self):
        scenario_generator_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Генератор сценариев")
        scenario_generator_frame.grid(padx=10, pady=5, row=1, column=1, sticky=tk.NSEW)

        tk.Label(scenario_generator_frame, text="Выбрать\nдиректорию:").grid(row=1, column=1)
        tk.Button(
            scenario_generator_frame,
            text="Выбрать",
            command=self.choose_directory,
            bd=3).grid(row=1, column=2, columnspan=2, sticky=tk.W)

        tk.Label(scenario_generator_frame, text="Адрес файла:").grid(row=2, column=1)
        tk.Entry(
            scenario_generator_frame,
            state='disabled',
            textvariable=self._scenario_file_path).grid(padx=2, pady=[0,4], row=2, column=2, columnspan=5, sticky=tk.EW)

        tk.Label(scenario_generator_frame, text="T_min:").grid(row=3, column=1, sticky="e")
        tk.Label(scenario_generator_frame, text="T_max:").grid(row=3, column=3, sticky="e")
        tk.Label(scenario_generator_frame, text="ΔT:").grid(row=3, column=5, sticky="e")
        tk.Label(scenario_generator_frame, text="Δt:").grid(row=4, column=1, sticky="e")
        tk.Label(scenario_generator_frame, text="<...>n:").grid(row=4, column=3, sticky="e")
        tk.Label(scenario_generator_frame, text="Δt_R&T:").grid(row=4, column=5, sticky="e")
        # <...>n -- к-во съемок\n для усреднения
        # Δt_R&T -- периодичность\n съемки R и T

        entry_T_min = tk.Entry(scenario_generator_frame,
                               validate="key",
                               validatecommand=self.floating_point_validation_function,
                               textvariable=self._Tmin,
                               width=5)
        entry_T_min.grid(padx=[2,3], row=3, column=2, sticky=tk.W)
        entry_T_max = tk.Entry(scenario_generator_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._Tmax,
            width=5)
        entry_T_max.grid(padx=[2,3], row=3, column=4, sticky=tk.W)
        entry_delta_T = tk.Entry(scenario_generator_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._delta_T,
            width=5)
        entry_delta_T.grid(padx=[2,0], row=3, column=6, sticky=tk.W)
        entry_delta_t = tk.Entry(scenario_generator_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._delta_t,
            width=5)
        entry_delta_t.grid(padx=[2,3], row=4, column=2, sticky=tk.W)
        entry_number_of_values_for_averaging = tk.Entry(scenario_generator_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._number_of_values_for_averaging,
            width=5)
        entry_number_of_values_for_averaging.grid(padx=[2,3], row=4, column=4, sticky=tk.W)
        entry_data_reading_frequency = tk.Entry(scenario_generator_frame,
            validate="key",
            validatecommand=self.floating_point_validation_function,
            textvariable=self._data_reading_frequency,
            width=5)
        entry_data_reading_frequency.grid(padx=[2,0], row=4, column=6, sticky=tk.W)

        tk.Button(scenario_generator_frame,
            text="Сгенерировать",
            bg="Green",
            command=self.generate_scenario,
            bd=3).grid(pady=5, row=5, column=4, columnspan=3, sticky=tk.E)

    def choose_directory(self):
        file_path_for_saving = filedialog.asksaveasfilename(initialfile="Untitled.txt",
                                                            filetypes=[("Text files", "*.txt")])
        self._scenario_file_path.set(file_path_for_saving)
        self.method_to_update_status_bar(f"Selected file path: {file_path_for_saving}")

    def generate_scenario(self):
        try:
            logger.info(f"Scenario generator settings: T_min = {self.Tmin}; T_max = {self.Tmax}; ΔT = {self.delta_T}"
                        f"; Δt ={self.delta_t}; <...>n = {self.number_of_values_for_averaging}"
                        f"; Δt_R&T = {self.data_reading_frequency}")
            self.task_queue.put(ScenarioGenerationEvent(self.scenario_file_path, self.Tmin, self.Tmax, self.delta_T, self.delta_t,
                                                 self.number_of_values_for_averaging,
                                                 self.data_reading_frequency))
        except tk.TclError:
            showerror(title='Ошибка', message='Некоторые настройки генератора сценария не заданы, перепроверьте их!')
            logger.error("Некоторые настройки генератора сценария не заданы")

    @staticmethod
    def scenario_creation(file_path, Tmin, Tmax, delta_T, delta_t, num_of_val_for_averaging,data_reading_frequency):
        try:
            with open(file_path, 'w+', encoding="utf-8") as file:
                file.write(f"Scenario generator settings: T_min = {Tmin}; T_max = {Tmax}; ΔT = {delta_T}"
                        f"; Δt ={delta_t}; <...>n = {num_of_val_for_averaging}"
                        f"; Δt_R&T = {data_reading_frequency}")
                file.close()
        except FileNotFoundError:
            showerror(title="Ошибка", message="Директория не выбрана")
            logger.error("Директория не выбрана")

    @property
    def all_scenario_generator_settings(self):
        return self._scenario_file_path, self._Tmin.get(), self._Tmax.get(), self._delta_T.get(), self._delta_t.get(), \
               self._number_of_values_for_averaging.get(), self._data_reading_frequency.get()

    @property
    def scenario_file_path(self):
        return self._scenario_file_path.get()

    @property
    def Tmin(self):
        return self._Tmin.get()

    @property
    def Tmax(self):
        return self._Tmax.get()

    @property
    def delta_T(self):
        return self._delta_T.get()

    @property
    def delta_t(self):
        return self._delta_t.get()

    @property
    def number_of_values_for_averaging(self):
        return self._number_of_values_for_averaging.get()

    @property
    def data_reading_frequency(self):
        return self._data_reading_frequency.get()
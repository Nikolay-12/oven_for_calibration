# Данная прога модифицированный вариант проги user_interface, в которой добавлена
# очередь задач и фоновой поток для их исполнения

import tkinter as tk
from tkinter import PhotoImage, ttk
import threading
import queue
import logging

from custom_logging_formatter import CustomFormatter
from events import ComportInitializedEvent, RTParametersWereSetEvent, RVParametersWereSetEvent, DevicesWereSelected, EndlessReadingEvent, StopReadingEvent, UpdateMonitoringTabInfoFromUsedDevices, UpdateMonitoringTabRealTimePlotting, UpdateStatusBar
from function_for_tasks import FunctionForTaskProcessing
from comport_settings import ComportSettings
from RT_mode_settings import RTSettings
from RV_mode_settings import RVSettings
from measurement_settings import MeasurementSettings
from radiobuttons import RadiobuttonsForModes, RadiobuttonsForPlotting
from scenario_control_settings import ScenarioControlSettings
from manual_control_settings import ManualControlSettings
from scenario_generator import ScenarioGenerator
from list_of_connected_devices import ListOfConnectedDevices
from real_time_plotting import RealTimePlotting
from information_from_connected_devices_25T16R import InformationFromConnectedDevices
from plotting import Plotting
from get_and_save_input_data import GetAndSaveInputData
from dop_frame_for_testing import Dop
from utils.input_validation import is_valid_floating_point

logger = logging.getLogger(__name__)
BASIC_FORMAT = "%(levelname)s:%(name)s:%(message)s"

class SettingsTab(ttk.Frame):
    def __init__(self, parent, a_task_queue, a_task_done_queue, method_to_update_status_bar):
        super().__init__()

        check_floating_point = (self.register(is_valid_floating_point), "%P")

        self.parent = parent
        self.task_queue = a_task_queue
        self.task_done_queue: queue.Queue = a_task_done_queue
        self.method_to_update_status_bar = method_to_update_status_bar

        self.comport_settings = ComportSettings(self, self.task_queue, self.method_to_update_status_bar)
        self.comport_settings.ui_draw()

        self.RT_settings = RTSettings(self, check_floating_point, self.task_queue, self.method_to_update_status_bar)
        self.RT_settings.ui_draw()

        self.RV_settings = RVSettings(self, check_floating_point, self.task_queue, self.method_to_update_status_bar)
        self.RV_settings.ui_draw()

        self.measurement_settings = MeasurementSettings(self, check_floating_point, self.task_queue, self.method_to_update_status_bar)
        self.measurement_settings.ui_draw()

        self.measurement_modes_radiobuttons = RadiobuttonsForModes(self)
        self.measurement_modes_radiobuttons.ui_draw()

        self.scenario_control_settings = ScenarioControlSettings(self, check_floating_point, self.task_queue, self.method_to_update_status_bar)
        self.scenario_control_settings.ui_draw()

        self.manual_control_settings = ManualControlSettings(self, check_floating_point, self.task_queue, self.method_to_update_status_bar)
        self.manual_control_settings.ui_draw()

class MonitoringTab(ttk.Frame):
    def __init__(self, parent, a_task_queue, a_task_done_queue, method_to_update_status_bar):
        super().__init__()

        check_floating_point = (self.register(is_valid_floating_point), "%P")

        self.parent = parent
        self.task_queue = a_task_queue
        self.task_done_queue: queue.Queue = a_task_done_queue
        self.method_to_update_status_bar = method_to_update_status_bar

        self.list_of_used_devices = ListOfConnectedDevices(self, self.task_queue, self.method_to_update_status_bar)
        self.list_of_used_devices.ui_draw()

        self.info_from_used_devices = InformationFromConnectedDevices(self)
        self.info_from_used_devices.ui_draw()

        self.real_time_plotting = RealTimePlotting(self)
        self.real_time_plotting.ui_draw()

class AuxiliaryTab(ttk.Frame):
    def __init__(self, parent, a_task_queue, a_task_done_queue, method_to_update_status_bar):
        super().__init__()

        check_floating_point = (self.register(is_valid_floating_point), "%P")

        self.parent = parent
        self.task_queue = a_task_queue
        self.task_done_queue: queue.Queue = a_task_done_queue
        self.method_to_update_status_bar = method_to_update_status_bar

        self.scenario_generator = ScenarioGenerator(self, check_floating_point, self.task_queue, self.method_to_update_status_bar)
        self.scenario_generator.ui_draw()

        self.get_and_save_settings_data = GetAndSaveInputData(self, self.task_queue)
        self.get_and_save_settings_data.ui_draw()

        self.graph_plotting = Plotting(self, check_floating_point, self.task_queue, self.method_to_update_status_bar)
        self.graph_plotting.ui_draw()

        self.dop_frame_for_testing = Dop(self, self.task_queue)
        self.dop_frame_for_testing.ui_draw()


class App(tk.Tk):
    def __init__(self, a_task_queue, a_task_done_queue):
        super().__init__()

        self.task_queue = a_task_queue
        self.task_done_queue: queue.Queue = a_task_done_queue
        logging.basicConfig(level=logging.INFO, handlers=[logging.StreamHandler()])
        logging.getLogger().handlers[0].setFormatter(CustomFormatter(BASIC_FORMAT))

        self.settings_tab = None
        self.monitoring_tab = None
        self.auxiliary_tab = None

    def set_settings_tab(self, settings_frame):
        self.settings_tab = settings_frame
    def set_monitoring_tab(self, monitoring_frame):
        self.monitoring_tab = monitoring_frame
    def set_auxiliary_tab(self, auxiliary_frame):
        self.auxiliary_tab = auxiliary_frame


if __name__ == "__main__":
    task_queue = queue.Queue()
    task_done_queue = queue.Queue()

    app = App(task_queue, task_done_queue)
    app.title("Интерфейс пользователя печки")

    notebook = ttk.Notebook()
    notebook.grid(padx=10, pady=10)

    status_bar_info = tk.StringVar(value="")
    status_bar_label = tk.Label(app, text="Сообщение пользователю:", font=('Arial', 8, 'bold'))
    status_bar_entry = tk.Entry(app, state="disabled", textvariable=status_bar_info, bd=1, relief=tk.SUNKEN)
    status_bar_label.grid(sticky=tk.W)
    status_bar_entry.grid(sticky=tk.EW)

    def update_status_bar(text: str):
        status_bar_info.set(text)

    settings_tab = SettingsTab(app, task_queue, task_done_queue, update_status_bar)
    monitoring_tab = MonitoringTab(app, task_queue, task_done_queue, update_status_bar)
    auxiliary_tab = AuxiliaryTab(app, task_queue, task_done_queue, update_status_bar)
    app.set_settings_tab(settings_tab)
    app.set_monitoring_tab(monitoring_tab)
    app.set_auxiliary_tab(auxiliary_tab)

    settings_logo = PhotoImage(file="./settings_logo.png")#./start_logo.jpg
    monitoring_logo = PhotoImage(file="./monitoring_logo.png") #./monitoring_logo.png

    notebook.add(settings_tab, text="Settings frame", image=settings_logo, compound="right")
    notebook.add(monitoring_tab, text="Monitoring frame", image=monitoring_logo, compound="right")
    notebook.add(auxiliary_tab, text="Auxiliary frame")

    def worker():  # Функция, которая выполняется в фоновом потоке (её работа заключается в выполнении задач из очереди)
        function_for_threading = FunctionForTaskProcessing(app, update_status_bar)

        async_tasks = dict()
        events_mapping = {
                EndlessReadingEvent: StopReadingEvent
                }

        while True:
            try:
                task_data = task_queue.get(timeout=1)
            except queue.Empty:
                pass
            except Exception as e:
                logger.info(f"Ошибка: {e}")
                print(f"Ошибка: {e}")
            else:
                if task_data in async_tasks:
                    async_tasks[task_data].set()
                    async_tasks.pop(task_data)
                    return

                if not task_data.is_async():
                    result_event = function_for_threading.func_for_task_processing(task_data, task_queue, task_done_queue)
                    task_done_queue.put(result_event)
                    update_status_bar(result_event)
                    logger.info(result_event)
                else:
                    if events_mapping[task_data] not in async_tasks:
                        event_for_stopping_async_task = function_for_threading.func_for_async_task(task_data, task_done_queue)
                        async_tasks[events_mapping[task_data]] = event_for_stopping_async_task
    thread = threading.Thread(target=worker, daemon=True)  # Создание фонового потока
    thread.start()


    def get_result():
        try:
            last_done_event = task_done_queue.get(timeout=0.01)
        except queue.Empty:
            pass
        except Exception as e:

            print(f"Ошибка: {e}")
        else:
            if isinstance(last_done_event, ComportInitializedEvent):
                settings_tab.comport_settings.update_button_color()
            if isinstance(last_done_event, RTParametersWereSetEvent):
                settings_tab.RT_settings.update_button_color()
            if isinstance(last_done_event, RVParametersWereSetEvent):
                settings_tab.RV_settings.update_button_color()
            if isinstance(last_done_event, DevicesWereSelected):
                monitoring_tab.list_of_used_devices.update_button_color()
            if isinstance(last_done_event, UpdateStatusBar):
                status_bar_info.set(last_done_event.message)
            if isinstance(last_done_event, UpdateMonitoringTabInfoFromUsedDevices):
                monitoring_tab.info_from_used_devices.update_data(last_done_event.data_array)
            if isinstance(last_done_event, UpdateMonitoringTabRealTimePlotting):
                monitoring_tab.real_time_plotting.update_plot(last_done_event.data_array)
        app.after(10, get_result)

    get_result()

    app.mainloop()

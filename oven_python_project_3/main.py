# Данная прога модифицированный вариант проги user_interface, в которой добавлена
# очередь задач и фоновой поток для их исполнения

import tkinter as tk
import threading
import queue
import logging

from custom_logging_formatter import CustomFormatter
from events import ComportInitializedEvent, EndlessReadingStartedEvent, PidInitializedEvent, DevicesWereSelected
from function_for_tasks import FunctionForTaskProcessing
from comport_settings import ComportSettings
from RT_mode_settings import PidSettings
from data_exchange import DataExchange
from scenario_control_settings import ScenarioControlSetttings
from scenario_generator import ManualControlSettings
from list_of_connected_devices import ListOfConnectedDevices
from get_and_save_input_data import GetAndSaveInputData
from plotting import Plotting
from information_from_connected_devices_25T16R import InformationFromConnectedDevices
from dop_frame_for_testing import Dop
from info_frame import InfoFrame
from utils.input_validation import is_valid_floating_point

logger = logging.getLogger(__name__)
BASIC_FORMAT = "%(levelname)s:%(name)s:%(message)s"

class App(tk.Tk):
    def __init__(self, a_task_queue, a_task_done_queue):
        super().__init__()

        check_floating_point = (self.register(is_valid_floating_point), "%P")

        self.task_queue = a_task_queue
        self.task_done_queue: queue.Queue = a_task_done_queue
        logging.basicConfig(level=logging.INFO, handlers=[logging.StreamHandler()])
        logging.getLogger().handlers[0].setFormatter(CustomFormatter(BASIC_FORMAT))

        self.comport_settings = ComportSettings(self, self.task_queue)
        self.comport_settings.ui_draw()

        self.pid_setting = PidSettings(self, check_floating_point, self.task_queue)
        self.pid_setting.ui_draw()

        self.list_of_used_devices = ListOfConnectedDevices(self, self.task_queue)
        self.list_of_used_devices.ui_draw()

        #self.data_exchanging = DataExchange()

        self.scenario_control_setting = ScenarioControlSetttings(self, check_floating_point, self.task_queue)
        self.scenario_control_setting.ui_draw()

        self.manual_control_setting = ManualControlSettings(self, check_floating_point, self.task_queue)
        self.manual_control_setting.ui_draw()

        #self.get_and_save_data = GetAndSaveInputData(self, self.task_queue)
        #self.get_and_save_data.ui_draw()

        #self.graph_plotting = Plotting(self, check_floating_point, self.task_queue)
        #self.graph_plotting.ui_draw()

        self.info_from_used_devices = InformationFromConnectedDevices(self)
        self.info_from_used_devices.ui_draw()

        #self.dop_frame_for_testing = Dop(self, self.task_queue)
        #self.dop_frame_for_testing.ui_draw()

        self.information = InfoFrame(self)
        self.information.ui_draw()

        #self.after(10, self.get_result)
    def get_result(self):
        try:
            last_done_event = task_done_queue.get(timeout=1)
        except queue.Empty:
            pass
        except Exception as e:
            print(f"Ошибка: {e}")
        else:
            if isinstance(last_done_event, ComportInitializedEvent):
                self.comport_settings.update_button_color()
            if isinstance(last_done_event, PidInitializedEvent):
                self.pid_setting.update_button_color()
            if isinstance(last_done_event, DevicesWereSelected):
                self.list_of_used_devices.update_button_color()
        self.after(10, self.get_result)


if __name__ == "__main__":
    task_queue = queue.Queue()
    task_done_queue = queue.Queue()
    #last_done_event = None

    app = App(task_queue, task_done_queue)
    app.title("Интерфейс пользователя печки")

    def worker():  # Функция, которая выполняется в фоновом потоке (её работа заключается в выполнении задач из очереди)
        function_for_threading = FunctionForTaskProcessing(app)

        while True:
            try:
                task_data = task_queue.get(timeout=1)
            except queue.Empty:
                pass
            except Exception as e:
                print(f"Ошибка: {e}")
            else:
                result_event = function_for_threading.func_for_task_processing(task_data, task_queue)
                task_done_queue.put(result_event)
                print(result_event)
                #last_done_event = task_done_queue.get(timeout=1)
                #print()

    thread = threading.Thread(target=worker, daemon=True)  # Создание фонового потока
    thread.start()

    app.mainloop()


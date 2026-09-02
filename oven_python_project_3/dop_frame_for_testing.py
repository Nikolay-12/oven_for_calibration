# Данная прога нужна для тестирования проги user_interface
import tkinter as tk
import queue

from events import EndlessReadingEvent, StopReadingEvent

#from utils.floating_point_validation import is_valid_floating_point

class Dop:
    def __init__(self, parent, task_queue) -> None:
        self.parent = parent
        #self.floating_point_validation_function = floating_point_validation_function
        self.task_queue = task_queue

    def ui_draw(self):
        dop_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Список дополнительных задач")
        dop_frame.grid(padx=10, pady=5, row=3, column=1, sticky=tk.NSEW)

        tk.Label(dop_frame, text="Чтение данных\nс устройств:").grid(row=1, column=1)
        tk.Label(dop_frame, text="Остановить чтение:").grid(row=2, column=1)

        tk.Button(dop_frame,
                  text="read a lot",
                  command=self.add_reading_task_to_queue,
                  bd=3).grid(padx=2, row=1, column=2)
        tk.Button(dop_frame,
                  text="stop",
                  command=self.add_stop_task_to_queue,
                  bd=3).grid(padx=2, row=2, column=2)

    def add_reading_task_to_queue(self):
        self.task_queue.put(EndlessReadingEvent())
    def add_stop_task_to_queue(self):
        self.task_queue.put(StopReadingEvent())

import tkinter as tk
from tkinter import StringVar

common_label_ui_params = {"font": ('Arial', 9, 'bold'), "bd": 4}

class InfoFrame:
    def __init__(self, parent):
        self.parent = parent

        self._info_textvar = StringVar(value="")

    def ui_draw(self):
        info_frame = tk.LabelFrame(self.parent, padx=15, pady=10, text="Информационное окно")
        info_frame.grid(padx=10, pady=5, row=5, column=1, columnspan=3, sticky=tk.EW)

        tk.Label(info_frame, text="Сообщения пользователю:", font=('Arial', 9)).grid(row=1, column=1, columnspan=10,
                                                                                                sticky=tk.W)
        tk.Entry(info_frame,
                 state="disabled",
                 textvariable=self._info_textvar,
                 font=('Arial', 10, 'bold'), width=140).grid(row=2, column=1, sticky=tk.EW)

    def update_info_data(self, text):
        self._info_textvar.set(text)

    @property
    def info_var_content(self):
        return self._info_textvar.get()
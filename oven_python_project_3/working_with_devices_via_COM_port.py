import serial
from datetime import datetime

from comport_settings import ComportSettings
from RT_mode_settings import PidSettings
#from scenario_control_settings import ScenarioControlSetttings
from scenario_generator import ManualControlSettings
from list_of_connected_devices import ListOfConnectedDevices
from information_from_connected_devices import InformationFromConnectedDevices

Number_of_thermocouples = 9
Number_of_sensors = 16

class WorkingWithDevicesViaCOMPort:
    def __init__(self, parent):
        self.parent = parent
        self.Selected_sensors_data = dict()
        self.Selected_thermocouples_data = dict()

    def read_data_from_devices_and_write_it(self):
        #self.COM_port = self.parent.comport_settings.port
        self.baudrate = self.parent.comport_settings.baudrate
        self.used_devices = self.parent.list_of_used_devices.connected_devices
        print("Успешно определен baudrate и устройства!", self.baudrate, self.used_devices)
        for i in range(1, Number_of_sensors + 1):
            self.parent.info_from_used_devices.selected_sensors_data[i].set("")
            if self.parent.info_from_used_devices._selected_sensors_text[i].get() in self.used_devices:
                self.parent.info_from_used_devices.selected_sensors_data[i].set("...")
        for i in range(1, Number_of_thermocouples + 1):
            self.parent.info_from_used_devices.selected_thermocouples_data[i].set("")
            if self.parent.info_from_used_devices._selected_thermocouples_text[i].get() in self.used_devices:
                self.parent.info_from_used_devices.selected_thermocouples_data[i].set("...")

        self.read_data_from_devices()

    def read_data_from_devices(self):
        self.COM_port = self.parent.comport_settings.port
        command_line = 'Read'
        try:
            ser = serial.Serial(self.COM_port, baudrate=self.baudrate) #, timeout=1
            print("Serial connection established.")
            ser.write(command_line.encode())
            #print(command_line.encode())
            while command_line == 'Read':
                #print('раз')
                ser.write(command_line.encode())
                line = ser.readline().decode().strip()
                glossary = '0123456789,. '
                if line:
                    current_string_line = str(line)  # чтение данных из выбранного СОМ-порта и перевод в строку
                    #current_data_line = ''.join([ch for ch in current_string_line if ch in glossary])
                    #print("Получено:", current_string_line, ' и ', current_data_line)
                    print("Получено:", current_string_line)

        except serial.SerialException as se:
            print("Serial port error:", str(se))

        except KeyboardInterrupt:
            pass

    def save_data_from_devices(self):
        self.COM_port = self.parent.comport_settings.port
        r = serial.Serial(self.COM_port, self.baudrate, timeout=1)
        f = open('save_data_testing.txt', 'w')
        glossary = '0123456789,. '  # словарик, который передаёт нам из COM-порта только цифры и знаки препинания
        start_line = 'Начинаем считывать данные'
        for i in range(20):  # собираем 20 показаний
            N = str(r.readline())  # чтение данных из выбранного СОМ-порта и перевод в строку
            N1 = ''.join([ch for ch in N if ch in glossary])
            f.write(datetime.now().strftime("%Y-%m-%d_%H:%M:%S  ") + N1 + '\n')
        f.close()

    def start_working_on_the_script(self):
        pass

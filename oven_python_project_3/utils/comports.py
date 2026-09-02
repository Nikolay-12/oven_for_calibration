import typing
import serial
import serial.tools.list_ports

def get_comports() -> typing.List[str]:
    return [element.device for element in serial.tools.list_ports.comports()]

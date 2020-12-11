import RPi.GPIO as GPIO
from enum import Enum

class GpioOutputDevices(Enum):
    Lamp01 = 17
    Lamp02 = 18

class GpioInputDevices(Enum):
    Presence01 = 25
    Presence02 = 26
    Open01 = 5
    Open02 = 6
    Open03 = 12
    Open04 = 16


def init_in_devices():
    for device in GpioInputDevices:
        GPIO.setup(device.value, GPIO.IN)

def init_out_devices():
    for device in GpioOutputDevices:
        GPIO.setup(device.value, GPIO.OUT)

def setup_gpio():
    GPIO.setmode(GPIO.BCM)
    init_in_devices()
    init_out_devices()

def read_devices():
    devices_status = []

    for device in GpioInputDevices:
        devices_status.append({"name": device.name, "value": GPIO.input(device.value)})

    for device in GpioOutputDevices:
        devices_status.append({"name": device.name, "value": GPIO.input(device.value)})

    return devices_status

def toggle_device(device):
    current = GPIO.input(device)
    if current == 0:
        GPIO.output(device, 1)
    else:
        GPIO.output(device, 0)

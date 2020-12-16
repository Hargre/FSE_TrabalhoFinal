import threading
import subprocess
import json
import csv
from time import sleep
from logger import Logger
from mqtt import MqttClient
from climate import Climate
from state import HouseState
from gpio import read_devices, toggle_device

def trigger_alarm():
    alarm_playback = threading.Thread(target=play_alarm)
    alarm_playback.start()
    logger = Logger.get_instance()
    logger.log_alarm()

def play_alarm():
    alarm_playback = subprocess.Popen(['omxplayer', 'media/alarm.mp3'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True)
    sleep(5)
    alarm_playback.stdin.write(b'q')

def mqtthread(socketio):
    client = MqttClient.get_instance(socketio)

def sensor_pollings(socketio):
    climate = Climate()
    state = HouseState.get_instance()
    while True:
        cli_data = climate.read_data()
        sensor_data = read_devices()
        socketio.emit('update_central', {
            "temperature": "%.2f" % cli_data.temperature,
            "humidity": "%.2f" % cli_data.humidity,
            "sensors": sensor_data
        })
        if state.alarm and state.did_state_change(sensor_data):
            print("Alarm!!!")
            trigger_alarm()
        state.set_state(sensor_data)
        sleep(1)

def register_device(data, socketio):
    print(data)
    client = MqttClient.get_instance(socketio)
    client.publish("dispositivos/%s" % data['address'], json.dumps({"room": data['roomName']}))
    with open('data/devices.csv', 'a+', newline='') as devices_csv:
        writer = csv.writer(devices_csv)
        writer.writerow([data['roomName'], data['inDevice'], data['outDevice']])
    logger = Logger.get_instance()
    logger.log_command("Register")

def toggle_alarm(data, socketio):
    success = True
    state = HouseState.get_instance()
    if state.alarm:
        state.alarm = False
    else:
        if state.any_inputs_active():
            success = False
        else:
            state.alarm = True
    logger = Logger.get_instance()
    logger.log_command('ToggleAlarm')
    socketio.emit('alarmToggled', {'success': success, 'newState': state.alarm})

def handle_button(data):
    toggle_device(data['id'])
    logger = Logger.get_instance()
    logger.log_command(str(data['id']))


def toggle_client_output(data, socketio):
    print(data)
    client = MqttClient.get_instance(socketio)
    client.publish("%s/output" % data['id'], json.dumps({"toggle": 1}))

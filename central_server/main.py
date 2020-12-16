from gpio import setup_gpio
from gpio import close_gpio
from climate import Climate
from state import HouseState
from control import mqtthread
from control import sensor_pollings
from control import register_device
from control import handle_button
from control import toggle_client_output
from control import toggle_alarm
from logger import Logger
from mqtt import MqttClient
from flask import Flask, render_template
from flask_socketio import SocketIO, emit
import threading

import signal
import sys

import csv

def cleanup(signal, frame):
    close_gpio()
    sys.exit(0)

signal.signal(signal.SIGINT, cleanup)

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)

import logging
logging.getLogger('werkzeug').setLevel(logging.ERROR)

@app.route('/')
def index():
    state = HouseState.get_instance()
    state_data = {
        "alarm": "Ativado" if state.alarm else "Desativado",
        "alarmButton": "Desativar" if state.alarm else "Ativar"
    }
    devices = []
    try:
        with open("data/devices.csv") as devices_csv:
            reader = csv.reader(devices_csv)
            for row in reader:
                devices.append({"room": row[0], "in": row[1], "out": row[2]})
    except FileNotFoundError:
        with open("data/devices.csv", "w") as devices_csv:
            pass
    return render_template('index.html', devices=devices, state=state_data)


@socketio.on('button')
def handle_button_event(data):
    handle_button(data)

@socketio.on('register')
def register_device_event(data):
    register_device(data, socketio)

@socketio.on('remoteOut')
def toggle_client_output_event(data):
    toggle_client_output(data, socketio)

@socketio.on('toggleAlarm')
def toggle_alarm_event(data):
    toggle_alarm(data, socketio)

def main():
    logger = Logger.get_instance()
    state = HouseState.get_instance()
    setup_gpio()
    t1 = threading.Thread(target=mqtthread, args=(socketio,), daemon=True)
    t1.start()
    t2 = threading.Thread(target=sensor_pollings, args=(socketio,), daemon=True)
    t2.start()

if __name__ == "__main__":
    main()
    socketio.run(app, port=10103)


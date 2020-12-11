from gpio import setup_gpio
from gpio import read_devices
from gpio import toggle_device
from climate import Climate
from mqtt import MqttClient
from flask import Flask, render_template
from flask_socketio import SocketIO, emit
import threading
from time import sleep

import json
import csv

def mqtthread(socketio):
    client = MqttClient.get_instance(socketio)

def sensor_pollings(socketio):
    climate = Climate()
    while True:
        cli_data = climate.read_data()
        sensor_data = read_devices()
        socketio.emit('update_central', {
            "temperature": "%.2f" % cli_data.temperature,
            "humidity": "%.2f" % cli_data.humidity,
            "sensors": sensor_data
        })
        sleep(1)

app = Flask(__name__)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)

@app.route('/')
def index():
    devices = []
    try:
        with open("data/devices.csv") as devices_csv:
            reader = csv.reader(devices_csv)
            for row in reader:
                devices.append({"room": row[0], "in": row[1], "out": row[2]})
    except FileNotFoundError:
        with open("data/devices.csv", "w") as devices_csv:
            pass
    return render_template('index.html', devices=devices)


@socketio.on('button')
def handle_button(data):
    toggle_device(data['id'])

@socketio.on('register')
def register_device(data):
    print(data)
    client = MqttClient.get_instance(socketio)
    client.publish("dispositivos/%s" % data['address'], json.dumps({"room": data['roomName']}))
    with open('data/devices.csv', 'a+', newline='') as devices_csv:
        writer = csv.writer(devices_csv)
        writer.writerow([data['roomName'], data['inDevice'], data['outDevice']])
    print(data)

@socketio.on('remoteOut')
def toggle_client_output(data):
    print(data)
    client = MqttClient.get_instance(socketio)
    client.publish("%s/output" % data['id'], json.dumps({"toggle": 1}))


def main():
    setup_gpio()
    t1 = threading.Thread(target=mqtthread, args=(socketio,), daemon=True)
    t1.start()
    t2 = threading.Thread(target=sensor_pollings, args=(socketio,), daemon=True)
    t2.start()

if __name__ == "__main__":
    main()
    socketio.run(app, port=10103)


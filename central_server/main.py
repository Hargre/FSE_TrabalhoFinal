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
    return render_template('index.html')


@socketio.on('button')
def handle_button(data):
    toggle_device(data['id'])

@socketio.on('register')
def register_device(data):
    print(data)
    client = MqttClient.get_instance(socketio)
    client.publish("dispositivos/%s" % data['address'], json.dumps({"room": data['roomName']}))
    client.subscribe(data['roomName'])
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


import paho.mqtt.client as mqtt
import json



class MqttClient:
    def __init__(self, socketio):
        self.broker = "test.mosquitto.org"
        self.client = mqtt.Client()
        self.socket = socketio
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.connect(self.broker)
        self.client.loop_start()
    
    def on_message(self, client, userdata, message):
        data = json.loads(message.payload.decode('utf-8'))
        self.socket.emit('register', data)
        print(data)

    def on_connect(self, client, userdata, flags, rc):
        print("Connected to broker")
        client.subscribe("fse2020/150009313/dispositivos/#")

    def destroy(self):
        self.client.disconnect()
        self.client.loop_stop()


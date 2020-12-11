import paho.mqtt.client as mqtt
import json



class MqttClient:
    _instance = None

    def __init__(self, socketio):
        self.broker = "test.mosquitto.org"
        self.client = mqtt.Client()
        self.socket = socketio
        self.client.on_connect = self.on_connect
        self.client.on_message = self.on_message
        self.client.connect(self.broker)
        self.client.loop_start()
        self.subscribe()
    
    @classmethod
    def get_instance(cls, socketio):
        if cls._instance is None:
            cls._instance = cls(socketio)
        return cls._instance

    def on_message(self, client, userdata, message):
        print(message.payload)
        data = json.loads(message.payload.decode('utf-8'))


        topic = message.topic.split("/")
        print(topic)

        if topic[2] == 'dispositivos' and topic[3] not in ['temperatura', 'umidade', 'estado']:
            if (data.get('action')):
                self.socket.emit('register', {"address": topic[3]})
        else:
            self.socket.emit('update_client', {"sensor": data, "room": topic[2]})
        print(data)

    def on_connect(self, client, userdata, flags, rc):
        print("Connected to broker")
        client.subscribe("fse2020/150009313/dispositivos/#")

    def subscribe(self):
        self.client.subscribe("fse2020/150009313/+/temperatura")
        self.client.subscribe("fse2020/150009313/+/umidade")
        self.client.subscribe("fse2020/150009313/+/estado")


    def publish(self, topic, message):
        topic_base = "fse2020/150009313/%s" % (topic)
        print(topic_base)
        print(message)
        self.client.publish(topic_base, message)

    def destroy(self):
        self.client.disconnect()
        self.client.loop_stop()


import paho.mqtt.client as paho
import matplotlib.pyplot as plt
import numpy as np
import time as t

i = 0
tilt = np.zeros(40)

mqttc = paho.Client()

host = "localhost"
topic = "HAHAHAISME"
port = 1883

def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))
def on_message(mosq, obj, msg):
    global i
    global tilt
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")
    if str(msg.payload.decode("utf-8")) == "1":
        tilt[i] = 1
    else:
        tilt[i] = 0
    i = i + 1
def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")
def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port = 1883, keepalive = 60)
mqttc.subscribe(topic, 0)

mqttc.loop_start()
t.sleep(30)
mqttc.loop_stop()
mqttc.disconnect()

plt.stem(np.linspace(0, 20, 40), tilt)
plt.title('Tilt')
plt.xlabel('t')
plt.ylabel('tilt')
plt.show()
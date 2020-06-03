import matplotlib.pyplot as plt
import paho.mqtt.client as paho
import numpy as np
import serial
import time

mqttc = paho.Client()

host = "localhost"
topic= "HAHAHAISME"
port = 1883

def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))
def on_message(mosq, obj, msg):
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")
def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")
def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)

serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)
s.write("+++".encode())
char = s.read(2)
print("Enter AT mode.")
print(char.decode())
s.write("ATMY 0x415\r\n".encode())
char = s.read(3)
print("Set MY 41.")
print(char.decode())
s.write("ATDL 0x405\r\n".encode())
char = s.read(3)
print("Set DL 40.")
print(char.decode())
s.write("ATID 0x12\r\n".encode())
char = s.read(3)
print("Set PAN ID 12.")
print(char.decode())
s.write("ATWR\r\n".encode())
char = s.read(3)
print("Write config.")
print(char.decode())
s.write("ATMY\r\n".encode())
char = s.read(4)
print("MY :")
print(char.decode())
s.write("ATDL\r\n".encode())
char = s.read(4)
print("DL : ")
print(char.decode())
s.write("ATCN\r\n".encode())
char = s.read(3)
print("Exit AT mode.")
print(char.decode())
print("start sending RPC")

s.write("\r".encode())
s.readline()

duration = 20

t = np.arange(0, duration, 0.5)
t_0 = np.arange(0, duration, 1)
counter = np.zeros(20)
x = np.zeros(40)
y = np.zeros(40)
z = np.zeros(40)

for i in range(20):
    s.write("/getValue/run\r".encode())
    print(i)
    time.sleep(1)
    line = s.readline()
    counter[i] = int(line)
    for j in range(int(counter[i])):    
        line = s.readline()
        X = float(line)
        x[2 * i + j] = X
        line = s.readline()
        Y = float(line)
        y[2 * i + j] = Y
        line = s.readline()
        Z = float(line)
        z[2 * i + j] = Z
        if (X * X + Y * Y > Z * Z):
            mqttc.publish(topic, "1")
        else:
            mqttc.publish(topic, "0")
        
plt.plot(t, x, label = "$x$")
plt.plot(t, y, label = "$y$")
plt.plot(t, z, label = "$z$")
plt.plot(t_0, counter, label = "$counter$")
plt.legend(loc = 'lower left')
plt.show()

s.close()
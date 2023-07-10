import paho.mqtt.client as mqtt
import matplotlib.pyplot as plt
import numpy as np

mqttServer = "test.mosquitto.org"
mqttPort = 1883
mqttTopic = "mslc/umidade"

umidade_values = []  # Lista para armazenar os valores de umidade
moving_average_values = []  # Lista para armazenar os valores da média móvel

plt.ion()  # Ativa o modo de plotagem interativa

def on_connect(client, userdata, flags, rc):
    print("Conectado ao broker MQTT")
    client.subscribe(mqttTopic)

def on_message(client, userdata, msg):
    umidade = float(msg.payload.decode())
    umidade_values.append(umidade)

    # Calcula a média móvel com base nos últimos 10 valores de umidade
    if len(umidade_values) >= 10:
        moving_average = np.mean(umidade_values[-10:])
        moving_average_values.append(moving_average)

        # Limpa o gráfico anterior e plota o novo gráfico
        plt.clf()
        plt.plot(umidade_values, label='Umidade')
        plt.plot(moving_average_values, label='Média Móvel')
        plt.xlabel('Amostras')
        plt.ylabel('Umidade')
        plt.title('Gráfico de Umidade com Média Móvel')
        plt.legend()
        plt.draw()
        plt.pause(0.050)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(mqttServer, mqttPort, 60)

client.loop_start()

while True:
    pass

#include<Arduino.h>
#include<WiFi.h>
#include <PubSubClient.h>
#define AOUT_PIN 36 // ESP32 pin GIOP36 (ADC0) that connects to AOUT pin of moisture sensor

const char* ssid = "ClaroWiFi";       // SSID / nome da rede WiFi
const char* password = "yuri5252";  // Senha da rede WiFi

const char* mqttServer = "test.mosquitto.org"; // Endereço IP do Mosquitto Broker
const int mqttPort = 1883;                // Porta do Mosquitto Broker
int long cont = 0;

const char* topico = "umidade";
WiFiClient espClient;
PubSubClient mqttClient(espClient);

float calibracao(int sensor_value)
{
  int sensor_value_real = 0;
  float calc = 0;
  if (sensor_value > 2790){
    sensor_value_real = 2790;
    return calc;
  } else sensor_value_real = sensor_value;
  calc = 77.553 - 0.033557 * sqrt(2980 * sensor_value_real - 2703490);
  return calc;
}

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se à rede Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado com sucesso!");
  Serial.print("Endereço IP obtido: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando-se ao broker MQTT: ");
    Serial.println(mqttServer);

    if (mqttClient.connect("ESP8266Client")) {
      Serial.println("Conectado ao broker MQTT com sucesso!");
    } else {
      Serial.print("Falha na conexão ao broker. Estado: ");
      Serial.print(mqttClient.state());
      Serial.println(". Tentando novamente em 1 segundos...");
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setupWiFi();
  mqttClient.setServer(mqttServer, mqttPort);
}

void loop() {
  int value = analogRead(AOUT_PIN); // read the analog value from sensor
  float valor_calibrado = calibracao(value);
  Serial.print("Calibracao:");
  Serial.println(valor_calibrado);

  Serial.print("Moisture value: ");
  Serial.println(value);

  delay(500);
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // Publica uma mensagem no tópico "Umidade"
  mqttClient.publish(topico, String(valor_calibrado).c_str(), 1);

  cont = cont + 1;
  Serial.print("Hello, MQTT! ");
  Serial.print(cont);
  Serial.print("\n");
  delay(300);
}

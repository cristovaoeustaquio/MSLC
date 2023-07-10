#include<Arduino.h>
#include<WiFi.h>
#include <PubSubClient.h>
#define AOUT_PIN 36 // ESP32 pin GIOP36 (ADC0) that connects to AOUT pin of moisture sensor
#define PUMP_PIN 19 // ESP32 pin GPIO19 that connects to pump driver circuit
#define SAMPLES 10
#define DRY 1
#define WET 2
#define WATER 3
#define LIMITE_SECO 30
#define LIMITE_UMIDO 45

const char* ssid = "ClaroWiFi";       // SSID / nome da rede WiFi
const char* password = "yuri5252";  // Senha da rede WiFi

const char* mqttServer = "test.mosquitto.org"; // Endereço IP do Mosquitto Broker
const int mqttPort = 1883;                // Porta do Mosquitto Broker

const char* topico = "mslc/umidade";
WiFiClient espClient;
PubSubClient mqttClient(espClient);

bool pump_state = false;

float calibracao(int sensor_value)
{
  int sensor_value_real = 0;
  float calc = 0;
  if (sensor_value > 2700){
    sensor_value_real = 2700;
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

    if (mqttClient.connect("mslcClient")) {
      Serial.println("Conectado ao broker MQTT com sucesso!");
    } else {
      Serial.print("Falha na conexão ao broker. Estado: ");
      Serial.print(mqttClient.state());
      Serial.println(". Tentando novamente em 1 segundos...");
      delay(1000);
    }
  }
}


float realizar_leitura_sensor(){
  int value = analogRead(AOUT_PIN); // read the analog value from sensor
  Serial.println(value);
  float valor_calibrado = calibracao(value);
  return valor_calibrado;
}

float ler_umidade(){
  float soma = 0;
  for(int i = 0; i < SAMPLES; i++){
    soma += realizar_leitura_sensor();
    delay(20);
  }
  return soma/SAMPLES;
}

int classificar_umidade(float umidade){
  if(umidade <= LIMITE_SECO){
    return DRY;
  }
  else if(umidade <= LIMITE_UMIDO){
    return WET;
  }
  else{
    return WATER;
  }
}

bool bomba_esta_desligada(){
  return !pump_state;
}

bool bomba_esta_ligada(){
  return pump_state;
}

void desativar_bomba(){
  if(bomba_esta_ligada()){
    digitalWrite(PUMP_PIN, LOW);
    pump_state = false;
  }
}

void ativar_bomba(){
  Serial.println(pump_state);
  if(bomba_esta_desligada()){
    digitalWrite(PUMP_PIN, HIGH);
    pump_state = true;
  }
}


void comandar_bomba(int classificacao){
  if(classificacao == DRY){
    ativar_bomba();
  }
  else{
    desativar_bomba();
  }
}


void setup() {
  Serial.begin(9600);
  setupWiFi();
  mqttClient.setServer(mqttServer, mqttPort);
  pinMode(PUMP_PIN, OUTPUT);    // sets the digital pin 19 as output
}

void loop() {
  float valor_calibrado = ler_umidade();
  int classif = classificar_umidade(valor_calibrado);
  comandar_bomba(classif);
  
  Serial.print("Calibracao:");
  Serial.println(valor_calibrado);

  delay(500);
  if (mqttClient.connected()) {
    mqttClient.publish(topico, String(valor_calibrado).c_str(), 1);
    mqttClient.loop();
  }
  else reconnectMQTT();
  
}

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

const char *SSID = "your_wifi_ssid";
const char *PWD = "your_wifi-pwd";

Adafruit_BME280 bme;

long last_time = 0;
char data[100];

// MQTT client
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient); 

char *mqttServer = "broker.hivemq.com";
int mqttPort = 1883;


void connectToWiFi() {
  Serial.print("Connectiog to ");
 
  WiFi.begin(SSID, PWD);
  Serial.println(SSID);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.print("Connected.");
  
} 

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Callback - ");
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  mqttClient.setCallback(callback);
}


void setup() {
  Serial.begin(9600);
  
  connectToWiFi();

  if (!bme.begin(0x76)) {
    Serial.println("Problem connecting to BME280");
  }

  setupMQTT();
    
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT Broker..");
      String clientId = "ESP32Client-";
      clientId += String(random(0xffff), HEX);
      
      if (mqttClient.connect(clientId.c_str())) {
        Serial.println("Connected.");
        // subscribe to topic
        mqttClient.subscribe("/swa/commands");
      }
      
  }
}


void loop() {

  if (!mqttClient.connected())
    reconnect();

  mqttClient.loop();

  long now = millis();
  if (now - last_time > 60000) {
    // Send data
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = bme.readPressure() / 100;
    

    // Publishing data throgh MQTT
    sprintf(data, "%f", temp);
    Serial.println(data);
    mqttClient.publish("/swa/temperature", data);
    sprintf(data, "%f", hum);
    Serial.println(hum);
    mqttClient.publish("/swa/humidity", data);
    sprintf(data, "%f", pres);
    Serial.println(pres);
    mqttClient.publish("/swa/pressure", data);
    last_time = now;
  }

}
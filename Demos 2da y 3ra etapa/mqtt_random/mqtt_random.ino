#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "iot_informatica";
const char* password = "dragino-dragino";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "10.130.1.233";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//uncomment the following lines if you're using SPI
/*#include <SPI.h>
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5*/




// LED Pin
const int ledPin = 4;

void setup() {
  
  Serial.begin(115200);
  randomSeed(analogRead(0));
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);


}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(ledPin, LOW);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(ledPin, HIGH);
    }
  }
}

const char* mqtt_user = "esp32";  
const char* mqtt_password = "esp32";  
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("esp32",mqtt_user,mqtt_password)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    // Simulación de Temperatura
    int RA_Temp = random(15, 31);
    char tempString[8];
    dtostrf(RA_Temp, 1, 0, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("esp32/temperature", tempString);
  // Simulación de Humedad
    int RA_Hum = random(20,60);
    char humString[8];
    dtostrf(RA_Hum, 1, 0,humString );
    Serial.print("Humidity: ");
    Serial.println(humString);
    client.publish("esp32/humidity", humString);
  // Simulación de Luz solar
    int RA_Light = random(70,80);
    char lightString[8];
    dtostrf(RA_Light, 1, 0,lightString );
    Serial.print("Light Intensity: ");
    Serial.println(lightString);
    client.publish("esp32/light", lightString);

    // Simulación de calidad del aire 
    int RA_AirQuality = random(300, 500);  // Rango en ppm
    char airQualityString[8];
    dtostrf(RA_AirQuality, 1, 0, airQualityString);
    Serial.print("Air Quality: ");
    Serial.println(airQualityString);
    client.publish("esp32/air_quality", airQualityString);

    // Simulación de pH del suelo
    float RA_pH = random(50, 80) / 10.0;  
    char phString[8];
    dtostrf(RA_pH, 1, 1, phString);
    Serial.print("Soil pH: ");
    Serial.println(phString);
    client.publish("esp32/soil_ph", phString);

    // Simulación de velocidad del viento 
    int RA_WindSpeed = random(5, 20);  
    char windSpeedString[8];
    dtostrf(RA_WindSpeed, 1, 0, windSpeedString);
    Serial.print("Wind Speed: ");
    Serial.println(windSpeedString);
    client.publish("esp32/wind_speed", windSpeedString);
  }
}
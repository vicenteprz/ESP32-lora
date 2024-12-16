#include <ArduinoJson.h>      // Librería para manejar JSON, usada para organizar y leer datos.
#include <SPI.h>              // Protocolo de comunicación para LoRa.
#include <LoRa.h>             // Librería para manejar LoRa.
#include <WiFi.h>             // Librería para conectar el ESP32 a Wi-Fi.
#include <PubSubClient.h>     // Librería para trabajar con MQTT.
#include <Wire.h>             // Librería para comunicaciones entre dispositivos.
#include <../arduino_secrets.h> //Archivo oculto en github con el contenido de credenciales confidenciales del servidor

#define SCK 5                 // Pines para configurar el módulo LoRa.
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 23
#define DIO0 26

int id, Temperature, Humidity, Light, PM01, PM25, PM10, Gas, Moisture, Checksum;
                                          // Variable para el pH.

const char* ssid = "iot_informatica";             // Nombre de la red Wi-Fi.
const char* password = "dragino-dragino";         // Contraseña de la red Wi-Fi.

const char* mqtt_server = SECRET_SERVER;         // Dirección IP del servidor MQTT.

WiFiClient espClient;                             // Crea un objeto para la conexión Wi-Fi.
PubSubClient client(espClient);                   // Crea un cliente MQTT usando la conexión Wi-Fi.
long lastMsg = 0;                                 // Variable para manejar mensajes de tiempo.
char msg[50];                                     // Buffer para mensajes.
int value = 0;                                    // Variable temporal.
const int ledPin = 4;                             // Pin del LED que se puede controlar.

void setup() {
  Serial.begin(115200);                           // Inicia la comunicación en el puerto serie.
  setup_wifi();                                   // Llama a la función para conectar a Wi-Fi.
  client.setServer(mqtt_server, 1883);            // Configura el servidor MQTT.
  client.setCallback(callback);                   // Configura la función que se llama al recibir mensajes MQTT.
  while (!Serial);                                // Espera a que la conexión Serial esté lista.

  Serial.println("LoRa Receiver");                // Mensaje para indicar que el receptor LoRa está listo.
  LoRa.setPins(SS, RST, DIO0);                    // Configura los pines de LoRa.

  if (!LoRa.begin(915E6)) {                       // Inicia LoRa a una frecuencia de 915 MHz.
    Serial.println("Starting LoRa failed!");      // Mensaje de error si LoRa no inicia.
    while (1);                                    // Detiene el programa si hay error.
  }
}

void setup_wifi() {
  delay(10);                                      // Pausa breve.
  Serial.println();                               // Nueva línea en el monitor serie.
  Serial.print("Connecting to ");                 // Mensaje para mostrar red Wi-Fi.
  Serial.println(ssid);                           // Muestra el nombre de la red.

  WiFi.begin(ssid, password);                     // Inicia la conexión a Wi-Fi.

  while (WiFi.status() != WL_CONNECTED) {         // Espera hasta que se conecte a Wi-Fi.
    delay(500);                                   // Pausa breve mientras se conecta.
    Serial.print(".");                            // Muestra puntos mientras espera conexión.
  }

  Serial.println("");                             // Nueva línea.
  Serial.println("WiFi connected");               // Mensaje cuando está conectado.
  Serial.println("IP address: ");                 // Muestra la dirección IP obtenida.
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");     // Muestra el tema (topic) del mensaje recibido.
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {              // Lee el mensaje carácter por carácter.
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];              // Almacena el mensaje en una variable temporal.
  }
  Serial.println();

  if (String(topic) == "esp32/output") {          // Comprueba si el mensaje es para cambiar el LED.
    Serial.print("Changing output to ");
    if(messageTemp == "on"){                      // Si el mensaje es "on", enciende el LED.
      Serial.println("on");
      digitalWrite(ledPin, LOW);
    }
    else if(messageTemp == "off"){                // Si el mensaje es "off", apaga el LED.
      Serial.println("off");
      digitalWrite(ledPin, HIGH);
    }
  }
}

const char* mqtt_user = SECRET_USER;                  // Usuario MQTT.
const char* mqtt_password = SECRET_PASS;              // Contraseña MQTT.
void reconnect() {
  while (!client.connected()) {                   // Mantiene la conexión a MQTT.
    Serial.print("Attempting MQTT connection...");
    if (client.connect("esp32",mqtt_user,mqtt_password)) {  // Intenta conectar.
      Serial.println("connected");                // Conectado a MQTT.
      client.subscribe("esp32/output");           // Se suscribe al tema para controlar el LED.
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());               // Muestra el error y espera 5 segundos.
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {                      // Si no está conectado, intenta reconectar.
    reconnect();
  }
  client.loop();                                  // Mantiene la conexión MQTT activa.

  int packetSize = LoRa.parsePacket();            // Comprueba si llegó un paquete de datos.
  if (packetSize) {                               // Si hay un paquete de datos LoRa...
    String jsonStr;

    while (LoRa.available()) {                    // Lee los datos LoRa y los almacena como texto JSON.
      jsonStr += (char)LoRa.read();
    }
    Serial.print("Received JSON: ");
    Serial.println(jsonStr);

    StaticJsonDocument<200> lorajson;
    DeserializationError error = deserializeJson(lorajson, jsonStr); // Convierte el texto JSON en un formato de datos.
    
    if (error) {
      Serial.print("Error parsing JSON: ");
      Serial.println(error.c_str());
      return;
    }

    int CS_RX = lorajson["S"]; // se guarda el checksum obtenido del paquete
    id = lorajson["I"]; 
    Temperature = lorajson["T"];                      // Extrae valores de temperatura, humedad, luz, etc.
    Humidity = lorajson["H"];
    Light = lorajson["L"];
    PM01 = lorajson["A"];
    PM25 = lorajson["B"];
    PM10 = lorajson["C"];
    Gas = lorajson["G"];
    Moisture = lorajson["M"];
    
    char txpacket[64];
    
    // Construir el JSON sin el checksum
    sprintf(txpacket, "{\"I\":%d,\"T\":%d,\"H\":%d,\"L\":%d,\"A\":%d,\"B\":%d,\"C\":%d,\"G\":%d,\"M\":%d,\"S\":", 
        id, Temperature, Humidity, Light, PM01, PM25, PM10, Gas, Moisture);

    // Calcular el checksum
    int checksum = 0;
    for (int i = 0; txpacket[i] != '\0'; i++) {
        checksum += txpacket[i]; // Suma los valores ASCII de los caracteres
    }
    if (checksum == CS_RX){
        Serial.println("Checksum Correcto");
        int sensorValues[] = {Temperature, Humidity, Light, PM01, PM25, PM10, Gas, Moisture}; // Valores enteros de sensores.                                        // Valor de pH.
        const char* mqttTopics[] = {"esp32/Temperature","esp32/Humidity","esp32/Light","esp32/PM01","esp32/PM25","esp32/PM10","esp32/Gas","esp32/Moisture"};
        // Crear un documento JSON para el arreglo [id, datoSensor]
        StaticJsonDocument<130> mqttJson;

          // Publicar cada valor del sensor con el ID en el formato [id, datoSensor]
          for (int i = 0; i < sizeof(sensorValues) / sizeof(sensorValues[0]); i++) {
            mqttJson.clear();                           // Limpia el documento JSON para cada iteración.
            mqttJson.add(id);                           // Agrega el ID al arreglo.
            mqttJson.add(sensorValues[i]);             // Agrega el valor del sensor al arreglo.

            char buffer[100];                            // Buffer para almacenar el JSON como texto.
            serializeJson(mqttJson, buffer);            // Serializa el JSON en el buffer.

            client.publish(mqttTopics[i], buffer);      // Publica el JSON en el tema correspondiente.
          }
    }
    else{
      Serial.println("Fallo en Checksum");
    }
  }
}

#include <DFRobot_DHT11.h>  // Libreria para el sensor DHT11 de temperatura y humedad
#include "LoRaWan_APP.h"    // Libreria para comunicacion LoRa
#include "Arduino.h"        // Libreria principal de Arduino

// Definiciones de configuracion de LoRa
#define RF_FREQUENCY 915000000  // Frecuencia de transmision en Hz (915 MHz para region de America)
#define TX_OUTPUT_POWER 5       // Potencia de salida en dBm (5 dBm)
#define LORA_BANDWIDTH 0        // Ancho de banda de LoRa [0: 125 kHz, 1: 250 kHz, 2: 500 kHz]
#define LORA_SPREADING_FACTOR 7 // Factor de expansion de LoRa [SF7..SF12]
#define LORA_CODINGRATE 1       // Tasa de codificacion [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Longitud del preambulo (igual para transmision y recepcion)
#define LORA_SYMBOL_TIMEOUT 0   // Tiempo de espera de simbolo
#define LORA_FIX_LENGTH_PAYLOAD_ON false  // Desactiva longitud fija del paquete
#define LORA_IQ_INVERSION_ON false        // Desactiva la inversion de IQ

#define RX_TIMEOUT_VALUE 1000  // Tiempo de espera para recibir datos
#define BUFFER_SIZE 256        // Tamano del buffer para la carga util de LoRa

// Buffers para almacenar paquetes de transmision y recepcion
char txpacket[BUFFER_SIZE];  
char rxpacket[BUFFER_SIZE];  

// Variable para indicar si LoRa esta inactivo
bool lora_idle = true;  

// Estructura para los eventos de radio LoRa
static RadioEvents_t RadioEvents;

// Declaracion de funciones de manejo de eventos para transmision
void OnTxDone(void);     // Funcion llamada cuando se completa la transmision
void OnTxTimeout(void);  // Funcion llamada si la transmision excede el tiempo

// Inicializacion del sensor DHT11 para temperatura y humedad
DFRobot_DHT11 DHT;
#define DHT11_PIN 2      // Definicion del pin para el sensor DHT11

// Definicion de pines para otros sensores
#define moisture_pin 13  // Sensor de humedad del suelo
#define light_pin 12     // Sensor de luz ambiental
#define gas_pin 36       // Sensor de gas (CO2)

// Funcion para obtener el promedio de 10 lecturas analogicas de un sensor
float obtenerPromedio(int pin) {
  float suma = 0;
  for (int i = 0; i < 10; i++) {
    int valorLectura = analogRead(pin);  // Leer valor del sensor conectado al pin
    suma += valorLectura;  // Sumar las lecturas
    delay(50);  // Espera entre lecturas para evitar lecturas rapidas consecutivas
  }
  return suma / 10;  // Calcular y devolver el promedio
}

// Funcion para obtener el promedio de temperatura y humedad del DHT11
float* promedioDTH11(){
  static float promDTH11[2];  // Array para almacenar temperatura y humedad
  float sumaTemp = 0;
  float sumaHum = 0;
  for (int i = 0; i < 10; i++) {
    DHT.read(DHT11_PIN);  // Leer datos del DHT11
    int valorTemp = DHT.temperature;  // Obtener temperatura
    sumaTemp += valorTemp;  // Sumar lecturas de temperatura
    
    int valorHum = DHT.humidity;  // Obtener humedad
    sumaHum += valorHum;  // Sumar lecturas de humedad
    delay(50);  // Esperar entre lecturas
  }
  // Calcular los promedios y almacenarlos
  promDTH11[0] = sumaTemp / 10;  // Promedio de temperatura
  promDTH11[1] = sumaHum / 10;   // Promedio de humedad
  return promDTH11;  // Retornar el array de promedios
}

void setup() {
  Serial.begin(115200);  // Inicializar la comunicacion serial
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);  // Inicializar el MCU de la placa Heltec
	
  // Configuracion de los eventos de LoRa
  RadioEvents.TxDone = OnTxDone;       // Asignar la funcion de transmision completada
  RadioEvents.TxTimeout = OnTxTimeout; // Asignar la funcion de timeout en transmision
  
  // Inicializar el modulo de radio y configurar los parametros de LoRa
  Radio.Init(&RadioEvents);  // Inicializar LoRa con los eventos configurados
  Radio.SetChannel(RF_FREQUENCY);  // Establecer la frecuencia de LoRa

  // Configurar parametros de transmision LoRa
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000); 
                    // Configuración de pines como entradas
  pinMode(DHT11_PIN, INPUT);     // Pin del sensor DHT11 como entrada
  pinMode(moisture_pin, INPUT);  // Pin del sensor de humedad del suelo como entrada
  pinMode(light_pin, INPUT);     // Pin del sensor de luz como entrada
  pinMode(gas_pin, INPUT);       // Pin del sensor de gas como entrada
}
const float voltageFactor = 5000.0 / 4095.0; // factor de voltage entre el votage de 5 volts en milivots dividido por la resolucion del adc de 12 bits
void loop() {
  // Si LoRa esta inactivo, proceder a enviar un nuevo paquete
  if(lora_idle == true){
    delay(500);  // Espera de 500 ms entre envios
    float* promDTH11 = promedioDTH11();  // Obtener promedios de temperatura y humedad
    float promMoisture = (float)(obtenerPromedio(moisture_pin) - 0.0) / (2250.0 - 0.0) * 100.0; //obtenerPromedio(moisture_pin);  // Obtener promedio de humedad del suelo (float)(obtenerPromedio(moisture_pin); - valorSeco) / (valorMojado - valorSeco) * 100
    float promLuz = obtenerPromedio(light_pin);          // Obtener promedio de luz ambiental
    float promGas = obtenerPromedio(gas_pin);            // Obtener promedio de gas (CO2)

    // Formatear los datos en formato JSON para enviarlos
    sprintf(txpacket, "{\"Temp\":%.1f,\"Hum\":%.1f,\"Moisture\":%.1f,\"Lux\":%.1f,\"CO2\":%.1f}", 
            promDTH11[0], promDTH11[1], promMoisture, promLuz, promGas);

    // Imprimir los datos en la consola serial
    Serial.printf("\r\nSending packet \"%s\", length %d\r\n", txpacket, strlen(txpacket));

    // Enviar el paquete a traves de LoRa
    Radio.Send((uint8_t *)txpacket, strlen(txpacket)); 
    lora_idle = false;  // Marcar LoRa como ocupado durante la transmision
  }

  // Procesar las interrupciones de LoRa (manejo de eventos)
  Radio.IrqProcess();
}

// Funcion llamada cuando la transmision se completa exitosamente
void OnTxDone(void) {
  lora_idle = true;  // Marcar LoRa como inactivo
}

// Funcion llamada si la transmision excede el tiempo de espera
void OnTxTimeout(void) {
  Radio.Sleep();  // Poner el radio en modo de suspension
  lora_idle = true;  // Marcar LoRa como inactivo
}

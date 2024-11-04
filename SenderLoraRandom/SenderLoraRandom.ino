#include "LoRaWan_APP.h"    // Libreria para comunicacion LoRa
#include "Arduino.h"        // Libreria principal de Arduino
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
}

void loop() {
  // Si LoRa esta inactivo, proceder a enviar un nuevo paquete
  if(lora_idle == true){
    delay(1500);  // Espera de 1500 ms entre envios
    int temp = random(15,31);  // Se genera temperatura aleatoria en rango 15-31 grados
    int humidity = random(20,60); // Se genera humedad aleatoria en rango 20-60%
    int light = random(0,100);// Se genera ucantidad luminica aleatoria en 0 a 100 kLux
    int airQuality = random(300,500); // Se genera Calidad del aire de materia particulado aleatoria en rango 300-500 ug/m3 
    float pH = random(50, 80) / 10.0; // Se genera pH de la tierra aleatoria en rango 5.0-8.0 para medir pH
    int WindSpeed = random(5, 20); // Se genera velocidad del viento aleatoria en rango 5-20 km/h
    // Formatear los datos en formato JSON para enviarlos
    sprintf(txpacket, "{\"temp\":%d,\"humidity\":%d,\"light\":%d,\"airQuality\":%d,\"pH\":%.1f,\"WindSpeed\":%d}", 
        temp, humidity, light, airQuality, pH, WindSpeed);

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
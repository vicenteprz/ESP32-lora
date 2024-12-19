#include "LoRaWan_APP.h"    // Libreria para comunicacion LoRa
#include "Arduino.h"        // Libreria principal de Arduino
#include <SoftwareSerial.h>  // Incluye la librería SoftwareSerial para comunicación serial por pines específicos

// VARIABLES DE LORA 
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

//  VARIABLES PARA DATOS  
#define id_ESP32 1          //id que se le quiere asignar al microcontrolador
#define nRX_1 16          // Define el pin 5 como el receptor (RX) para el primer puerto serial
#define nTX_1 17          // Define el pin 4 como el transmisor (TX) para el primer puerto serial
#define moisture_pin 13  // Sensor de humedad del suelo
#define nTIME 3000       // Define un tiempo de espera de 3000 ms (3 segundos)


SoftwareSerial MySer_1(nRX_1, nTX_1);  // Crea un objeto SoftwareSerial para el primer puerto con los pines definidos

// Comandos que se enviarán al sensor PM7003
uint8_t aCMD[2][7] = { 
    {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70}, // Modo pasivo
    {0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71}  // Leer datos
    //{0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71},   // Modo activo
    //{0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73},   // Sleep
    //{0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74}   // Wakeup
};

// Matriz para almacenar los datos de las mediciones (10 filas, 3 columnas)
int aHisto[10][9] = {
    {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0},
    {0, 0, 0}, {0, 0, 0}
};
// Estructura global para almacenar los datos del sensor
uint8_t aData[32];  // Array para almacenar 32 bytes de datos 
//byte nFila = 0;         // Variable para controlar la fila actual en el historial de datos
int nPM01, nPM25, nPM10;  // Variables para almacenar las mediciones de partículas

// Configura el sensor en modo pasivo
void Set_PMS_Pasive(){
    MySer_1.write(aCMD[0], sizeof(aCMD[0]));  // Envía el comando para configurar el modo pasivo
}

// Verifica si la trama de datos recibida es válida, comprobando el checksum
boolean Es_Trama_OK()
{
    int nSuma = 0;  // Variable para almacenar la suma de los bytes de datos
    int nCheckSum = aData[30] * 256 + aData[31];  // Calcula el checksum a partir de los últimos dos bytes
    for (byte i = 0; i < 30; i++)  // Itera sobre los bytes de datos (del 1 al 29)
    {
        nSuma += aData[i];  // Suma cada byte de la trama
    }
    if (nSuma == nCheckSum){  // Si la suma coincide con el checksum, la trama es válida
       // Serial.println("nSuma=  "+String(nSuma)+" nChecksum: "+ String(nCheckSum));
        return true;}
    else{
        Serial.println("nSuma=  "+String(nSuma)+" nChecksum: "+ String(nCheckSum));
        return false;}  // Si no coincide, la trama es incorrecta
}

// Obtiene los datos del sensor, validando y almacenando los resultados en el historial
int* Get_Data_PM()
{
    
    static int pmValues[3]; // se busca retornar un arreglo con los 3 valores de PM
    nPM01 = nPM25 = nPM10 =  0; // Reinicia las variables de medición
    String sLine = "";  // Variable para almacenar la línea de datos en formato string
    byte nFila;
    int errores=0;
    for (nFila = 0; nFila < 10 ; nFila++){
          MySer_1.flush();  // Vacía el buffer de recepción del primer puerto serial
          MySer_1.write(aCMD[1], sizeof(aCMD[1]));  // Envía el comando para leer los datos del sensor
          MySer_1.readBytes(aData, 32);  // Lee los 32 bytes de la respuesta del sensor
          if (Es_Trama_OK())  // Si la trama de datos es válida
          {
            
            // Extrae los datos de las partículas y las almacena en la matriz 'aHisto'
            aHisto[nFila][0] = aData[4] * 256 + aData[5];   // PM1.0
            aHisto[nFila][1] = aData[6] * 256 + aData[7];   // PM2.5
            aHisto[nFila][2] = aData[8] * 256 + aData[9];  // PM10
            // Suma los valores de las mediciones
            nPM01 += aHisto[nFila][0]; nPM25 += aHisto[nFila][1]; nPM10 += aHisto[nFila][2];
            //Serial.println(nFila);
          }
         else {
            Serial.println("Error checksum");  // Si la trama no es válida, muestra un error
            pmValues[0] = pmValues[1] = pmValues[2] = -1; //le asigna un valor de -1 a esta 
            //operacion puesto que C++ no permite manejar numeros NULOS
            // De esta manera en consulta SQL para graficar los datos se da la condicion de solo mostrar
            //valores mayores a -1 para no graficarlos.
            return pmValues;
           }
    delay(500);
    }
    // Calcula el promedio de las mediciones y lo retorna
    nPM01 /= 10; nPM25 /= 10; nPM10 /= 10;
    pmValues[0] =  nPM01 ; // PM1.0
    pmValues[1] =  nPM25 ;// PM2.5
    pmValues[2] =  nPM10; // PM10
    sLine = String(nPM01) + ',' + String(nPM25) + ',' + String(nPM10);  // Crea una cadena con los promedios
    Serial.println(String(nFila)+"PM0.1 , PM2.5 , PM10: "+ sLine);  // Imprime la línea en el puerto serial
    return pmValues;
    
}

// Funcion para obtener el promedio de 10 lecturas analogicas de un sensor
int obtenerPromedio(int pin) {
  int suma = 0;
  for (int i = 0; i < 10; i++) {
    int valorLectura = analogRead(pin);  // Leer valor del sensor conectado al pin
    suma += valorLectura;  // Sumar las lecturas
    delay(50);  // Espera entre lecturas para evitar lecturas rapidas consecutivas
  }
  return suma / 10;  // Calcular y devolver el promedio
}



void setup() {
  randomSeed(1111);
  Serial.begin(9600);  // Inicializar la comunicacion serial
  MySer_1.begin(9600);  // Inicia el primer puerto serial a 9600 baudios
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
  Set_PMS_Pasive();
  pinMode(moisture_pin, INPUT);  // Pin del sensor de humedad del suelo como entrada
}

void loop() {
  // Si LoRa esta inactivo, proceder a enviar un nuevo paquete
  if(lora_idle == true){
    int randomDelay = random(100, 2500);
    delay(3000+randomDelay);  // Espera de 1500 ms entre envios
    int Temperature, Humidity, Light, PM01, PM25, PM10, Gas, Moisture; // Variables donde se guardarán las mediciones
    int* Data_PMS7003= Get_Data_PM(); //Se obtiene arreglo con los valores de los PM
    PM01 = Data_PMS7003[0]; 
    PM25 = Data_PMS7003[1]; 
    PM10 = Data_PMS7003[2];
    Moisture = obtenerPromedio(moisture_pin); // Se obtiene valor de la humedad de la tierra
    
    // Construir el JSON sin el checksum
    sprintf(txpacket, "%d,%d,%d,%d,%d,%d,%d,%d,%d,", 
        id_ESP32, Temperature, Humidity, Light, PM01, PM25, PM10, Gas, Moisture); // Se construye csv a ser enviado

    // Calcular el checksum
    int checksum = 0;
    for (int i = 0; txpacket[i] != '\0'; i++) {
        checksum += txpacket[i]; // Suma los valores ASCII de los caracteres
    }
    // Agregar el checksum al paquete
    sprintf(txpacket + strlen(txpacket), "%d", checksum);

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
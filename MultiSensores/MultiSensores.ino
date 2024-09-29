#include <DFRobot_DHT11.h>
#include "LoRaWan_APP.h"
#include "Arduino.h"


#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             5        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 256 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];


bool lora_idle=true;

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );

DFRobot_DHT11 DHT;
#define DHT11_PIN 2
#define moisture_pin 13
#define light_pin 12
#define gas_pin 36

float obtenerPromedio(int pin) {
  float suma = 0;

  for (int i = 0; i < 10; i++) {
    int valorLectura = analogRead(pin);  // Leer el valor del sensor (entrada analógica)
    suma += valorLectura;  // Sumar las lecturas
    delay(50);  // Esperar un poco entre lecturas para evitar leer muy rápido
  }

  // Calcular el promedio
  float promedio = suma / 10;
  return promedio;
}
float* promedioDTH11(){
  static float promDTH11[2];
  float sumaTemp = 0;
  float sumaHum = 0;
    for (int i = 0; i < 10; i++) {
      DHT.read(DHT11_PIN);
      int valorTemp = DHT.temperature;  // Leer el valor del sensor (entrada analógica)
      sumaTemp += valorTemp;  // Sumar las lecturas
      
      int valorHum = DHT.humidity;
      sumaHum += valorHum;

      delay(50);  // Esperar un poco entre lecturas para evitar leer muy rápido
    }

    // Calcular el promedio
    promDTH11[0] = sumaTemp / 10;
    promDTH11[1] = sumaHum /10;
    return promDTH11;
}
void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
	
    

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
   

}



void loop() {
  if(lora_idle == true){
    delay(500);
    char buffer[100];
    float* promDTH11 = promedioDTH11();
    float promMoisture = obtenerPromedio(moisture_pin);
    float promLuz = obtenerPromedio(light_pin);
    float promGas = obtenerPromedio(gas_pin);
  // Formatear la cadena con sprintf
    sprintf(txpacket, "{\"Temp\":%.1f,\"Hum\":%.1f,\"Moisture\":%.1f,\"Lux\":%.1f,\"CO2\":%.1f}", promDTH11[0],promDTH11[1],promMoisture, promLuz, promGas);


    Serial.printf("\r\nSending packet \"%s\", length %d\r\n", txpacket, strlen(txpacket));
    Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out	
    lora_idle = false;

    
  }
  Radio.IrqProcess( );
}
void OnTxDone( void )
{
	//Serial.println("TX done......");
	lora_idle = true;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    //Serial.println("TX Timeout......");
    lora_idle = true;
}
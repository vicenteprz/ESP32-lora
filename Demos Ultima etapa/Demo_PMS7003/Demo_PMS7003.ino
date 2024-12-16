#include <SoftwareSerial.h>  // Incluye la librería SoftwareSerial para comunicación serial por pines específicos

#define nRX_1 16          // Define el pin 5 como el receptor (RX) para el primer puerto serial
#define nTX_1 17          // Define el pin 4 como el transmisor (TX) para el primer puerto serial

#define nTIME 3000       // Define un tiempo de espera de 3000 ms (3 segundos)

SoftwareSerial MySer_1(nRX_1, nTX_1);  // Crea un objeto SoftwareSerial para el primer puerto con los pines definidos

// Comandos que se enviarán a los sensores
uint8_t aCMD[2][7] = { 
    {0x42, 0x4D, 0xE1, 0x00, 0x00, 0x01, 0x70}, // Modo pasivo
    {0x42, 0x4D, 0xE2, 0x00, 0x00, 0x01, 0x71}  // Leer datos
    //{0x42, 0x4D, 0xE1, 0x00, 0x01, 0x01, 0x71},   // Modo activo
    //{0x42, 0x4D, 0xE4, 0x00, 0x00, 0x01, 0x73},   // Sleep
    //{0x42, 0x4D, 0xE4, 0x00, 0x01, 0x01, 0x74}   // Wakeup
};

// Matriz para almacenar los datos de las mediciones (10 filas, 9 columnas)
int aHisto[10][9] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Estructura global para almacenar los datos del sensor
uint8_t aData[ 32];  // Array para almacenar 32 bytes de datos 
byte nFila = 0;         // Variable para controlar la fila actual en el historial de datos
int nPM01, nPM25, nPM10, nHI03, nHI05, nHI01, nHI25, nHI50, nHI10;  // Variables para almacenar las mediciones de partículas

// Configura el sensor en modo pasivo
void Set_PMS_Pasive()
{
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
        Serial.println("nSuma=  "+String(nSuma)+" nChecksum: "+ String(nCheckSum));
        return true;}
    else{
        Serial.println("nSuma=  "+String(nSuma)+" nChecksum: "+ String(nCheckSum));
        return false;}  // Si no coincide, la trama es incorrecta
}

// Obtiene los datos del sensor, validando y almacenando los resultados en el historial
void Get_Histo()
{
    String sLine = "";  // Variable para almacenar la línea de datos en formato string

    if (Es_Trama_OK())  // Si la trama de datos es válida
    {
        // Extrae los datos de las partículas y las almacena en la matriz 'aHisto'
        aHisto[nFila][0] = aData[4] * 256 + aData[5];   // PM1.0
        aHisto[nFila][1] = aData[6] * 256 + aData[7];   // PM2.5
        aHisto[nFila][2] = aData[8] * 256 + aData[9];  // PM10
        aHisto[nFila][3] = aData[17] * 256 + aData[18]; // HI03
        aHisto[nFila][4] = aData[19] * 256 + aData[20]; // HI05
        aHisto[nFila][5] = aData[21] * 256 + aData[22]; // HI01
        aHisto[nFila][6] = aData[23] * 256 + aData[24]; // HI25
        aHisto[nFila][7] = aData[25] * 256 + aData[26]; // HI50
        aHisto[nFila][8] = aData[27] * 256 + aData[28]; // HI10

        // Suma los valores de las mediciones
        nPM01 += aHisto[nFila][0]; nPM25 += aHisto[nFila][1]; nPM10 += aHisto[nFila][2];
        nHI03 += aHisto[nFila][3]; nHI05 += aHisto[nFila][4]; nHI01 += aHisto[nFila][5];
        nHI25 += aHisto[nFila][6]; nHI50 += aHisto[nFila][7]; nHI10 += aHisto[nFila][8];
        nFila++;  // Incrementa el contador de filas

        if (nFila == 10)  // Cuando se han almacenado 10 muestras
        {
            nFila = 0;  // Reinicia la fila
            // Calcula el promedio de las mediciones y lo muestra por Serial
            nPM01 /= 10; nPM25 /= 10; nPM10 /= 10; nHI03 /= 10; nHI05 /= 10; nHI01 /= 10; 
            nHI25 /= 10; nHI50 /= 10; nHI10 /= 10;
            sLine = String(nPM01) + ',' + String(nPM25) + ',' + String(nPM10);  // Crea una cadena con los promedios
            Serial.println("PM0.1 , PM2.5 , PM10: "+ sLine);  // Imprime la línea en el puerto serial
            
            // Reinicia las variables de medición
            nPM01 = nPM25 = nPM10 = nHI03 = nHI05 = nHI01 = nHI25 = nHI50 = nHI10 = 0;
        }
    }
    else
        Serial.println("Error checksum");  // Si la trama no es válida, muestra un error
}

// Función para obtener los datos del sensor
void Get_Data()
{
    MySer_1.flush();  // Vacía el buffer de recepción del primer puerto serial
    MySer_1.write(aCMD[1], sizeof(aCMD[1]));  // Envía el comando para leer los datos del sensor
    MySer_1.readBytes(aData, 32);  // Lee los 32 bytes de la respuesta del sensor
    
    Get_Histo();  // Procesa los datos recibidos
}

void setup()
{
    Serial.begin(9600);  // Inicia la comunicación serial a 9600 baudios
    MySer_1.begin(9600);  // Inicia el primer puerto serial a 9600 baudios
    Set_PMS_Pasive();  // Configura el sensor en modo pasivo
}

void loop()
{
    
    static unsigned long nStart = millis();  // Inicializa la variable de tiempo

    if (millis() - nStart > nTIME)  // Si han pasado 3000 ms desde el último ciclo
    {
        nStart = millis();  // Reinicia el contador de tiempo
        Get_Data();  // Obtiene los datos del sensor
        
    }
}

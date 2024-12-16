#define LED_PIN 4  // Cambia esto si el pin del LED es diferente

void setup() {
  pinMode(LED_PIN, OUTPUT);  // Configura el pin del LED como salida
  digitalWrite(LED_PIN, HIGH);  // Enciende el LED
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  // Aquí puedes añadir el resto de tu código
}
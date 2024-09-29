
void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
}
const float voltageFactor = 5000.0 / 4095.0; // factor de voltage entre el votage de 5 volts en milivots dividido por la resolucion del adc de 12 bits

void loop() {
  // put your main code here, to run repeatedly:
  int sensorValue = analogRead(2);
  float voltage = sensorValue * voltageFactor;
  Serial.println(voltage/10);
  delay(2000);

}

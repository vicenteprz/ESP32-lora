const float voltageFactor = 5000.0 / 4095.0;
void setup() {
  Serial.begin(115200);

}

void loop() {
float val;
val=analogRead(13);//Read Gas value from analog 0
float voltage = val;
Serial.println(val);//Print the value to serial port
delay(500);

}

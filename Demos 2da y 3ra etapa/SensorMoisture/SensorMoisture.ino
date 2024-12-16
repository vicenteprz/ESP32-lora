void setup(){

 Serial.begin(115200);

}
void loop(){

 Serial.print("Moisture Sensor Value:");
 Serial.println(analogRead(12));
 delay(500);
}
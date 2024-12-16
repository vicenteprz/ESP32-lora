#include <DFRobot_DHT11.h>
DFRobot_DHT11 DHT;
#define DHT11_PIN 15
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(DHT11_PIN, INPUT);
}

void loop() {
  DHT.read(DHT11_PIN);
  Serial.print("temp:");
  Serial.print(DHT.temperature);
  Serial.print("  humi:");
  Serial.println(DHT.humidity);
  delay(1000);
  // put your main code here, to run repeatedly:

}

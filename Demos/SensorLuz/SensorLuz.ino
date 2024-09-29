void setup()
{
  Serial.begin(115200); // open serial port, set the baud rate to 9600 bps
}
void loop()
{
      int val;
      val=analogRead(2);   //connect grayscale sensor to Analog 0
      Serial.println(val,DEC);//print the value to serial
      delay(800);
}
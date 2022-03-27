
#include <SoftwareSerial.h>


SoftwareSerial SoftSerial(12, 13); // RX, TX

void setup() {
    Serial.begin(9600);

  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
    Serial.print(SoftSerial.read());

}

#include <EEPROM.h>

void setup() {
  Serial.begin(9600);
  while (!Serial)
  {
    digitalWrite(13,HIGH);
    delay(1000);
    digitalWrite(13,LOW);
    delay(1000);
  }
  Serial.println("Hello");
  for (int i = 0; i<1024; ++i)
  {
    Serial.println(EEPROM.read(i));
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

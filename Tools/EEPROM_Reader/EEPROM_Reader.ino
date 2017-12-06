#include <EEPROM.h>

void setup() {
  Serial.begin(9600);
  while(!Serial){}
  for (int i = 0; i<1024; ++i) {
    Serial.println(EEPROM.read(i));
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

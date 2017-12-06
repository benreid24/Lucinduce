#include <EEPROM.h>

void setup() {
  Serial.begin(9600);
  while (!Serial){}

  Serial.print("Clearing EEPROM...");
  for (int i = 0; i<1024; ++i)
    EEPROM.write(i,171);
  Serial.println("done\n");
}

void loop() {
  // do nothing
}

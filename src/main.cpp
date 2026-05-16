#include <Arduino.h>

#include "logger.h"

void setup() {
  Serial.begin(115200);
  delay(500);

  initLogger("[CLENA-JARED]");
}

void loop() {
}

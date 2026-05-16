#include <Arduino.h>

#include "logger.h"
#include "ble_interface.h"

#define DEVICE_NAME "CLENA-JARED"

void setup() {
  initLogger(DEVICE_NAME);

  ble_comm_init(DEVICE_NAME, [](int value) {
    LOGI("Received value from BLE client: %d", value);
  });
}

void loop() {
}

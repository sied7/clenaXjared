#include <Arduino.h>

#include "logger.h"
#include "ble_interface.h"

#define DEVICE_NAME "CLENA-JARED"

static void ble_input_callback(int value);

void setup()
{
  initLogger(DEVICE_NAME);

  ble_comm_init(DEVICE_NAME, ble_input_callback);
}

void loop()
{
}

/*------ STATIC FUNCTIONS ------*/
static void ble_input_callback(int value)
{
    LOGI("Received value from BLE client: %d", value);
}

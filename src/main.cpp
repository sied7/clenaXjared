#include <Arduino.h>

#include "logger.h"
#include "ble_interface.h"
#include "motor_drv.h"

#define DEVICE_NAME "CLENA-JARED"

static motor_handle_t motor1_handle = {.id = 1};
static pins_t motor1_pins = {
    .inPin1 = D0,
    .inPin2 = D1,
    .inPin3 = D2,
    .inPin4 = D7};

static bool initializationCompleted = false;
static int current_ble_value = 0;

static void ble_input_callback(int value);

void setup()
{
  ret_status_t status = RET_STATUS_OK;

  logger_init(DEVICE_NAME);

  ble_comm_init(DEVICE_NAME, ble_input_callback);

  status = motor_drv_init(&motor1_handle, motor1_pins);
  if (status != RET_STATUS_OK)
  {
    LOGE("Failed to initialize motor driver");
    return;
  }

  LOGI("Motor driver initialized successfully");

  // Set speed to 10 steps/sec
  status = motor_drv_set_speed(&motor1_handle, 10);
  if (status != RET_STATUS_OK)
  {
    LOGE("Failed to set motor speed");
    return;
  }

  LOGI("Motor speed set successfully");
  initializationCompleted = true;
}

void loop()
{
  if (!initializationCompleted)
  {
    delay(100);
    return;
  }

  // Example: Change motor direction based on BLE input value
  (void)motor_drv_set_drive(&motor1_handle, (motor_dir_t)current_ble_value);
}

/*------ STATIC FUNCTIONS ------*/
static void ble_input_callback(int value)
{
  LOGI("Received value from BLE client: %d", value);
  current_ble_value = value;
}

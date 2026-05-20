#include <Arduino.h>

#include "config.h"
#include "logger.h"
#include "input_handler.h"
#include "motor_drv.h"

static motor_handle_t motor1_handle = {0};
static pins_t motor1_pins = {
    .inPin1 = MOTOR1_PIN1,
    .inPin2 = MOTOR1_PIN2,
    .inPin3 = MOTOR1_PIN3,
    .inPin4 = MOTOR1_PIN4};

static bool initializationCompleted = false;
static uint8_t current_input_state = 0;
static uint8_t last_input_state = 0;

static void process_input_state(uint8_t state);

void setup()
{
  ret_status_t status = RET_STATUS_OK;

  logger_init(DEVICE_NAME);

  input_handler_init();

  status = motor_drv_init(&motor1_handle, motor1_pins);
  if (status != RET_STATUS_OK)
  {
    LOGE("Failed to initialize motor driver");
    return;
  }

  LOGI("Motor driver initialized successfully");

  // Set step delay to achieve desired speed (e.g., 100 steps per second)
  status = motor_drv_set_speed(&motor1_handle, 1);
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
  ret_status_t status = RET_STATUS_OK;

  if (!initializationCompleted)
  {
    delay(100);
    return;
  }
  
  // TODO: Consider refreshing input state (queuing mechanism?) instead of just processing the current state repeatedly
  // refresh_input_state();
  
  status = input_get_current_state(&current_input_state);
  if (status != RET_STATUS_OK)
  {
    return;
  }

  if (last_input_state == 0 && current_input_state == 0)
  {
    delay(100);
    return;
  }

  process_input_state(current_input_state);
  last_input_state = current_input_state;
}

/*------ STATIC FUNCTIONS ------*/
static void process_input_state(uint8_t state)
{
  if (state & INPUT_UP)
  {
    (void)motor_drv_set_drive(&motor1_handle, MOTOR_DRV_DIR_CW);
  }
  else if (state & INPUT_DOWN)
  {
    (void)motor_drv_set_drive(&motor1_handle, MOTOR_DRV_DIR_CCW);
  }
  else
  {
  (void)motor_drv_set_drive(&motor1_handle, MOTOR_DRV_DIR_STOP);
  }
}
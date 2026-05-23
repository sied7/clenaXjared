#include <Arduino.h>

#include "config.h"
#include "logger.h"
#include "system_manager.h"

extern bool system_initialized;

void setup()
{
  ret_status_t status = RET_STATUS_OK;

  logger_init(DEVICE_NAME);

  status = system_manager_init();
  if (status != RET_STATUS_OK)
  {
    LOGE("System initialization failed");
    return;
  }

  LOGI("System initialized successfully");
}

void loop()
{
if (system_initialized)
  {
    update_system();
  }
}

#ifdef OLD_MAIN
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

static void motor_state_change_callback(motor_handle_t *handle)
{
  LOGD("Motor %d position changed: %u", handle->id, handle->position);
}
#endif
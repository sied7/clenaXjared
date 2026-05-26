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

#include "logger.h"
#include "input_handler.h"
#include "ble_interface.h"

static uint8_t current_input_state = 0;

static void ble_input_callback(uint8_t value);

void input_handler_init(void)
{
    ble_comm_init(DEVICE_NAME, ble_input_callback);

    LOGI("Input handler initialized successfully");
}

ret_status_t input_get_current_state(uint8_t *state)
{
    if (state == NULL)
    {
        LOGE("Invalid parameter: state pointer is NULL");
        return RET_STATUS_INVALID_PARAM;
    }

    *state = current_input_state;
    return RET_STATUS_OK;
}

void refresh_input_state(void)
{
    current_input_state = 0U;
}

/*------ STATIC FUNCTIONS ------*/
static void ble_input_callback(uint8_t value)
{
    LOGD("Received BLE input value: %02X", value);
    current_input_state = value;
}

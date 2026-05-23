#include <stdio.h>
#include <stdbool.h>
#include <Preferences.h>

#include "config.h"
#include "system_manager.h"
#include "ble_interface.h"
#include "motor_drv.h"

/*------ GLOBAL VARIABLES ------*/
bool system_initialized = false;

/*------ STATIC VARIABLES ------*/
static motor_handle_t motor1_handle = {0};
static pins_t motor1_pins = {
    .inPin1 = MOTOR1_PIN1,
    .inPin2 = MOTOR1_PIN2,
    .inPin3 = MOTOR1_PIN3,
    .inPin4 = MOTOR1_PIN4};
static motor_handle_t motor2_handle = {0};
static pins_t motor2_pins = {
    .inPin1 = MOTOR2_PIN1,
    .inPin2 = MOTOR2_PIN2,
    .inPin3 = MOTOR2_PIN3,
    .inPin4 = MOTOR2_PIN4};

static uint8_t current_input_state = 0;
static motor_position_t current_motor_position = {0, 0};

/*------ STATIC FUNCTION DECLARATIONS ------*/
static void process_input(void);
static void input_handler_callback(void *data);
static void motor_state_change_callback(motor_handle_t *handle);
static void load_preferences(void);
static void save_preferences(void);

ret_status_t system_manager_init(void)
{
    ret_status_t status = RET_STATUS_OK;

    /* Initialize the BLE communicator */
    status = ble_comm_init(DEVICE_NAME, input_handler_callback);
    if (status != RET_STATUS_OK)
    {
        LOGE("Failed to initialize BLE communicator");
        return RET_STATUS_ERROR;
    }

    /* Initialize components */
    status = motor_drv_init(&motor1_handle, motor1_pins, motor_state_change_callback);
    if (status != RET_STATUS_OK)
    {
        LOGE("Failed to initialize motor driver 1");
        return RET_STATUS_ERROR;
    }
    status = motor_drv_init(&motor2_handle, motor2_pins, motor_state_change_callback);
    if (status != RET_STATUS_OK)
    {
        LOGE("Failed to initialize motor driver 2");
        return RET_STATUS_ERROR;
    }

    (void)motor_drv_set_speed(&motor1_handle, MOTOR1_STEP_DELAY_MS);
    (void)motor_drv_set_speed(&motor2_handle, MOTOR2_STEP_DELAY_MS);

    load_preferences();

    system_initialized = true;
    LOGI("System manager initialized successfully");
    return RET_STATUS_OK;
}

void update_system(void)
{
    if (!system_initialized)
    {
        LOGW("System manager not initialized, skipping update");
        return;
    }

    process_input();
}

void system_manager_deinit(void)
{
    LOGD("Deinitializing system manager");
    ble_comm_deinit();
    system_initialized = false;
}

/*------ STATIC FUNCTION DEFINITIONS ------*/
static void process_input(void)
{
    static uint8_t last_input_state = 0;

    if (last_input_state == 0 && current_input_state == 0)
    {
        return;
    }

    if (current_input_state & INPUT_UP)
    {
        (void)motor_drv_set_drive(&motor1_handle, MOTOR_DRV_DIR_CW);
    }

    if (current_input_state & INPUT_DOWN)
    {
        (void)motor_drv_set_drive(&motor1_handle, MOTOR_DRV_DIR_CCW);
    }

    if (current_input_state & INPUT_RIGHT)
    {
        (void)motor_drv_set_drive(&motor2_handle, MOTOR_DRV_DIR_CW);
    }

    if (current_input_state & INPUT_LEFT)
    {
        (void)motor_drv_set_drive(&motor2_handle, MOTOR_DRV_DIR_CCW);
    }

    if (current_input_state & INPUT_CATCH)
    {
        // TODO Catch action
    }

    last_input_state = current_input_state;
}

static void load_preferences(void)
{
    Preferences preferences;
    preferences.begin(DEVICE_NAME, true); // Read-only mode

    current_motor_position.y = preferences.getInt("motor1_pos", 0);
    current_motor_position.x = preferences.getInt("motor2_pos", 0);

    motor_drv_set_position(&motor1_handle, current_motor_position.y);
    motor_drv_set_position(&motor2_handle, current_motor_position.x);

    preferences.end();

    LOGI("Loaded preferences: motor1_pos=%d, motor2_pos=%d", current_motor_position.y, current_motor_position.x);
}
static void save_preferences(void)
{
    Preferences preferences;
    preferences.begin(DEVICE_NAME, false); // Read-write mode

    (void)preferences.putInt("motor1_pos", current_motor_position.y);
    (void)preferences.putInt("motor2_pos", current_motor_position.x);

    preferences.end();
}

static void input_handler_callback(void *data)
{
    static uint8_t last_value = 0;
    uint8_t value = *(uint8_t *)data;

    if (value != last_value)
    {
        LOGD("Received changed BLE input value: %02X", value);
    }

    current_input_state = value;
    last_value = value;
}

static void motor_state_change_callback(motor_handle_t *handle)
{
    if (handle == NULL || handle->id == 0 || handle->id > MAX_MOTORS)
    {
        LOGE("Invalid motor handle in state change callback");
        return;
    }
    if (handle->id == motor1_handle.id)
    {
        current_motor_position.y = handle->position;
    }
    else if (handle->id == motor2_handle.id)
    {
        current_motor_position.x = handle->position;
    }

    save_preferences();
    LOGD("Current motor positions updated: motor1_pos=%d, motor2_pos=%d", current_motor_position.y, current_motor_position.x);
}
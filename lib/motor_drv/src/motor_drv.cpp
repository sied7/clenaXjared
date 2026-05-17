#include "motor_drv.h"
#include "logger.h"

#define MAX_MOTORS 2

// Min/Max delay (in ms or microseconds)
#define MIN_DELAY 2  // fastest
#define MAX_DELAY 20 // slowest
#define MAXSPEED 1000 // steps per second

#define HANDLE_CHECK(handle)                    \
    do                                          \
    {                                           \
        if (!(handle))                          \
        {                                       \
            LOGE("Invalid motor handle(NULL)"); \
            return RET_STATUS_INVALID_PARAM;    \
        }                                       \
    } while (0)

typedef struct
{
    motor_handle_t *handle;
    int current_step;
    unsigned long last_step_time;
    uint16_t step_delay; // Speed in steps per second
} motor_drv_context_t;

/// @brief  Step sequence for controlling a 4-pin stepper motor in full-step mode
static const int steps[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}};

static motor_drv_context_t motor_ctx[MAX_MOTORS];

static const int stepCOUNT = sizeof(steps) / sizeof(steps[0]);
static void setup_motor_pins(const motor_handle_t *handle);
static void set_motor_pins(const motor_handle_t *handle, const int step[4]);

ret_status_t motor_drv_init(motor_handle_t *handle, pins_t pins)
{
    HANDLE_CHECK(handle);

    for (size_t i = 0; i < MAX_MOTORS; i++)
    {
        if (!motor_ctx[i].handle)
        {
            motor_ctx[i].handle = handle;
            motor_ctx[i].current_step = 0;
            motor_ctx[i].last_step_time = 0;
            motor_ctx[i].step_delay = 0;

            // Assign a unique ID starting from 1
            handle->id = i + 1;
            handle->pins = pins;

            setup_motor_pins(handle);

            LOGI("Motor driver initialized with ID %d on pins: %d, %d, %d, %d",
                 handle->id, pins.inPin1, pins.inPin2, pins.inPin3, pins.inPin4);

            return RET_STATUS_OK;
        }
    }

    LOGE("Motor driver initialization failed: maximum number of motors reached");
    return RET_STATUS_OUT_OF_BOUNDS;
}

ret_status_t motor_drv_set_speed(motor_handle_t *handle, uint16_t speed)
{
    uint16_t step_delay = 0U;

    HANDLE_CHECK(handle);

    // Clamp speed to avoid invalid values
    if (speed > MAXSPEED)
        speed = MAXSPEED;

    // Convert speed → delay (inverse relationship)
    step_delay = MAX_DELAY - (speed / (float)MAXSPEED) * (MAX_DELAY - MIN_DELAY);

    for (size_t i = 0; i < MAX_MOTORS; i++)
    {
        if (motor_ctx[i].handle == handle)
        {
            motor_ctx[i].step_delay = step_delay;
            return RET_STATUS_OK;
        }
    }

    LOGE("Motor driver set speed failed: handle not found");
    return RET_STATUS_NOT_FOUND;
}

ret_status_t motor_drv_set_drive(motor_handle_t *handle, motor_dir_t direction)
{
    int motor_index = -1;
    unsigned long current_time = 0U;
    motor_drv_context_t *current_motor = NULL;

    HANDLE_CHECK(handle);

    if (direction != MOTOR_DRV_DIR_STOP &&
        direction != MOTOR_DRV_DIR_CW &&
        direction != MOTOR_DRV_DIR_CCW)
    {
        LOGE("Motor driver set drive failed: invalid direction:%d", direction);
        return RET_STATUS_INVALID_PARAM;
    }

    for (size_t i = 0; i < MAX_MOTORS; i++)
    {
        if (motor_ctx[i].handle == handle)
        {
            motor_index = i;
            break;
        }
    }

    if (motor_index == -1)
    {
        LOGE("Motor driver set drive failed: handle not found");
        return RET_STATUS_NOT_FOUND;
    }

    current_motor = &motor_ctx[motor_index];

    current_time = millis();
    if (current_time - current_motor->last_step_time >= current_motor->step_delay)
    {
        current_motor->last_step_time = current_time;

        if (direction == MOTOR_DRV_DIR_CW)
        {
            // LOGD("Motor %d: Step CW to step %d", current_motor->handle->id, current_motor->current_step);
            current_motor->current_step = (current_motor->current_step + 1) % stepCOUNT;
        }
        else if (direction == MOTOR_DRV_DIR_CCW)
        {
            // LOGD("Motor %d: Step CCW to step %d", current_motor->handle->id, current_motor->current_step);
            current_motor->current_step = (current_motor->current_step - 1 + stepCOUNT) % stepCOUNT;
        }
        else if (direction == MOTOR_DRV_DIR_STOP)
        {
            // LOGD("Motor %d: Stop", current_motor->handle->id);
            // Do nothing, just keep the current step to hold the position
        }

        set_motor_pins(handle, steps[current_motor->current_step]);

        return RET_STATUS_OK;
    }

    // Implementation for setting motor drive direction
    return RET_STATUS_OK;
}

ret_status_t motor_drv_release(motor_handle_t *handle)
{
    HANDLE_CHECK(handle);

    for (size_t i = 0; i < MAX_MOTORS; i++)
    {
        if (motor_ctx[i].handle == handle)
        {
            motor_ctx[i].handle = NULL;
            return RET_STATUS_OK;
        }
    }

    LOGE("Motor driver release failed: handle not found");
    return RET_STATUS_NOT_FOUND;
}

/*------ STATIC FUNCTIONS ------*/
static void setup_motor_pins(const motor_handle_t *handle)
{
    pinMode(handle->pins.inPin1, OUTPUT);
    pinMode(handle->pins.inPin2, OUTPUT);
    pinMode(handle->pins.inPin3, OUTPUT);
    pinMode(handle->pins.inPin4, OUTPUT);
}

static void set_motor_pins(const motor_handle_t *handle, const int step[4])
{
    digitalWrite(handle->pins.inPin1, step[0]);
    digitalWrite(handle->pins.inPin2, step[1]);
    digitalWrite(handle->pins.inPin3, step[2]);
    digitalWrite(handle->pins.inPin4, step[3]);
}

#ifndef __MOTOR_DRV_H__
#define __MOTOR_DRV_H__ 

#include <Arduino.h>

#include <customdef.h>

/// @brief  Enumeration for motor rotation direction.
typedef enum
{
    MOTOR_DRV_DIR_STOP = 0, ///< Stop the motor
    MOTOR_DRV_DIR_CW = 1,  ///< Clockwise direction
    MOTOR_DRV_DIR_CCW = 2, ///< Counter-clockwise direction
} motor_dir_t;

/// @brief  Motor driver library for controlling a 4-pin motor using an ESP32-C3 microcontroller.
typedef struct
{
    uint8_t inPin1;
    uint8_t inPin2;
    uint8_t inPin3;
    uint8_t inPin4;
} pins_t;

/// @brief Handle structure for motor driver, containing an ID and the associated pins.
typedef struct
{
    uint8_t id;
    pins_t pins;
} motor_handle_t;

ret_status_t motor_drv_init(motor_handle_t *handle, pins_t pins);
ret_status_t motor_drv_set_speed(motor_handle_t *handle, uint16_t speed);
ret_status_t motor_drv_set_drive(motor_handle_t *handle, motor_dir_t direction);
ret_status_t motor_drv_release(motor_handle_t *handle);

#endif /* __MOTOR_DRV_H__ */
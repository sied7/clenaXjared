#ifndef __BLE_INTERFACE_H__
#define __BLE_INTERFACE_H__

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <stdint.h>

#include "customdef.h"
#include "logger.h"

typedef void (*ble_callback)(void*);

ret_status_t ble_comm_init(const char *bleName, ble_callback onChangeValue);
ret_status_t set_motor_speed_write_callback(ble_callback callback);
ret_status_t set_motor_position(motor_position_t currentPosition);
void ble_comm_deinit(void);

#endif // __BLE_INTERFACE_H__
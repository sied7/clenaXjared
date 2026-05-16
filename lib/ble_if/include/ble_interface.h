#ifndef __BLE_INTERFACE_H__
#define __BLE_INTERFACE_H__

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

typedef void (*ble_callback)(int);

void ble_comm_init(const char* bleName, ble_callback onChangeValue);
void ble_comm_deinit(void);

#endif // __BLE_INTERFACE_H__
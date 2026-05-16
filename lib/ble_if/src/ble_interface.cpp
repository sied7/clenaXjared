#include <stdbool.h>

#include "ble_interface.h"
#include "logger.h"

// ADDRESS = "68:67:25:EC:83:4A"  
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

static ble_callback onDataCallback = NULL;

static TaskHandle_t main_task_handle = NULL;
static volatile bool main_loop_active = false;

static BLEServer* pServer = NULL;
static BLECharacteristic* pCharacteristic = NULL;
static volatile bool deviceConnected = false;
static bool oldDeviceConnected = false;

static void ble_connectingMode(bool connected);
static void main_ble_loop(void *params);

static int writeData = -1;

class MyServerCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer)
    {
        deviceConnected = true;
        ble_connectingMode(deviceConnected);
    };

    void onDisconnect(BLEServer* pServer)
    {
        deviceConnected = false;
        ble_connectingMode(deviceConnected);
    }

};

class MyCallbacks: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
      writeData = *((int*)pCharacteristic->getData());
      if (onDataCallback != NULL)
      {
         onDataCallback(writeData);
      }
    }
};

void ble_comm_init(const char* bleName, ble_callback clientCallback)
{
    LOGI("Initializing BLE communicator with name: %s", bleName);
    BLEDevice::init(bleName);

    onDataCallback = clientCallback;

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ   |
                        BLECharacteristic::PROPERTY_NOTIFY |
                        BLECharacteristic::PROPERTY_WRITE
                        );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue(writeData);

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    (void)xTaskCreatePinnedToCore(main_ble_loop, "main_ble_loop", 4096, NULL, 1, NULL, 0);
    LOGI("BLE Device Address: %s", BLEDevice::getAddress().toString().c_str());
}

void ble_comm_deinit(void)
{
    LOGD("Deinitializing BLE communicator");
    main_loop_active = false;
    if (main_task_handle != NULL)    {
        vTaskDelete(main_task_handle);
        main_task_handle = NULL;
    }

    pServer->removeService(pServer->getServiceByUUID(SERVICE_UUID));
    BLEDevice::deinit();
    LOGD("BLE communicator deinitialized");
}

///* INTERNAL FUNCTIONS *///
static void ble_connectingMode(bool connected)
{
    if (connected == true)
    {
      LOGI("Device connected");
    }
    else
    {
      LOGI("Device disconnected");
    }
}

static void main_ble_loop(void *params)
{
    (void)params;

    LOGI("Starting main BLE loop");
    main_loop_active = true;
    while (main_loop_active == true)
    {
      if (!deviceConnected && oldDeviceConnected)
      {
          vTaskDelay(1000 / portTICK_PERIOD_MS); // give the bluetooth stack the chance to get things ready
          pServer->startAdvertising(); // restart advertising
          oldDeviceConnected = deviceConnected;
      }
      // connecting
      if (deviceConnected && !oldDeviceConnected)
      {
          // do stuff here on connecting
          oldDeviceConnected = deviceConnected;
      }

      vTaskDelay(5 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}
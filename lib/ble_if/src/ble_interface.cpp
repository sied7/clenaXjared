#include <stdbool.h>

#include "ble_interface.h"

// ADDRESS = "68:67:25:EC:83:4A"
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define INPUT_C_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define MOTORSPD_C_UUID "2077dde7-c78e-4ed6-8452-d5a89f15ab9a"
#define MOTORPOS_C_UUID "6eb55e16-fab2-47d2-acd4-6d50a4b1ed04"

enum CharType {
    CHAR_CMD,
    CHAR_MOTOR_SPEED,
    CHAR_MOTOR_POSITION,
};

class TypedCharacteristic : public BLECharacteristic {
public:
    CharType type;

    TypedCharacteristic(const char* uuid, uint32_t props, CharType t)
        : BLECharacteristic(uuid, props), type(t) {}
};

static ble_callback onInputWriteCallback = NULL;
static ble_callback onMotorSpeedWriteCallback = NULL;
static ble_callback onMotorPositionWriteCallback = NULL;

static TaskHandle_t main_task_handle = NULL;
static volatile bool main_loop_active = false;

static BLEServer *pServer = NULL;
static TypedCharacteristic *pInputCharacteristic = NULL;
static TypedCharacteristic *pMotorSpeedCharacteristic = NULL;
static TypedCharacteristic *pMotorPositionCharacteristic = NULL;
static volatile bool deviceConnected = false;
static bool oldDeviceConnected = false;
static bool initialized = false;
static uint8_t defaultInputData = 0;

static void ble_connectingMode(bool connected);
static void main_ble_loop(void *params);
static void initializeCharacteristics(BLEService *pService);

static void handleWriteInput(const uint8_t* data, size_t len);
static void handleWriteMotorSpeed(const uint8_t* data, size_t len);
static void handleWriteMotorPosition(const uint8_t* data, size_t len);

class ServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        ble_connectingMode(deviceConnected);
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        ble_connectingMode(deviceConnected);
    }
};

class DataCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *targetCharacteristic)
    {
        TypedCharacteristic* tc = (TypedCharacteristic*)targetCharacteristic;
        std::string raw = tc->getValue();
        const uint8_t* data = (uint8_t*)raw.data();
        size_t len = raw.length();

        switch (tc->type)
        {
            case CHAR_CMD:
                handleWriteInput(data, len);
                break;
            case CHAR_MOTOR_SPEED:
                handleWriteMotorSpeed(data, len);
                break;
            case CHAR_MOTOR_POSITION:
                handleWriteMotorPosition(data, len);
                break;
            default:
                LOGW("Received write for unknown characteristic type");
                break;
        }
    }
};

ret_status_t ble_comm_init(const char *bleName, ble_callback writeCallback)
{
    LOGI("Initializing BLE communicator with name: %s", bleName);
    BLEDevice::init(bleName);

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    if (pServer == nullptr)
    {
        LOGE("Failed to create BLE server");
        return RET_STATUS_ERROR;
    }

    pServer->setCallbacks(new ServerCallbacks());
    
    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);
    if (pService == nullptr)
    {
        LOGE("Failed to create BLE service");
        delete pServer;
        return RET_STATUS_ERROR;
    }

    initializeCharacteristics(pService);
    
    pService->start();
    
    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    if (pAdvertising == nullptr)
    {
        LOGE("Failed to get BLE advertising");
        delete pService;
        delete pServer;
        return RET_STATUS_ERROR;
    }

    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    onInputWriteCallback = writeCallback;
    initialized = true;

    (void)xTaskCreatePinnedToCore(main_ble_loop, "main_ble_loop", 4096, NULL, 1, NULL, 0);
    LOGI("BLE Device Address: %s", BLEDevice::getAddress().toString().c_str());

    return RET_STATUS_OK;
}

ret_status_t set_motor_speed_write_callback(ble_callback callback)
{
    if (!initialized)
    {
        LOGE("Cannot set motor speed write callback: BLE communicator not initialized");
        return RET_STATUS_ERROR;
    }

    onMotorSpeedWriteCallback = callback;
    return RET_STATUS_OK;
}

ret_status_t set_motor_position(motor_position_t currentPosition)
{
    if (!initialized)
    {
        LOGE("Cannot set motor position: BLE communicator not initialized");
        return RET_STATUS_ERROR;
    }

    if (pMotorPositionCharacteristic == nullptr)
    {
        LOGE("Motor position characteristic not initialized");
        return RET_STATUS_ERROR;
    }

    pMotorPositionCharacteristic->setValue((uint8_t*)&currentPosition, sizeof(motor_position_t));
    pMotorPositionCharacteristic->notify();

    return RET_STATUS_OK;
}

void ble_comm_deinit(void)
{
    LOGD("Deinitializing BLE communicator");
    main_loop_active = false;
    if (main_task_handle != NULL)
    {
        vTaskDelete(main_task_handle);
        main_task_handle = NULL;
    }

    if (pServer != nullptr)
    {
        BLEService* service = pServer->getServiceByUUID(SERVICE_UUID);
        if (service != nullptr)
        {
            service->stop();
            delete service;
        }
        pServer->removeService(pServer->getServiceByUUID(SERVICE_UUID));
        delete pServer;
        if (pInputCharacteristic != nullptr)
            delete pInputCharacteristic;
        if (pMotorSpeedCharacteristic != nullptr)
            delete pMotorSpeedCharacteristic;
        if (pMotorPositionCharacteristic != nullptr)
            delete pMotorPositionCharacteristic;
    }

    BLEDevice::deinit();
    LOGD("BLE communicator deinitialized");
}

/*------ STATIC FUNCTIONS ------*/
static void main_ble_loop(void *params)
{
    (void)params;

    LOGI("Starting main BLE loop");
    main_loop_active = true;
    while (main_loop_active == true)
    {
        if (!deviceConnected && oldDeviceConnected)
        {
            // give the bluetooth stack the chance to get things ready
            vTaskDelay(1000 / portTICK_PERIOD_MS);

            // restart advertising
            pServer->startAdvertising();
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

static void initializeCharacteristics(BLEService *pService)
{
    pInputCharacteristic = new TypedCharacteristic(
        INPUT_C_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE,
        CHAR_CMD
    );

    if (pInputCharacteristic == nullptr)
    {
        LOGE("Failed to create input characteristic");
    }
    else
    {
        pInputCharacteristic->setCallbacks(new DataCallbacks());
        pInputCharacteristic->setValue(&defaultInputData, 1);
        pService->addCharacteristic(pInputCharacteristic);
    }

    pMotorSpeedCharacteristic = new TypedCharacteristic(
        MOTORSPD_C_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE,
        CHAR_MOTOR_SPEED
    );

    if (pMotorSpeedCharacteristic == nullptr)
    {
        LOGE("Failed to create motor speed characteristic");
    }
    else
    {
        pMotorSpeedCharacteristic->setCallbacks(new DataCallbacks());
        pService->addCharacteristic(pMotorSpeedCharacteristic);
    }

    pMotorPositionCharacteristic = new TypedCharacteristic(
        MOTORPOS_C_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY,
        CHAR_MOTOR_POSITION
    );

    if (pMotorPositionCharacteristic == nullptr)
    {
        LOGE("Failed to create motor position characteristic");
    }
    else
    {
        pMotorPositionCharacteristic->setCallbacks(new DataCallbacks());
        pService->addCharacteristic(pMotorPositionCharacteristic);
    }
}

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

static void handleWriteInput(const uint8_t* data, size_t len)
{
    if (len < 1)
    {
        LOGW("Received write with no data");
        return;
    }

    uint8_t value = data[0];
    if (onInputWriteCallback != NULL)
    {
        onInputWriteCallback(&value);
    }
    else
    {
        LOGW("No input write callback registered");
    }
}
static void handleWriteMotorSpeed(const uint8_t* data, size_t len)
{
    if (len != sizeof(uint32_t)) return;

    uint32_t speed;
    (void)memcpy(&speed, data, sizeof(uint32_t));

    if (onMotorSpeedWriteCallback != NULL)
    {
        onMotorSpeedWriteCallback(&speed);
    }
    else
    {
        LOGW("No motor speed write callback registered");
    }
}
static void handleWriteMotorPosition(const uint8_t* data, size_t len)
{
    if (len != sizeof(motor_position_t)) return;

    motor_position_t pos;
    (void)memcpy(&pos, data, sizeof(motor_position_t));

    if (onMotorPositionWriteCallback != NULL)
    {
        onMotorPositionWriteCallback(&pos);
    }
    else
    {
        LOGW("No motor position write callback registered");
    }
}
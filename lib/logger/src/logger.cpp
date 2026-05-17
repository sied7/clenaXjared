#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Syslog.h>
#include <ESP32Ping.h>

#include "hidden.h"
#include "customdef.h"
#include "logger.h"

#define USE_SERIAL_LOGGING

WiFiUDP udpClient;
Syslog syslog(udpClient, SYSLOG_PROTO_IETF);

static bool connectedToWifi = false;
static bool reconnectingToWifi = false;
static bool syslogServerConnected = false;
static volatile bool monitoring_on = false;
static unsigned long lastSyslogCheck = 0;

static void connectSyslogServer(void);
static void wifiEventHandler(WiFiEvent_t event, WiFiEventInfo_t info);
static void connectionMonitoringThread(void *params);

void logger_init(const char *processName)
{
#ifdef USE_SERIAL_LOGGING
    Serial.begin(115200);
    delay(500);
#endif

    syslog.deviceHostname("xiao-esp32c3");
    syslog.appName(processName);
    syslog.defaultPriority(LOG_USER | LOG_INFO);

    (void)xTaskCreatePinnedToCore(connectionMonitoringThread, "WiFiConnect",
                                  4096, (void *)processName, 1, NULL, 0);

    return;
}

void printlog(uint16_t severity, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);

    if (syslogServerConnected)
    {
        (void)syslog.vlogf_P((LOG_LOCAL2 | severity), msg, args);
    }

#ifdef USE_SERIAL_LOGGING
    LOG_SERIAL(msg, args);
#endif
    va_end(args);
}

/*------ STATIC FUNCTIONS ------*/
static void connectionMonitoringThread(void *params)
{
    bool syslogReachable = false;

    WiFi.onEvent(wifiEventHandler);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    vTaskDelay(100 / portTICK_PERIOD_MS);

    WiFi.begin(SSID, WIFI_PASSWORD);

    monitoring_on = true;
    while (monitoring_on)
    {
        // Wait until WiFi is connected
        while (!connectedToWifi || reconnectingToWifi)
        {
            LOGD("Connecting to WiFi...");
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }

        // Check syslog server every 5 seconds
        if (millis() - lastSyslogCheck > 5000)
        {
            lastSyslogCheck = millis();

            syslogReachable = Ping.ping("192.168.68.106", 1);

            if (!syslogReachable && syslogServerConnected)
            {
                LOGD("Syslog server lost, will retry...");
                syslogServerConnected = false;
            }

            if (syslogReachable && !syslogServerConnected)
            {
                LOGD("Syslog server reachable again, reconnecting...");
                connectSyslogServer();
            }
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

static void wifiEventHandler(WiFiEvent_t event, WiFiEventInfo_t info)
{
    LOGD("WiFi event: %s(%d)", WiFi.eventName(event), (int)event);
    if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP)
    {
        connectedToWifi = true;
        reconnectingToWifi = false;
        connectSyslogServer();
        LOGI("Connected to WiFi network: %s", SSID);
    }
    else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED)
    {
        reconnectingToWifi = true;
        if (syslogServerConnected)
        {
            syslogServerConnected = false;
            LOGW("Disconnected from WiFi network");
        }
    }
}

static void connectSyslogServer(void)
{
    syslog.server("192.168.68.106", 514);
    syslogServerConnected = true;
    LOGI("Connected to syslog server!");
}
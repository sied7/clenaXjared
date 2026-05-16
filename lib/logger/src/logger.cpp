#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Syslog.h>

#include "customdef.h"
#include "logger.h"

WiFiUDP udpClient;
Syslog syslog(udpClient, SYSLOG_PROTO_IETF);

static bool connectedToWifi = false;
static bool syslogInitialized = false;

void connectionInitialization(void* params) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    vTaskDelay(100 / portTICK_PERIOD_MS);

    WiFi.begin("Ayayu", "feelmysoul");
    for (;;) {
        if (WiFi.status() == WL_CONNECTED) {
            connectedToWifi = true;
            break;
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    syslog.server("192.168.68.106", 514);
    syslog.deviceHostname("xiao-esp32c3");
    syslog.appName((const char*)params);
    syslog.defaultPriority(LOG_USER | LOG_INFO);

    syslogInitialized = true;
    LOGI("Connected to syslog server!");
    vTaskDelete(NULL);
}

void initLogger(const char* processName) {
    Serial.begin(115200);
    delay(500);

    (void)xTaskCreatePinnedToCore(
        connectionInitialization,
        "WiFiConnect",
        4096,
        (void*)processName,
        1,
        NULL,
        0
    );
}

void printlog(uint16_t severity, const char* msg, ...) {
    va_list args;
    va_start(args, msg);

    if (syslogInitialized) {
        (void)syslog.vlogf_P((LOG_LOCAL2 | severity), msg, args);
    }

    LOG_SERIAL(msg, args);
    va_end(args);
}
#include "wifi_config.h"

SemaphoreHandle_t xBinarySemaphoreInternet = nullptr;

static const TickType_t WIFI_CONNECT_TIMEOUT = pdMS_TO_TICKS(15000);

void startAP()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(String(SSID_AP), String(PASS_AP));
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void startSTA()
{
    if (WIFI_SSID.isEmpty())
    {
        Serial.println("WiFi SSID is empty, switching to AP mode");
        startAP();
        return;
    }

    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to WiFi SSID: ");
    Serial.println(WIFI_SSID);

    if (WIFI_PASS.isEmpty())
    {
        WiFi.begin(WIFI_SSID.c_str());
    }
    else
    {
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }

    TickType_t startTick = xTaskGetTickCount();
    while (WiFi.status() != WL_CONNECTED)
    {
        if ((xTaskGetTickCount() - startTick) >= WIFI_CONNECT_TIMEOUT)
        {
            Serial.println("WiFi connect timeout, starting AP mode");
            startAP();
            return;
        }
        Serial.println("Waiting for WiFi connection...");
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
    // Give a semaphore here
    if (xBinarySemaphoreInternet != nullptr) {
        xSemaphoreGive(xBinarySemaphoreInternet);
    }
}

bool Wifi_reconnect()
{
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        return true;
    }
    startSTA();
    return false;
}
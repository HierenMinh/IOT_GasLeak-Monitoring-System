// Webserver task: serves files from LittleFS and broadcasts sensor data via WebSocket
#include "task_webserver.h"
#include "task_sensor.h"
#include "wifi.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "check_info.h"

static AsyncWebServer* server = nullptr;
static AsyncWebSocket* ws = nullptr;

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type,
               void * arg, uint8_t *data, size_t len){
    if(type == WS_EVT_CONNECT){
        Serial.printf("WebSocket client connected: %u\n", client->id());
    } else if(type == WS_EVT_DISCONNECT){
        Serial.printf("WebSocket client disconnected: %u\n", client->id());
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            char message[len + 1];
            memcpy(message, data, len);
            message[len] = '\0';

            Serial.print("WebSocket payload: ");
            Serial.println(message);

            StaticJsonDocument<512> doc;
            DeserializationError error = deserializeJson(doc, message);
            if (error) {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                return;
            }

            const char *page = doc["page"];
            if (page != nullptr && strcmp(page, "setting") == 0) {
                JsonObject value = doc["value"];
                if (value.isNull()) {
                    Serial.println("Invalid settings payload: missing value object");
                    return;
                }

                String ssid = value["ssid"] | "";
                String password = value["password"] | "";
                String token = value["token"] | "";
                String server = value["server"] | "";
                String port = value["port"] | "";

                Serial.println("Saving Wi-Fi and CoreIoT settings...");
                Save_info_File(ssid, password, token, server, port);
            }
        }
    }
}

void webserver_task(void *pvParameters){
    sensor_handle_t sensor = (sensor_handle_t) pvParameters;

    if (!LittleFS.begin()){
        Serial.println("LittleFS mount failed");
    }

    server = new AsyncWebServer(80);
    ws = new AsyncWebSocket("/ws");
    ws->onEvent(onWsEvent);
    server->addHandler(ws);

    // Serve static files from /data (LittleFS root)
    server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server->begin();
    Serial.println("Webserver started");

    sensor_data_t data;
    while(1){
        if (sensor_get_data(sensor, &data, WEBSERVER_BIT, portMAX_DELAY)){
            char buf[256];
            int n = snprintf(buf, sizeof(buf), "{\"type\":\"sensor_data\",\"temperature\":%.2f,\"humidity\":%.2f,\"gas\":%.2f}",
                             data.temperature, data.humidity, data.gas);
            if (n > 0) {
                ws->textAll(buf);
            }
        } else {
            // No new data for webserver: yield to avoid tight-loop and watchdog
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void webserver_init(sensor_handle_t sensor){
    xTaskCreate(webserver_task, "webserver", 8192, (void*)sensor, 1, NULL);
}

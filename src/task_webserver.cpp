// Webserver task: serves files from LittleFS and broadcasts sensor data via WebSocket
#include "task_webserver.h"
#include "task_sensor.h"
#include "wifi.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

static AsyncWebServer* server = nullptr;
static AsyncWebSocket* ws = nullptr;

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type,
               void * arg, uint8_t *data, size_t len){
    if(type == WS_EVT_CONNECT){
        Serial.printf("WebSocket client connected: %u\n", client->id());
    } else if(type == WS_EVT_DISCONNECT){
        Serial.printf("WebSocket client disconnected: %u\n", client->id());
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
        }
    }
}

void webserver_init(sensor_handle_t sensor){
    xTaskCreate(webserver_task, "webserver", 8192, (void*)sensor, 1, NULL);
}

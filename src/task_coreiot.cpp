#include "task_coreiot.h"

const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

static void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client", CORE_IOT_TOKEN.c_str(), NULL))
        {
            Serial.println("Connected to CoreIOT Server!");
            client.subscribe("v1/devices/me/rpc/request/+");
            Serial.println("Subscribed to v1/devices/me/rpc/request/+");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
}

static void callback(char *topic, byte *payload, unsigned int length){
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    Serial.print("Payload: ");
    Serial.println(message);

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    // const char* method = doc["method"];
    // if (method == nullptr) return;

    // if (strcmp(method, "Pump_control") == 0){
    //     Serial.println("RPC: Pump_control received");
    //     if (doc["params"].is<bool>()){
    //         bool pump_state = doc["params"];
    //         pinMode(PUMP_PIN, OUTPUT);
    //         digitalWrite(PUMP_PIN, pump_state ? HIGH : LOW);
    //         String statusPayload = String("{\"pump_state\":") + (pump_state ? "true" : "false") + String("}");
    //         client.publish("v1/devices/me/attributes", statusPayload.c_str());
    //         Serial.print("Pump state set to: ");
    //         Serial.println(pump_state ? "ON" : "OFF");
    //     } else {
    //         Serial.println("Invalid parameters for Pump_control");
    //     }
    // } else {
    //     Serial.print("Unknown method received: ");
    //     Serial.println(method);
    // }
}

void setup_coreiot() {
    while (1){
        if (xSemaphoreTake(xBinarySemaphoreInternet, pdMS_TO_TICKS(10000)) == pdTRUE) {
            Serial.println("Internet connection established, setting up MQTT...");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        Serial.print(".");
    }
    Serial.println("Connected!");

    client.setServer(CORE_IOT_SERVER.c_str(), mqtt_port);
    client.setCallback(callback);
}

void coreiot_init(sensor_handle_t sensor){
    xTaskCreate(task_coreiot, "coreiot", 8192, (void*)sensor, 1, NULL);
}

void task_coreiot(void *pvParameters)
{
    sensor_handle_t sensor = (sensor_handle_t)pvParameters;
    setup_coreiot();

    sensor_data_t data;
    TickType_t lastPublishTick = 0;
    const TickType_t publishInterval = pdMS_TO_TICKS(2000);

    while (1){
        if (!client.connected()){
            reconnect();
        }
        client.loop();

        // Wait for next sensor data for CoreIoT
        if (sensor_get_data(sensor, &data, COREIOT_BIT, portMAX_DELAY)){
            TickType_t now = xTaskGetTickCount();
            if ((now - lastPublishTick) < publishInterval) {
                /* Too soon to publish again — run MQTT loop and yield
                   briefly to avoid starving other tasks / watchdog. */
                client.loop();
                vTaskDelay(pdMS_TO_TICKS(10));
                continue;
            }

            StaticJsonDocument<256> doc;
            doc["temperature"] = data.temperature;
            doc["humidity"] = data.humidity;
            doc["gas"] = data.gas;
            doc["ratio"] = data.ratio;
            doc["score"] = data.score;

            char jsonBuffer[256];
            serializeJson(doc, jsonBuffer);

            if (client.connected()){
                bool ok = client.publish("v1/devices/me/telemetry", jsonBuffer);
                if (ok) {
                    Serial.println("Published telemetry data to CoreIOT");
                } else {
                    Serial.println("Failed to publish telemetry to CoreIOT");
                }
                lastPublishTick = now;
            }
        } else {
            // No data (shouldn't happen with portMAX_DELAY), yield
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        /* Small yield to keep scheduler responsive in case of tight loops */
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

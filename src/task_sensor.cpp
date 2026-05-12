#include "task_sensor.h"

#include <MQ2_LPG.h>
#include <DHT20.h>
#include <Wire.h>

#define BASELINE_SAMPLES 100
#define SAMPLE_INTERVAL_MS 100
#define MQ2_WARMUP_MS 10000
#define READ_INTERVAL_MS 1000

#define I2C_SDA 11
#define I2C_SCL 12

#define READY_BIT (1 << 8)

struct sensor_t {
    TaskHandle_t task_handle;
    QueueHandle_t queue;
    EventGroupHandle_t event_group;
    TwoWire *wire;
    DHT20 dht20;
    MQ2Sensor mq2;
    sensor_t(): dht20(), mq2(MQ2PIN) {}
    float gas_baseline;
};

static float mq2_get_baseline(sensor_t *sensor)
{
    float sum = 0;

    Serial.println("MQ2 warmup...");
    vTaskDelay(pdMS_TO_TICKS(MQ2_WARMUP_MS));

    Serial.println("Calibrating baseline...");

    for (int i = 0; i < BASELINE_SAMPLES; i++)
    {
        float gas = sensor->mq2.readGas();
        sum += gas;
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
    }

    return sum / BASELINE_SAMPLES;
}

bool sensor_get_data(sensor_handle_t handle, sensor_data_t *data, EventBits_t myBits, TickType_t timeout)
{
    if (xQueuePeek(handle->queue, data, timeout) == pdTRUE) {
        xEventGroupSetBits(handle->event_group, myBits);
        return true;
    } else {
        Serial.println("No sensor data available");
        return false;
    }
}

void sensor_task(void *pvParameters){
    sensor_t *sensor = (sensor_t *) pvParameters;

    sensor_data_t accumulated_data = {};
    uint64_t sample_count = 0;

    EventBits_t uxBitsToWait = UI_BIT | TINYML_BIT | COREIOT_BIT | WEBSERVER_BIT;

    (*sensor).gas_baseline = mq2_get_baseline(sensor);
    while (1) {
        sensor_data_t data;
        sensor->dht20.read();

        data.temperature = sensor->dht20.getTemperature();
        data.humidity = sensor->dht20.getHumidity();
        data.gas = sensor->mq2.readGas();
        data.ratio = data.gas / sensor->gas_baseline;

        if (isnan(data.temperature) || isnan(data.humidity) || isnan(data.gas)) {
            Serial.println("Failed to read from sensors!");
            continue;
        }

        accumulated_data.temperature += data.temperature;
        accumulated_data.humidity    += data.humidity;
        accumulated_data.gas         += data.gas;
        accumulated_data.ratio       += data.ratio;
        sample_count++;

        sensor_data_t avg_data;
        avg_data.temperature = accumulated_data.temperature / sample_count;
        avg_data.humidity = accumulated_data.humidity / sample_count;
        avg_data.gas = accumulated_data.gas / sample_count;
        avg_data.ratio = accumulated_data.ratio / sample_count;

        EventBits_t uxMyBits = xEventGroupWaitBits(sensor->event_group, uxBitsToWait, pdTRUE, pdTRUE, pdMS_TO_TICKS(0));

        if ((uxMyBits & uxBitsToWait) == uxBitsToWait) {
            xQueueReceive(sensor->queue, NULL, pdMS_TO_TICKS(0));
        }

        BaseType_t result = xQueueSend(sensor->queue, &avg_data, 0);
        if (result == pdPASS)
        {
            memset(&accumulated_data, 0, sizeof(accumulated_data));
            sample_count = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL_MS));
    }
}

void sensor_init(sensor_handle_t *handle)
{
    static sensor_t sensor;
    sensor.wire = &Wire;
    (*sensor.wire).begin(I2C_SDA, I2C_SCL);
    sensor.dht20.begin();
    sensor.mq2.begin();
    sensor.mq2.setCalibration(
        10,     // RL
        1,      // Ro (temporary)
        3.3,    // ESP32 voltage
        4095.0, // ESP32 ADC resolution
        0,0,0,
        0,0,0);

    sensor.queue = xQueueCreate(10, sizeof(sensor_data_t));
    if (sensor.queue == NULL) {
        Serial.println("Failed to create sensor queue");
        return;
    }
    sensor.event_group = xEventGroupCreate();
    if (sensor.event_group == NULL) {
        Serial.println("Failed to create sensor event group");
        return;
    }
    if ( pdPASS != xTaskCreate(sensor_task, "sensor_task", 4096, (void*)&sensor, 1, &sensor.task_handle) ) {
        Serial.println("Failed to create sensor task");
        return;
    }
}



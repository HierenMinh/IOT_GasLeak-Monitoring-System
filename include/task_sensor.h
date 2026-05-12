#pragma once

#define MQ2PIN 35 // mq2 pin declaration

typedef struct {
    float temperature = 0.0f;
    float humidity = 0.0f;
    float gas = 0.0f;
    float gas_baseline = 0.0f;
    float ratio = 0.0f;
} sensor_data_t;

typedef struct sensor_t *sensor_handle_t;

#define UI_BIT (1 << 0)
#define TINYML_BIT (1 << 1)
#define COREIOT_BIT (1 << 2)
#define WEBSERVER_BIT (1 << 3)

void sensor_init(sensor_handle_t *handle);
/* Every task calls this API must wait for the sensor to be ready by calling sensor_wait_for_warmup, this API will block until the sensor is ready */
bool sensor_get_data(sensor_handle_t handle, sensor_data_t *data, EventBits_t myBits, TickType_t timeout);

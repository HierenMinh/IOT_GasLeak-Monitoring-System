#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

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

/* This API initializes the sensor components and creates the sensor task 
   Uses: sensor_handle_t sensor;
         sensor_init(&sensor); */
void sensor_init(sensor_handle_t *handle);

bool sensor_i2c_mutex_take(sensor_handle_t sensor, TickType_t timeout);
void sensor_i2c_mutex_give(sensor_handle_t sensor);

/* This API retrieves data from the sensor 
   Uses: sensor_data_t data;
         sensor_get_data(sensor, &data, UI_BIT, portMAX_DELAY); 
@params handle: The sensor handle
        data: Pointer to the sensor data structure that receives the data
        myBits: The event bits to wait for, each bit corresponds to a task that needs the data. 
                If no new data is available, means that sensor have not been updated since the last time the task retrieved data, 
                or task reads faster than the sensor update rate, this API returns false.
        timeout: The timeout value, in ticks, to wait for data to become available. Use portMAX_DELAY to wait indefinitely.
@returns: true if data is available, false otherwise */
bool sensor_get_data(sensor_handle_t handle, sensor_data_t *data, EventBits_t myBits, TickType_t timeout);

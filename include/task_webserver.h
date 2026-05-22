#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "task_sensor.h"
#include <Arduino.h>

/* Initialize webserver task that will broadcast sensor data via WebSocket */
void webserver_init(sensor_handle_t sensor);

/* Webserver task (internal) */
void webserver_task(void *pvParameters);
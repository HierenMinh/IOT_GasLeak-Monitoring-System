#ifndef __TASK_COREIOT_H__
#define __TASK_COREIOT_H__

#include "task_sensor.h"
#include "check_info.h"
#include "wifi_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

void task_coreiot(void *pvParameters);
void coreiot_init(sensor_handle_t sensor);

#endif /* __TASK_COREIOT_H__ */
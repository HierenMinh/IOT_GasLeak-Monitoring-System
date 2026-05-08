#ifndef __VARIABLE_H__
#define __VARIABLE_H__

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <DHT20.h>
#include <ElegantOTA.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

typedef struct 
{
    float temperature;
    float humidity;
    int8_t score;
} sensor_data_t;

extern QueueHandle_t qSensorLed;
extern QueueHandle_t qSensorNeo;
extern QueueHandle_t qSensorLcd;

extern QueueHandle_t qSensorTinyML;
extern QueueHandle_t qAnomalyResult;

extern SemaphoreHandle_t mI2cBus;

#endif /* __VARIABLE_H__ */
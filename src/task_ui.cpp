#include "task_ui.h"
#include "task_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDR 0x21
#define LCD_COLS 16
#define LCD_ROWS 2

#define NEO_PIN         45
#define LED_COUNT       1 

#define LED_PIN             48

#define LED_CYCLE_NORMAL    20 //20ms -> 50Hz
#define LED_CYCLE_WARNING   10 //10ms -> 100Hz
#define LED_CYCLE_CRITICAL  5 //5ms -> 200Hz

typedef struct  uinion {
    float f;
    uint32_t u32;
} sensor_data_notify_t;

struct ui_control_block_t {
    LiquidCrystal_I2C lcd;
    Adafruit_NeoPixel strip;
    ui_control_block_t(): lcd(LCD_ADDR, LCD_COLS, LCD_ROWS), strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800) {}

    xTaskHandle pLED_thread;
    xTaskHandle pNEO_thread;
    xTaskHandle pLCD_thread;

    StreamBufferHandle_t myLCDstreamBuf;

    sensor_handle_t sensor;
};

void vSendCallbackFunction( StreamBufferHandle_t xStreamBuffer,  
                            BaseType_t xIsInsideISR,  
                            BaseType_t * const pxHigherPriorityTaskWoken ){}

void vReceiveCallbackFunction( StreamBufferHandle_t xStreamBuffer,  
                               BaseType_t xIsInsideISR,  
                               BaseType_t * const pxHigherPriorityTaskWoken )  {}

void thread_led_blinky(void *pvParameters){
    uint8_t led_res = LED_CYCLE_NORMAL;
    bool led_state = false;
    digitalWrite(LED_PIN, led_state);
    while (1) {
        sensor_data_notify_t data;
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &data.u32, pdMS_TO_TICKS(led_res/2));
        float ratio = data.f;
        if (ratio >= GAS_CRITICAL_THRESHOLD) {
            led_res = LED_CYCLE_CRITICAL;
        } else if (ratio >= GAS_WARNING_THRESHOLD && ratio < GAS_CRITICAL_THRESHOLD) {
            led_res = LED_CYCLE_WARNING;
        } else if (ratio > 0 && ratio < GAS_WARNING_THRESHOLD) {
            led_res = LED_CYCLE_NORMAL;
        } else {
            //Timeout, no new data, keep the current LED state and wait for the next notification
        }
        led_state = !led_state;
        digitalWrite(LED_PIN, led_state);
    }
}

void thread_neo_pixel(void *pvParameters){
    ui_control_block_t *uxCBT = (ui_control_block_t *) pvParameters;
    Adafruit_NeoPixel strip = uxCBT->strip;
    while (1){
        sensor_data_notify_t data;
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &data.u32, portMAX_DELAY);
        float temperature = data.f;
        if (data.f >= TEMP_CRITICAL_THRESHOLD) {
            strip.setPixelColor(0, strip.Color(255, 0, 0)); // Red
        } else if (data.f >= TEMP_HOT_THRESHOLD && data.f < TEMP_CRITICAL_THRESHOLD) {
            strip.setPixelColor(0, strip.Color(255, 165, 0)); // Orange
        } else if (data.f >= TEMP_WARM_THRESHOLD && data.f < TEMP_HOT_THRESHOLD) {
            strip.setPixelColor(0, strip.Color(255, 255, 0)); // Yellow
        } else if (data.f >= TEMP_PLEASANT_THRESHOLD && data.f < TEMP_WARM_THRESHOLD) {
            strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green
        } else if (data.f >= TEMP_COLD_THRESHOLD && data.f < TEMP_PLEASANT_THRESHOLD) {
            strip.setPixelColor(0, strip.Color(0, 255, 255)); // Cyan
        } else {
            strip.setPixelColor(0, strip.Color(0, 0, 255)); // Blue
        }
        strip.show();
    }
}

void thread_lcd_display(void *pvParameters){
    ui_control_block_t *uxCBT = (ui_control_block_t *) pvParameters;
    uxCBT->myLCDstreamBuf = xStreamBufferCreate(2 * 3 * sizeof(float), sizeof(float));
    //int8_t lastScore = -1;
    uint8_t data_recv_count = 0;
    while (1){
        // Check if any reads failed and exit early to try again.
        // sensor_data_t inference_result;
        // if (xQueueReceive(qAnomalyResult, &inference_result, 0) == pdTRUE){
        //     data.score = inference_result.score;
        // } else {
        //     data.score = -1; // No inference result available
        // } 
        /*TINYML NEED TO BE REVIEWED*/
        switch(data_recv_count) {
            case 0:
                float temperature;
                uxCBT->lcd.setCursor(0, 0);
                char line1[8];
                xStreamBufferReceive(uxCBT->myLCDstreamBuf, &temperature, sizeof(float), portMAX_DELAY);
                sprintf(line1, "T:%.2f", temperature);
                if (sensor_i2c_mutex_take(uxCBT->sensor, portMAX_DELAY)) {
                    uxCBT->lcd.print(line1);
                    sensor_i2c_mutex_give(uxCBT->sensor);
                }
                data_recv_count++;
            break;
            case 1:
                float humidity;
                uxCBT->lcd.setCursor(9, 0);
                char line1[8];
                xStreamBufferReceive(uxCBT->myLCDstreamBuf, &humidity, sizeof(float), portMAX_DELAY);
                sprintf(line1, "H:%.2f%%", humidity);
                if (sensor_i2c_mutex_take(uxCBT->sensor, portMAX_DELAY)) {
                    uxCBT->lcd.print(line1);
                    sensor_i2c_mutex_give(uxCBT->sensor);
                }
                data_recv_count++;
            break;
            case 2:
                float gas;
                uxCBT->lcd.setCursor(0, 1);
                char line2[12];
                xStreamBufferReceive(uxCBT->myLCDstreamBuf, &gas, sizeof(float), portMAX_DELAY);
                sprintf(line2, "Gas:%.2f", gas);
                if (sensor_i2c_mutex_take(uxCBT->sensor, portMAX_DELAY)) {
                    uxCBT->lcd.print(line2);
                    sensor_i2c_mutex_give(uxCBT->sensor);
                }
                data_recv_count = 0;
            break;
            default:
                data_recv_count = 0;
            break;
        }
    }
}

void ui_controller(void *pvParameters){
    ui_control_block_t *uxCBT = (ui_control_block_t *) pvParameters;
    xTaskCreate(thread_led_blinky, "LED Blinky", 1024, NULL, 2, &uxCBT->pLED_thread);
    xTaskCreate(thread_neo_pixel, "NeoPixel Control", 1024, NULL, 2, &uxCBT->pNEO_thread);
    xTaskCreate(thread_lcd_display, "LCD Display", 1024 * 2, (void*)uxCBT, 2, &uxCBT->pLCD_thread);
    
    while (1) {
        sensor_data_t data;
        /*This API will block for available sensor data*/
        if (sensor_get_data(uxCBT->sensor, &data, UI_BIT, portMAX_DELAY)) {
            /*LED will indicates gas concentration*/
            xTaskNotify(uxCBT->pLED_thread, data.ratio, eSetValueWithOverwrite);
            /*Neopixel used to indicate temperature levels*/
            xTaskNotify(uxCBT->pNEO_thread, data.temperature, eSetValueWithOverwrite);
            /*LCD used to display all sensor data and inference result*/
            xStreamBufferSend(uxCBT->myLCDstreamBuf, &data, sizeof(data), pdMS_TO_TICKS(0));
        } else {
            // No new data available, can perform other tasks or simply wait for the next notification
        }
    }
}

void ui_init(sensor_handle_t sensor)
{
    static ui_control_block_t myCBT;
    myCBT.sensor = sensor;

    myCBT.lcd.begin();
    myCBT.lcd.backlight();

    myCBT.strip.begin();
    myCBT.strip.clear();
    myCBT.strip.show();

    pinMode(LED_PIN, OUTPUT);
    xTaskCreate(ui_controller, "Monitor", 1024, (void*)&myCBT, 1, NULL);
}
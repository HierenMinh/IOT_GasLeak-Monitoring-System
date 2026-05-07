#include "task_neo_pixel.h"

void task_neo_pixel(void *pvParameters){
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    strip.show();

    while (1){
        sensor_data_t data;
        if (xQueueReceive(qSensorNeo, &data, portMAX_DELAY) == pdTRUE){
            if (data.humidity >= 45.0 && data.humidity <= 65.0){
                strip.setPixelColor(0, strip.Color(0, 255, 0));         // Green for normal humidity
                strip.show();
                vTaskDelay(DELAY_NORMAL / portTICK_PERIOD_MS);
                strip.setPixelColor(0, strip.Color(0, 0, 0));
                strip.show();
                vTaskDelay(DELAY_NORMAL / portTICK_PERIOD_MS);
            } else if ((data.humidity > 65.0 && data.humidity <= 75.0) || (data.humidity >= 35.0 && data.humidity < 45.0)){
                strip.setPixelColor(0, strip.Color(255, 255, 0));       // Yellow for warning humidity
                strip.show();
                vTaskDelay(DELAY_WARNING / portTICK_PERIOD_MS);
                strip.setPixelColor(0, strip.Color(0, 0, 0));
                strip.show();
                vTaskDelay(DELAY_WARNING / portTICK_PERIOD_MS);
            } else {
                strip.setPixelColor(0, strip.Color(255, 0, 0));         // Red for critical humidity
                strip.show();
                vTaskDelay(DELAY_CRITICAL / portTICK_PERIOD_MS);
                strip.setPixelColor(0, strip.Color(0, 0, 0));
                strip.show();
                vTaskDelay(DELAY_CRITICAL / portTICK_PERIOD_MS);
            }
        }        
    }
}
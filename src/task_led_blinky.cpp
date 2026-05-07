#include "task_led_blinky.h"

void task_led_blinky(void *pvParameters){
    pinMode(LED_PIN, OUTPUT);

    while (1){
        sensor_data_t data;
        if (xQueueReceive(qSensorLed, &data, portMAX_DELAY) == pdTRUE){
            if (data.temperature >= 28.0 && data.temperature <= 32.0){
                digitalWrite(LED_PIN, HIGH);
                vTaskDelay(LED_BLINK_NORMAL / portTICK_PERIOD_MS);
                digitalWrite(LED_PIN, LOW);
                vTaskDelay(LED_BLINK_NORMAL / portTICK_PERIOD_MS);
            } else if ((data.temperature > 32.0 && data.temperature <= 35.0) || (data.temperature >= 22.0 && data.temperature < 28.0)){
                digitalWrite(LED_PIN, HIGH);
                vTaskDelay(LED_BLINK_WARNING / portTICK_PERIOD_MS);
                digitalWrite(LED_PIN, LOW);
                vTaskDelay(LED_BLINK_WARNING / portTICK_PERIOD_MS);
            } else {
                digitalWrite(LED_PIN, HIGH);
                vTaskDelay(LED_BLINK_CRITICAL / portTICK_PERIOD_MS);
                digitalWrite(LED_PIN, LOW);
                vTaskDelay(LED_BLINK_CRITICAL / portTICK_PERIOD_MS);
            }
        }
    }
}


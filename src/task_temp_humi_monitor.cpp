#include "task_temp_humi_monitor.h"

void task_temp_humi_monitor(void *pvParameters){
    DHT20 dht20;
    dht20.begin();

    while (1){
        sensor_data_t data;
        float temperature = -1.0f;
        float humidity = -1.0f;

        if (xSemaphoreTake(mI2cBus, pdMS_TO_TICKS(50)) == pdTRUE) {
            dht20.read();
            temperature = dht20.getTemperature();
            humidity = dht20.getHumidity();
            xSemaphoreGive(mI2cBus);
        }

        // Check if any reads failed and exit early to try again.
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity = -1;
        }

        data.temperature = temperature;
        data.humidity = humidity;

        sensor_data_t inference_result;
        if (xQueueReceive(qAnomalyResult, &inference_result, 0) == pdTRUE){
            data.score = inference_result.score;
        } else {
            data.score = -1; // No inference result available
        }

        // Gửi dữ liệu qua queue
        xQueueOverwrite(qSensorLed, &data);
        xQueueOverwrite(qSensorNeo, &data);
        xQueueOverwrite(qSensorLcd, &data);
        xQueueOverwrite(qSensorTinyML, &data);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
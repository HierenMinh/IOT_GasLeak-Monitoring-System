#include "task_temp_humi_monitor.h"

void task_temp_humi_monitor(void *pvParameters){
    DHT20 dht20;
    LiquidCrystal_I2C lcd(0x21, 16, 2);
    Wire.begin(11, 12);
    dht20.begin();
    lcd.begin();
    lcd.backlight();

    int8_t lastScore = -1;

    while (1){
        sensor_data_t data;
        dht20.read();
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();

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

        // LCD display 
        // Hàng 1: Hiển thị nhiệt độ + độ ẩm
        lcd.setCursor(0, 0);
        char line1[17];
        sprintf(line1, "T:%.2f H:%.2f%", temperature, humidity);
        lcd.print(line1);

        // Hàng 2: Hiển thị đánh giá của mô hình (normal, warning, critical)
        lcd.setCursor(0, 1);
        char line2[17];
        if (data.score == 0) {
            sprintf(line2, "Status: Normal");
        } else if (data.score == 1) {
            sprintf(line2, "Status: Warning");
        } else if (data.score == 2) {
            sprintf(line2, "Status: Critical");
        } else {
            sprintf(line2, "Status: N/A");
        }
        lcd.print(line2);

        // Gửi dữ liệu qua queue
        xQueueOverwrite(qSensorLed, &data);
        xQueueOverwrite(qSensorNeo, &data);
        xQueueOverwrite(qSensorTinyML, &data);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
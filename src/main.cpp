// Include variable
#include "variable.h"

// Include task
#include "task_led_blinky.h"
#include "task_neo_pixel.h"
#include "task_temp_humi_monitor.h"
#include "task_webserver.h"
#include "task_tinyml.h"
#include "task_coreiot.h"

// Include task additional
#include "check_info.h"
#include "wifi.h"


void setup()
{
    Serial.begin(115200);

    // Create queue
    qSensorLed = xQueueCreate(1, sizeof(sensor_data_t));
    qSensorNeo = xQueueCreate(1, sizeof(sensor_data_t));
    qSensorTinyML = xQueueCreate(1, sizeof(sensor_data_t));
    qAnomalyResult = xQueueCreate(1, sizeof(sensor_data_t));

    if (qSensorLed == NULL || qSensorNeo == NULL || qSensorTinyML == NULL || qAnomalyResult == NULL) {
        Serial.println("Failed to create queues");
        while (1){
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Create task
    xTaskCreatePinnedToCore(task_led_blinky, "task_led_blinky", 1024 * 2, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(task_neo_pixel, "task_neo_pixel", 1024 * 2, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(task_temp_humi_monitor, "task_temp_humi_monitor", 1024 * 4, NULL, 1, NULL, 0);
    // xTaskCreatePinnedToCore(task_coreiot, "task_coreiot", 1024 * 8, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(task_tinyml, "task_tinyml", 1024 * 16, NULL, 1, NULL, 1);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
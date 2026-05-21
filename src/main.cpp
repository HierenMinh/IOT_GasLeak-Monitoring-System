// Include task
#include "task_webserver.h"
#include "task_coreiot.h"
#include "Arduino.h"
#include "WiFi.h"
// Include task additional
#include "task_ui.h"
#include "task_sensor.h"
#include "check_info.h"
#include "wifi.h"


void setup()
{
    Serial.begin(115200);
    //Initialize sensor and UI tasks
    sensor_handle_t sensor;
    sensor_init(&sensor);
    ui_init(sensor);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
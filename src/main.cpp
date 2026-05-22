// Include task
#include "task_webserver.h"
#include "task_coreiot.h"
#include "Arduino.h"
// Include task additional
#include "task_ui.h"
#include "task_sensor.h"
#include "check_info.h"
#include "wifi.h"
#include "task_tinyml.h"


void setup()
{
    Serial.begin(115200);
    Serial.println("Booting IOT GasLeak monitor...");
    check_info_File(false);
    //Initialize sensor and UI tasks
    sensor_handle_t sensor;
    sensor_init(&sensor);
    ui_init(sensor);
    webserver_init(sensor);
    coreiot_init(sensor);
    tinyml_init(sensor);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
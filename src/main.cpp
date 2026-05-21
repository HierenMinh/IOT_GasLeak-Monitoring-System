// Include task
#include "task_webserver.h"
#include "task_coreiot.h"
#include "Arduino.h"
<<<<<<< HEAD
#include "WiFi.h"
=======
#include "task_tinyml.h"
>>>>>>> 50dba553822c3a2b15713e4754954e235edbddfc
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
    // Start TinyML task and WebServer task
    xTaskCreate(task_tinyml, "TinyML", 8192, (void*)sensor, 2, NULL);
    webserver_init(sensor);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
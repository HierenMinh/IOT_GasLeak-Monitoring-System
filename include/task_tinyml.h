#ifndef __TASK_TINYML_H__
#define __TASK_TINYML_H__

#include "task_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "check_info.h"
#include "wifi_config.h"
#include <Arduino.h>
#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

void task_tinyml(void *pvParameters);

/* Initialize TinyML task. Pass the sensor handle so the task can read sensor data. */
void tinyml_init(sensor_handle_t sensor);

#endif /* __TASK_TINYML_H__ */
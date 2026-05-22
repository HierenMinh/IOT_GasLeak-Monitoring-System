#pragma once

#include "task_sensor.h"

#define GAS_WARNING_THRESHOLD 1.5f
#define GAS_CRITICAL_THRESHOLD 2.0f

#define TEMP_COLD_THRESHOLD 16.0f
#define TEMP_PLEASANT_THRESHOLD 28.0f
#define TEMP_WARM_THRESHOLD 35.0f
#define TEMP_HOT_THRESHOLD 40.0f
#define TEMP_CRITICAL_THRESHOLD 50.0f

typedef struct ui_data_t *ui_handle_t;

/*this API initializes all UI components and thread that manage them*/
void ui_init(sensor_handle_t sensor);
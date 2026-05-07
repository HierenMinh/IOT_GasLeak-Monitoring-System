#ifndef __TASK_TINYML_H__
#define __TASK_TINYML_H__

#include "variable.h"

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

void task_tinyml(void *pvParameters);

#endif /* __TASK_TINYML_H__ */

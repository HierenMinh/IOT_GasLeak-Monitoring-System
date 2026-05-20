#include "task_tinyml.h"
#include "task_sensor.h"
#include "dht_anomaly_model.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <Arduino.h>

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"


void task_tinyml(void *pvParameters){
    // pvParameters is expected to be a sensor_handle_t
    sensor_handle_t sensor = (sensor_handle_t) pvParameters;
    // Setup TensorFlow Lite
    constexpr int kTensorArenaSize = 32 * 1024; // Adjust based on model requirements

    tflite::MicroErrorReporter micro_error_reporter;
    tflite::ErrorReporter* error_reporter = &micro_error_reporter;

    const tflite::Model* model = tflite::GetModel(dht_anomaly_model_tflite);

    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report("Model schema mismatch");
        vTaskDelete(NULL);
        return;
    }

    uint8_t *tensor_arena = new uint8_t[kTensorArenaSize];
    if (tensor_arena == nullptr) {
        error_reporter->Report("Failed to allocate tensor arena");
        vTaskDelete(NULL);
        return;
    }

    tflite::AllOpsResolver *resolver = new tflite::AllOpsResolver();
    if (resolver == nullptr) {
        error_reporter->Report("Failed to create op resolver");
        delete[] tensor_arena;
        vTaskDelete(NULL);
        return;
    }

    tflite::MicroInterpreter *interpreter = new tflite::MicroInterpreter(
        model, *resolver, tensor_arena, kTensorArenaSize, error_reporter);
    if (interpreter == nullptr) {
        error_reporter->Report("Failed to create interpreter");
        delete resolver;
        delete[] tensor_arena;
        vTaskDelete(NULL);
        return;
    }

    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        error_reporter->Report("Failed to allocate tensors");
        delete interpreter;
        delete resolver;
        delete[] tensor_arena;
        vTaskDelete(NULL);
        return;
    }

    TfLiteTensor* input = interpreter->input(0);
    TfLiteTensor* output = interpreter->output(0);

    while (1){
        sensor_data_t data;
        // Use sensor_get_data to obtain averaged sensor data for TinyML
        if (!sensor_get_data(sensor, &data, TINYML_BIT, portMAX_DELAY)){
            continue;
        }

        // Prepare input data
        input->data.f[0] = data.temperature;
        input->data.f[1] = data.humidity;

        // Debug: Print input values
        Serial.printf("TinyML Input - Temp: %.2f, Humi: %.2f\n", data.temperature, data.humidity);

        // Run inference
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
            error_reporter->Report("Invoke failed");
            continue;
        }
        
        // Get output
        float normal_score = output->data.f[0];
        float warning_score = output->data.f[1];
        float critical_score = output->data.f[2];

        // Check for NaN/Inf values in output
        if (isnan(normal_score) || isnan(warning_score) || isnan(critical_score) ||
            isinf(normal_score) || isinf(warning_score) || isinf(critical_score)) {
            error_reporter->Report("Output contains NaN or Inf");
            continue;
        }

        // Determine predicted class based on output scores
        int predicted_class = 0; // 0: Normal, 1: Warning, 2: Critical
        if (warning_score > normal_score && warning_score > critical_score) {
            predicted_class = 1;
        } else if (critical_score > normal_score && critical_score > warning_score) {
            predicted_class = 2;
        }

        // Currently send inference result back by notifying via sensor event bits
        // (Could be extended to a dedicated queue if needed)
        sensor_data_t result;
        result.temperature = data.temperature;
        result.humidity = data.humidity;
        // There is no shared queue declared for anomaly results in current codebase.
        // For now just print the predicted class; integration point available here.
        Serial.printf("Anomaly result - class:%d T:%.2f H:%.2f\n", predicted_class, result.temperature, result.humidity);

        // Debug: Print output scores
        Serial.printf("TinyML Output - Normal: %.4f, Warning: %.4f, Critical: %.4f, Predicted Class: %d\n",
                      normal_score, warning_score, critical_score, predicted_class);
    }
}
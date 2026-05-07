#include "task_tinyml.h"
#include "dht_anomaly_model.h"

void task_tinyml(void *pvParameters){
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

        // If data is not received, skip this iteration and wait for the next one
        if (xQueueReceive(qSensorTinyML, &data, portMAX_DELAY) != pdTRUE){
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

        // Send inference result to the anomaly result queue
        sensor_data_t result;
        result.score = predicted_class;
        result.temperature = data.temperature;
        result.humidity = data.humidity;
        xQueueOverwrite(qAnomalyResult, &result);

        // Debug: Print output scores
        Serial.printf("TinyML Output - Normal: %.4f, Warning: %.4f, Critical: %.4f, Predicted Class: %d\n",
                      normal_score, warning_score, critical_score, predicted_class);
    }
}
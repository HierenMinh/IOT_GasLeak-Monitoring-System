#include "task_tinyml.h"
#include "gas_leak_model.h"

void tinyml_init(sensor_handle_t sensor) {
    // Create the TinyML task and pass the sensor handle as parameter
    xTaskCreate(task_tinyml, "TinyML Task", 8192, (void*)sensor, 1, NULL);
}

void task_tinyml(void *pvParameters){
    sensor_handle_t sensor = (sensor_handle_t)pvParameters;
    // Setup TensorFlow Lite
    constexpr int kTensorArenaSize = 32 * 1024; // Adjust based on model requirements

    tflite::MicroErrorReporter micro_error_reporter;
    tflite::ErrorReporter* error_reporter = &micro_error_reporter;

    const tflite::Model* model = tflite::GetModel(gas_leak_model_tflite);

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
        // Read sensor data from the shared sensor interface
        if (!sensor_get_data(sensor, &data, TINYML_BIT, portMAX_DELAY)) {
            // No data available, wait and retry
            continue;
        }

        // Prepare input data (temperature, humidity, gas)
        input->data.f[0] = data.temperature;
        input->data.f[1] = data.humidity;
        input->data.f[2] = data.gas;

        // Log inputs so we can verify model receives gas
        // Serial.printf("[TINYML] Inputs - Temp: %.2f, Humi: %.2f, Gas: %.4f\n",
        //           data.temperature, data.humidity, data.gas);

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

        // Attach predicted class to data and log it
        data.score = predicted_class;
        // Write predicted score back into shared sensor record so other
        // consumers (e.g., CoreIoT) can publish it.
        if (sensor != NULL) {
            (void)sensor_update_score(sensor, predicted_class);
        }

        // Serial.printf("[TINYML] Output - Normal: %.4f, Warning: %.4f, Critical: %.4f, Predicted Class: %d\n",
        //           normal_score, warning_score, critical_score, predicted_class);
    }
}
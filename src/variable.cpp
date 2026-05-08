#include "variable.h"

QueueHandle_t qSensorLed = NULL;
QueueHandle_t qSensorNeo = NULL;
QueueHandle_t qSensorLcd = NULL;

QueueHandle_t qSensorTinyML = NULL;
QueueHandle_t qAnomalyResult = NULL;

SemaphoreHandle_t mI2cBus = NULL;

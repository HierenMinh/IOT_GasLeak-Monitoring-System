#include "task_lcd_display.h"

static void printFixedLine(LiquidCrystal_I2C &lcd, uint8_t row, const char *text)
{
    char line[17];
    snprintf(line, sizeof(line), "%-16s", text);
    lcd.setCursor(0, row);
    lcd.print(line);
}

static void printSensorData(LiquidCrystal_I2C &lcd, const sensor_data_t &data)
{
    char line[17];

    snprintf(line, sizeof(line), "T:%5.1f H:%4.1f", data.temperature, data.humidity);
    printFixedLine(lcd, 0, line);

    if (data.score == 0) {
        printFixedLine(lcd, 1, "Status: Normal");
    } else if (data.score == 1) {
        printFixedLine(lcd, 1, "Status: Warning");
    } else if (data.score == 2) {
        printFixedLine(lcd, 1, "Status: Critical");
    } else {
        printFixedLine(lcd, 1, "Status: N/A");
    }
}

void task_lcd_display(void *pvParameters)
{
    (void)pvParameters;

    LiquidCrystal_I2C lcd(0x21, 16, 2);

    if (xSemaphoreTake(mI2cBus, portMAX_DELAY) == pdTRUE) {
        lcd.begin();
        lcd.backlight();
        printFixedLine(lcd, 0, "Aquarium System");
        printFixedLine(lcd, 1, "Waiting data...");
        xSemaphoreGive(mI2cBus);
    }
    int8_t latestScore = -1;

    while (true) {
        sensor_data_t data;

        if (xQueueReceive(qSensorLcd, &data, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        sensor_data_t inference;
        while (xQueueReceive(qAnomalyResult, &inference, 0) == pdTRUE) {
            latestScore = inference.score;
        }
        data.score = latestScore;

        if (xSemaphoreTake(mI2cBus, portMAX_DELAY) == pdTRUE) {
            printSensorData(lcd, data);
            xSemaphoreGive(mI2cBus);
        }
    }
}

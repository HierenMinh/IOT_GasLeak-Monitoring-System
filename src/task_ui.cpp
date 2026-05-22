#include "task_ui.h"
#include "task_sensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include "Wire.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>

/* ─────────────────────────────────────────────────────────────
 * LCD I2C address: scan with I2C scanner to confirm.
 * Most PCF8574-based backpack modules use 0x27 or 0x3F.
 * ───────────────────────────────────────────────────────────── */
#define LCD_ADDR 0x21
#define LCD_COLS 16
#define LCD_ROWS 2

#define NEO_PIN 45
#define LED_COUNT 1

#define LED_PIN 48

#define RELAY_PIN 46
#define BUZZER_PIN 47

#define LED_CYCLE_NORMAL 5000   // 5000 ms visible blink
#define LED_CYCLE_WARNING 3000  // 3000 ms faster blink
#define LED_CYCLE_CRITICAL 1000 // 1000 ms critical blink

/* ─────────────────────────────────────────────────────────────
 * FIX: must be `union`, not `struct`.
 * With a struct, f and u32 occupy *separate* memory locations,
 * so writing u32 then reading f (or vice-versa) returns garbage.
 * A union makes both fields share the same 4 bytes, which is the
 * only way to safely reinterpret a float as uint32_t for
 * xTaskNotify / xTaskNotifyWait.
 * ───────────────────────────────────────────────────────────── */
typedef union
{
    float f;
    uint32_t u32;
} sensor_data_notify_t;

/* ─────────────────────────────────────────────────────────────
 * UI control block
 * ───────────────────────────────────────────────────────────── */
struct ui_control_block_t
{
    LiquidCrystal_I2C lcd;
    Adafruit_NeoPixel strip;

    ui_control_block_t()
        : lcd(LCD_ADDR, LCD_COLS, LCD_ROWS),
          strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800) {}

    xTaskHandle pLED_thread;
    xTaskHandle pNEO_thread;
    xTaskHandle pLCD_thread;
    xTaskHandle pBUZZER_thread;
    xTaskHandle pRELAY_thread;

    StreamBufferHandle_t myLCDstreamBuf;

    sensor_handle_t sensor;
};

/* ─────────────────────────────────────────────────────────────
 * Stream-buffer callbacks (empty stubs – required by FreeRTOS
 * when sbSEND_COMPLETED / sbRECEIVE_COMPLETED are overridden)
 * ───────────────────────────────────────────────────────────── */
void vSendCallbackFunction(StreamBufferHandle_t xStreamBuffer,
                           BaseType_t xIsInsideISR,
                           BaseType_t *const pxHigherPriorityTaskWoken) {}

void vReceiveCallbackFunction(StreamBufferHandle_t xStreamBuffer,
                              BaseType_t xIsInsideISR,
                              BaseType_t *const pxHigherPriorityTaskWoken) {}

/* ─────────────────────────────────────────────────────────────
 * thread_led_blinky
 * Blinks the onboard LED; blink rate now reflects temperature.
 * Notification value carries the temperature as a float bit-cast
 * to uint32_t via sensor_data_notify_t.
 * ───────────────────────────────────────────────────────────── */
void thread_led_blinky(void *pvParameters)
{
    ui_control_block_t *uxCBT = (ui_control_block_t *)pvParameters;
    uint32_t led_res = LED_CYCLE_NORMAL;
    bool led_state = false;
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    float currentTemperature = 0.0f;

    while (1)
    {
        /* check for updated temperature notifications (non-blocking-ish) */
        sensor_data_notify_t data = {};
        if (xTaskNotifyWait(pdFALSE, ULONG_MAX, &data.u32, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            currentTemperature = data.f;
            if (currentTemperature >= TEMP_CRITICAL_THRESHOLD)
            {
                led_res = LED_CYCLE_CRITICAL;
            }
            else if (currentTemperature >= TEMP_WARM_THRESHOLD)
            {
                led_res = LED_CYCLE_WARNING;
            }
            else if (currentTemperature > 0.0f)
            {
                led_res = LED_CYCLE_NORMAL;
            }
        }

        /* toggle and wait half period so full on-off cycle equals led_res */
        led_state = !led_state;
        digitalWrite(LED_PIN, led_state ? HIGH : LOW);
        vTaskDelay(pdMS_TO_TICKS(led_res / 2));
    }
}

/* ─────────────────────────────────────────────────────────────
 * thread_neo_pixel
 * Blinks NeoPixel according to humidity levels. Notification
 * value carries humidity as a float bit-cast to uint32_t.
 * Colour also loosely reflects humidity level.
 * ───────────────────────────────────────────────────────────── */
void thread_neo_pixel(void *pvParameters)
{
    ui_control_block_t *uxCBT = (ui_control_block_t *)pvParameters;
    Adafruit_NeoPixel &strip = uxCBT->strip; // reference, not copy
    float currentHumidity = 0.0f;
    bool ledOn = false;
    TickType_t lastToggleTick = 0;
    TickType_t blinkPeriod = pdMS_TO_TICKS(LED_CYCLE_NORMAL);

    while (1)
    {
        sensor_data_notify_t data = {};
        if (xTaskNotifyWait(pdFALSE, ULONG_MAX, &data.u32, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            currentHumidity = data.f; // correct float via union

            /* determine blinkPeriod from humidity thresholds */
            if (currentHumidity >= HUM_CRITICAL_THRESHOLD)
            {
                blinkPeriod = pdMS_TO_TICKS(LED_CYCLE_CRITICAL);
            }
            else if (currentHumidity >= HUM_WARNING_THRESHOLD)
            {
                blinkPeriod = pdMS_TO_TICKS(LED_CYCLE_WARNING);
            }
            else if (currentHumidity >= 0.0f)
            {
                blinkPeriod = pdMS_TO_TICKS(LED_CYCLE_NORMAL);
            }
        }

        TickType_t now = xTaskGetTickCount();
        if ((now - lastToggleTick) >= blinkPeriod)
        {
            ledOn = !ledOn;
            lastToggleTick = now;
        }

        uint32_t color = 0;
        if (currentHumidity >= HUM_CRITICAL_THRESHOLD)
        {
            color = strip.Color(255, 0, 0); // Red
        }
        else if (currentHumidity >= HUM_WARNING_THRESHOLD)
        {
            color = strip.Color(255, 165, 0); // Orange
        }
        else
        {
            color = strip.Color(0, 255, 0); // Green (normal)
        }

        if (ledOn)
        {
            strip.setPixelColor(0, color);
        }
        else
        {
            strip.setPixelColor(0, 0);
        }
        strip.show();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* ─────────────────────────────────────────────────────────────
 * thread_lcd_display
 * Receives full sensor_data_t records from the stream buffer
 * and renders them on the 16×2 LCD.
 * Line 0: "T:23.4C  H:45.6%"
 * Line 1: "Gas:123.45"
 * ───────────────────────────────────────────────────────────── */
void thread_lcd_display(void *pvParameters)
{
    ui_control_block_t *uxCBT = (ui_control_block_t *)pvParameters;
    sensor_data_t recvData;
    sensor_data_t lastShownData = {};
    bool hasLastShownData = false;
    TickType_t lastLcdUpdateTick = 0;
    const TickType_t lcdUpdateInterval = pdMS_TO_TICKS(1000);

    while (1)
    {
        if (xStreamBufferReceive(uxCBT->myLCDstreamBuf,
                                 &recvData,
                                 sizeof(sensor_data_t),
                                 portMAX_DELAY) == sizeof(sensor_data_t))
        {
            TickType_t now = xTaskGetTickCount();
            bool shouldUpdate = !hasLastShownData || (fabs(recvData.temperature - lastShownData.temperature) >= 0.2f) || (fabs(recvData.humidity - lastShownData.humidity) >= 0.5f) || (fabs(recvData.gas - lastShownData.gas) >= 5.0f) || (fabs(recvData.ratio - lastShownData.ratio) >= 0.02f) || ((now - lastLcdUpdateTick) >= lcdUpdateInterval);

            if (!shouldUpdate)
            {
                continue;
            }

            char line1[17];
            char line2[17];
            snprintf(line1, sizeof(line1), "T:%.1fC H:%.1f%%",
                     recvData.temperature, recvData.humidity);
            snprintf(line2, sizeof(line2), "Gas:%.2f",
                     recvData.gas);

            char paddedLine1[17];
            char paddedLine2[17];
            snprintf(paddedLine1, sizeof(paddedLine1), "%-16s", line1);
            snprintf(paddedLine2, sizeof(paddedLine2), "%-16s", line2);

            if (sensor_i2c_mutex_take(uxCBT->sensor, portMAX_DELAY))
            {
                uxCBT->lcd.setCursor(0, 0);
                uxCBT->lcd.print(paddedLine1);
                uxCBT->lcd.setCursor(0, 1);
                uxCBT->lcd.print(paddedLine2);
                sensor_i2c_mutex_give(uxCBT->sensor);

                lastShownData = recvData;
                hasLastShownData = true;
                lastLcdUpdateTick = now;
            }
        }
    }
}

/* ─────────────────────────────────────────────────────────────
 * thread_buzzer
 * Drives a passive buzzer at different frequencies depending on
 * the gas concentration ratio.
 * ───────────────────────────────────────────────────────────── */
void thread_buzzer(void *pvParameters)
{
    while (1)
    {
        sensor_data_notify_t data;
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &data.u32, portMAX_DELAY);

        float ratio = data.f;

        if (ratio >= GAS_CRITICAL_THRESHOLD)
        {
            tone(BUZZER_PIN, 2000); // 2 kHz – critical
        }
        else if (ratio >= GAS_WARNING_THRESHOLD)
        {
            tone(BUZZER_PIN, 1000); // 1 kHz – warning
        }
        else if (ratio > 0.0f)
        {
            tone(BUZZER_PIN, 500); // 500 Hz – normal detection
        }
        else
        {
            noTone(BUZZER_PIN); // no gas – silence
        }
    }
}

/* ─────────────────────────────────────────────────────────────
 * thread_relay
 * Energises the relay when gas concentration is critical.
 * ───────────────────────────────────────────────────────────── */
void thread_relay(void *pvParameters)
{
    while (1)
    {
        sensor_data_notify_t data;
        xTaskNotifyWait(pdFALSE, ULONG_MAX, &data.u32, portMAX_DELAY);

        float ratio = data.f;

        if (ratio >= GAS_CRITICAL_THRESHOLD)
        {
            digitalWrite(RELAY_PIN, HIGH);
        }
        else
        {
            digitalWrite(RELAY_PIN, LOW);
        }
    }
}

/* ─────────────────────────────────────────────────────────────
 * ui_controller
 * Spawns all UI sub-tasks, then loops reading sensor data and
 * dispatching it to each sub-task.
 *
 * FIX: xTaskNotify only carries a uint32_t value.
 * To pass a float without losing precision we bit-cast it
 * through sensor_data_notify_t (which is now a union).
 * ───────────────────────────────────────────────────────────── */
void ui_controller(void *pvParameters)
{
    ui_control_block_t *uxCBT = (ui_control_block_t *)pvParameters;

    xTaskCreate(thread_led_blinky, "LED Blinky", 1024 * 2, (void *)uxCBT, 2, &uxCBT->pLED_thread);
    xTaskCreate(thread_neo_pixel, "NeoPixel Control", 1024 * 2, (void *)uxCBT, 2, &uxCBT->pNEO_thread);
    xTaskCreate(thread_relay, "Relay Control", 1024 * 2, (void *)uxCBT, 2, &uxCBT->pRELAY_thread);
    xTaskCreate(thread_buzzer, "Buzzer Control", 1024 * 2, (void *)uxCBT, 2, &uxCBT->pBUZZER_thread);
    xTaskCreate(thread_lcd_display, "LCD Display", 1024 * 4, (void *)uxCBT, 2, &uxCBT->pLCD_thread);

    sensor_data_notify_t notif;

    while (1)
    {
        sensor_data_t data;

        if (sensor_get_data(uxCBT->sensor, &data, UI_BIT, portMAX_DELAY))
        {

            /* ── LED: temperature ───────────────────────────── */
            notif.f = data.temperature;
            xTaskNotify(uxCBT->pLED_thread, notif.u32, eSetValueWithOverwrite);

            /* ── NeoPixel: humidity ─────────────────────────── */
            notif.f = data.humidity;
            xTaskNotify(uxCBT->pNEO_thread, notif.u32, eSetValueWithOverwrite);

            /* ── Buzzer: gas ratio ───────────────────────────── */
            notif.f = data.ratio;
            xTaskNotify(uxCBT->pBUZZER_thread, notif.u32, eSetValueWithOverwrite);

            /* ── Relay: gas ratio ────────────────────────────── */
            notif.f = data.ratio;
            xTaskNotify(uxCBT->pRELAY_thread, notif.u32, eSetValueWithOverwrite);

            /* ── LCD: full sensor record via stream buffer ───── */
            if (uxCBT->myLCDstreamBuf != NULL)
            {
                xStreamBufferSend(uxCBT->myLCDstreamBuf,
                                  &data,
                                  sizeof(data),
                                  pdMS_TO_TICKS(100));
            }
        }
    }
}

/* ─────────────────────────────────────────────────────────────
 * ui_init
 * Initialises peripherals and launches the UI controller task.
 * ───────────────────────────────────────────────────────────── */
void ui_init(sensor_handle_t sensor)
{
    static ui_control_block_t myCBT;
    myCBT.sensor = sensor;

    Serial.println("Scanning I2C bus...");
    // Do not call Wire.begin() here — I2C already initialised in sensor_init()
    uint8_t lcd_addr_found = 0;
    for (uint8_t addr = 0x08; addr < 0x78; addr++)
    {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0)
        {
            Serial.printf("  I2C device found at 0x%02X\n", addr);
            lcd_addr_found = addr;
        }
    }
    if (lcd_addr_found == 0)
    {
        Serial.println("  No I2C device found! Check wiring.");
        // Dừng lại, không init LCD để tránh watchdog
        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
    Serial.printf("  Using LCD at 0x%02X\n", lcd_addr_found);
    // ────────────────────────────────────────────────────────

    /* LCD */
    myCBT.lcd.begin();
    myCBT.lcd.backlight();
    myCBT.lcd.clear();
    myCBT.lcd.setCursor(0, 0);
    myCBT.lcd.print("Initialising...");

    /* NeoPixel */
    myCBT.strip.begin();
    myCBT.strip.clear();
    myCBT.strip.show();

    /* GPIO */
    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);

    /* Create LCD stream buffer BEFORE spawning tasks */
    myCBT.myLCDstreamBuf = xStreamBufferCreate(8 * sizeof(sensor_data_t),
                                               sizeof(sensor_data_t));
    configASSERT(myCBT.myLCDstreamBuf != NULL);

    /* Launch controller (priority 1; child tasks run at priority 2) */
    xTaskCreate(ui_controller, "Monitor", 1024 * 4, (void *)&myCBT, 1, NULL);
}
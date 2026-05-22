// Include task
#include "task_webserver.h"
#include "task_coreiot.h"
#include "Arduino.h"
// Include task additional
#include "task_ui.h"
#include "task_sensor.h"
#include "check_info.h"
#include "wifi.h"
#include "task_tinyml.h"


void setup()
{
    Serial.begin(115200);
    Serial.println("Booting IOT GasLeak monitor...");
    check_info_File(false);
    //Initialize sensor and UI tasks
    sensor_handle_t sensor;
    sensor_init(&sensor);
    ui_init(sensor);
    webserver_init(sensor);
    coreiot_init(sensor);
    tinyml_init(sensor);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// #include <Arduino.h>
// #define MQ2_PIN 1 // Chân A0 (GPIO1) trên Yolo UNO
// #define RL_VALUE 10.0 // Điện trở tải trên mạch thường là 10kOhm

// void setup() {
//   Serial.begin(115200);
//   analogReadResolution(12); // Đảm bảo độ phân giải 12-bit (0 - 4095)
  
//   Serial.println("Đang sưởi cảm biến 10 giây...");
//   delay(10000); 

//   float adc_sum = 0;
//   for(int i=0; i<100; i++) {
//     adc_sum += analogRead(MQ2_PIN);
//     delay(100);
//   }
//   float adc_avg = adc_sum / 100.0;

//   // Tính Rs trong không khí sạch
//   float rs_air = RL_VALUE * (4095.0 - adc_avg) / adc_avg;
  
//   // Tính Ro
//   float ro = rs_air / 9.83;

//   Serial.printf("\n--- KẾT QUẢ CÂN CHỈNH ---\n");
//   Serial.printf("ADC Trung bình: %.2f\n", adc_avg);
//   Serial.printf("Giá trị Ro thật của mạch bạn: %.4f\n", ro);
//   Serial.printf("-------------------------\n");
// }

// void loop() {}
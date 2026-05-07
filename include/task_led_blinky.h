#ifndef __TASK_LED_BLINKY_H__
#define __TASK_LED_BLINKY_H__

#include "variable.h"

#define LED_PIN             48
#define LED_BLINK_NORMAL    2000
#define LED_BLINK_WARNING   1000
#define LED_BLINK_CRITICAL  500

void task_led_blinky(void *pvParameters);

#endif /* __TASK_LED_BLINKY_H__ */
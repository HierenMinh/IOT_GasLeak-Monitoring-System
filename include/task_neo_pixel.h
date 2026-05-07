#ifndef __TASK_NEO_PIXEL_H__
#define __TASK_NEO_PIXEL_H__

#include "variable.h"

#define NEO_PIN         45
#define LED_COUNT       1 

#define DELAY_NORMAL    2000
#define DELAY_WARNING   1000
#define DELAY_CRITICAL  500

void task_neo_pixel(void *pvParameters);

#endif /* __TASK_NEO_PIXEL_H__ */
#ifndef __WIFI_CONFIG_H__
#define __WIFI_CONFIG_H__

#include "WiFi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"      
#include "freertos/semphr.h"

#include "check_info.h"

extern SemaphoreHandle_t xBinarySemaphoreInternet;

bool Wifi_reconnect();
void startAP();
void startSTA();

#endif /* __WIFI_CONFIG_H__ */
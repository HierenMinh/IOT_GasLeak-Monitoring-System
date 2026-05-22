#ifndef __CHECK_INFO_H__
#define __CHECK_INFO_H__

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "ArduinoJson.h"
#include "AsyncTCP.h"
#include "LittleFS.h"
#include "wifi_config.h"

extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

bool check_info_File(bool check);
void Load_info_File();
void Delete_info_File();
void Save_info_File(String WIFI_SSID, String WIFI_PASS, String CORE_IOT_TOKEN, String CORE_IOT_SERVER, String CORE_IOT_PORT);

#endif
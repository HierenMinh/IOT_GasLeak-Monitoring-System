#pragma once

/* Initialize webserver task that will broadcast sensor data via WebSocket */
void webserver_init(sensor_handle_t sensor);

/* Webserver task (internal) */
void webserver_task(void *pvParameters);
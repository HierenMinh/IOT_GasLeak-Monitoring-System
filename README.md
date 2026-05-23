# IoT-Based Smart Gas Leakage Monitoring System with Edge AI

> **Course:** IoT Application Development — Ho Chi Minh City University of Technology  
> **Advisor:** Dr. Lê Trọng Nhân  
> **Team:** Nguyễn Minh Hiển (2310998) · Lê Văn Đình Huy (2311160) · Bành Huỳnh Minh Huy (2311118)

---

## Abstract

This project presents the design and implementation of an end-to-end IoT ecosystem using a **Smart Gas Leakage Monitoring** system as a practical case study. Built on the **ESP32-S3 (YoloUNO)** platform, the system demonstrates a comprehensive multi-layered IoT architecture by integrating:

- Local sensor data acquisition via **FreeRTOS** multi-tasking
- On-device anomaly detection via **TinyML** (TensorFlow Lite Micro)
- Local monitoring via a **WebSocket-based Web Server** with mDNS discovery
- Cloud telemetry and remote control via **MQTT** on the **Core IOT** platform
- Peer-to-peer fallback communication via **ESP-NOW**

The system successfully bridges edge computing and cloud orchestration to deliver a scalable, intelligent, and responsive IoT solution.

---

## Table of Contents

1. [Introduction and Objectives](#1-introduction-and-objectives)
2. [System Architecture](#2-system-architecture)
3. [System Implementation](#3-system-implementation)
   - [3.1 Sensor Subsystem](#31-sensor-subsystem)
   - [3.2 User Interfaces Subsystem](#32-user-interfaces-subsystem)
   - [3.3 Web Server](#33-web-server)
   - [3.4 ESP-NOW Fallback](#34-esp-now-as-preventive-plan)
   - [3.5 Core IOT Cloud Integration](#35-core-iot-implementation)
   - [3.6 Edge AI Deployment (TinyML)](#36-edge-ai-deployment-tinyml)
4. [Experimental Results](#4-experimental-results)
5. [Group Organization & Git Contribution](#5-group-organization--git-contribution)
6. [Conclusions](#6-conclusions)

---

## 1. Introduction and Objectives

Modern IoT architectures have evolved from simple connected sensors into complex, multi-layered systems designed for massive scalability, real-time processing, and robust data management. A complete IoT architecture comprises five layers: **Edge**, **Computation**, **Connectivity**, **Cloud**, and **Application**.

This project adopts a **Smart Gas Leak Monitoring** scenario to practically validate this framework. By combining RTOS-managed task scheduling with Edge AI and cloud orchestration, the system can detect gas anomalies locally and respond immediately — even without internet connectivity.

### Objectives

| Objective | Description |
|---|---|
| **RTOS Integration** | Deterministic multi-tasking on ESP32-S3 using FreeRTOS |
| **Edge AI (TinyML)** | On-device gas-leak classification via TFLite Micro |
| **Cloud Orchestration** | MQTT telemetry and RPC control via Core IOT (ThingsBoard) |
| **Local Web Server** | Real-time WebSocket dashboard with mDNS device discovery |
| **Fault Tolerance** | ESP-NOW peer-to-peer fallback when cloud connectivity is lost |

---

## 2. System Architecture

### RTOS Task Architecture

The firmware uses FreeRTOS to decouple all operations into independent, schedulable tasks spawned at startup via `xTaskCreate`:

```
System Entry
    └── FreeRTOS xTaskCreate API
            ├── Sensor Data Task       — DHT20 + MQ2 acquisition & calibration
            ├── User Interface Task    — LCD, NeoPixel, LED, Buzzer, Relay
            ├── Webserver Service Task — Async HTTP + WebSocket server
            ├── Core IoT Task          — MQTT publish to cloud
            └── TinyML Inference Task  — On-device gas-leak classification
```

**Inter-task communication** uses FreeRTOS primitives exclusively — no global variables:

- `xQueueOverwrite` / `xQueuePeek` — sensor data distribution (size-1 queue)
- `xSemaphoreGive` / `xSemaphoreTake` — I2C bus mutex, shared resource protection
- `xTaskNotify` — lightweight float value passing to LED/NeoPixel/Buzzer subtasks
- `xStreamBuffer` — LCD data streaming

### Overall Connectivity & Data Flow

```
Sensor Node (ESP32-S3)
    ├── [MQTT]     → Core IOT Cloud → Rule Chain → Dashboard / RPC → Controller Node
    ├── [WebSocket] → Local Browser Dashboard
    └── [ESP-NOW]  → Controller Node (fallback when cloud is unavailable)
```

---

## 3. System Implementation

### 3.1 Sensor Subsystem

Handles initialization, baseline calibration, and periodic data collection from two sensors:

| Sensor | Measurement | Notes |
|---|---|---|
| **DHT20** | Temperature (°C), Humidity (%RH) | I2C, protected by mutex |
| **MQ2** | Gas concentration (ADC / ppm) | Baseline calibration on boot |

The subsystem computes a **gas ratio** (current reading / calibrated baseline) to normalize gas concentration, reducing sensitivity to environmental drift. Rolling averages are maintained for all three features and distributed as a `sensor_data_t` structure via the RTOS queue.

The public API `sensor_get_data()` supports configurable blocking behavior per consumer:

```c
// Each task supplies its own event bit and timeout
bool sensor_get_data(sensor_handle_t handle, sensor_data_t *data,
                     EventBits_t myBits, TickType_t timeout);
```

### 3.2 User Interfaces Subsystem

A unified local feedback layer combining five output components:

| Component | Behavior |
|---|---|
| **LCD** | Displays temperature, humidity, and gas ratio numerically |
| **NeoPixel LED** | Color encodes temperature level (cold → pleasant → warm → hot → critical) |
| **Blinking LED** | Blink rate reflects gas concentration level |
| **Buzzer** | Activates on Warning or Critical gas conditions |
| **Relay** | Activates on Critical gas conditions for actuator control |

Each component runs in its own FreeRTOS subtask. The UI controller distributes data using `xTaskNotify` (float bit-casting) and `xStreamBuffer` for the LCD.

### 3.3 Web Server

The webserver subsystem serves two roles depending on firmware configuration:

- **Sensor-node role:** Exposes a local configuration page for first-time Wi-Fi and CoreIoT setup
- **Controller role:** Serves a real-time monitoring dashboard with WebSocket push updates

**Technology stack:**

| Component | Choice | Reason |
|---|---|---|
| HTTP server | Async (ESPAsyncWebServer) | Non-blocking, compatible with FreeRTOS |
| File system | LittleFS | Efficient static file serving |
| Live data | WebSocket at `/ws` | Persistent push vs. repeated HTTP polling |
| Device discovery | mDNS (`GasMonitor.local`) | Human-readable hostname, no IP lookup needed |

The dashboard displays temperature, humidity, gas (ppm), TinyML classification result, relay states, and historical time-series charts. Three alert states are rendered in real time:

| State | Gas Level | TinyML Label | Color |
|---|---|---|---|
| Normal | ~110 ppm | `Predicted: 0` | Green |
| Warning | ~819 ppm | `Predicted: 1` | Orange |
| Critical | ~921 ppm | `Predicted: 2` | Red |

### 3.4 ESP-NOW as Preventive Plan

ESP-NOW provides a **peer-to-peer fallback** channel when the primary MQTT/cloud path is unavailable. Sensor nodes broadcast `sensor_data_t` packets directly to the controller node's MAC address. The controller receives them via a registered callback and injects them into the same internal sensor queue — requiring no changes to upstream consumers (UI, webserver, CoreIoT task).

```c
void sensor_espnow_receive_cb(const uint8_t mac[WIFIESPNOW_ALEN],
                               const uint8_t *buf, size_t count, void *arg) {
    sensor_data_t data;
    memcpy(&data, buf, sizeof(data));
    xQueueSend(handle->queue, &data, 0);
}
// Register:
WifiEspNow.onReceive(sensor_espnow_receive_cb, sensor_handle);
```

> **Note:** ESP-NOW is treated as an emergency state. Safety-critical actuator control still requires stable Wi-Fi for reliable authorization.

### 3.5 Core IOT Implementation

The Core IOT platform (ThingsBoard-based) serves as the central monitoring and control hub.

#### Device Connectivity

- ESP32-S3 connects in **STA mode** via MQTT to `app.coreiot.io:1883`
- Each device authenticated by a unique **Access Token** tied to its MAC address
- Telemetry published in JSON: `temperature`, `humidity`, `gas`, `ratio`, relay and buzzer states

#### Role-Based Device Profiles

| Profile | Role | Primary Action |
|---|---|---|
| **Sensor Node** | Environmental monitoring + Edge AI | `Post telemetry` → Cloud |
| **Controller Node** | Physical actuation | Receives `RPC Request to Device` → drives relays/fans |

#### Rule Chain Architecture

```
Root Rule Chain
    ├── [Post telemetry]        → RC Process Telemetry
    │       ├── Save Timeseries         (immediate DB write for dashboard)
    │       ├── Originator Attributes   (fetch dynamic gas threshold)
    │       ├── Filter Script           (gas ratio > threshold?)
    │       │       ├── TRUE → Create Alarm
    │       │       └── TRUE → RC Action Control
    │       └── ...
    └── [RPC Request to Device] → RC Control Action
```

The **Filter Script** compares the live gas ratio against a server-side configurable threshold — eliminating hardcoded firmware limits:

```javascript
var currentGas = msg.ratio;
var safeThreshold = parseFloat(metadata.cs_gas_threshold);
return typeof currentGas !== 'undefined' && currentGas > safeThreshold;
```

When triggered, the **Action Control** sub-chain dispatches an RPC to the controller node:

```javascript
rpcPayload.method = "Emergency_Action";
rpcPayload.params = { "fan_state": true, "valve_state": false };
```

### 3.6 Edge AI Deployment (TinyML)

A lightweight gas-leak classification model is deployed directly on the ESP32-S3, enabling autonomous alert decisions without cloud connectivity.

#### Dataset

| Label | State | Samples | Proportion |
|---|---|---|---|
| 0 | Normal | 1699 | 51.14% |
| 1 | Warning | 931 | 28.03% |
| 2 | Critical | 692 | 20.83% |
| **Total** | | **3322** | **100%** |

Collected from a simulated environment with three sensor features: `temperature`, `humidity`, `gas` (raw MQ ADC).

#### Model Architecture

| Layer | Type | Units | Activation |
|---|---|---|---|
| 1 | Normalization (embedded) | — | — |
| 2 | Dense | 32 | ReLU |
| 3 | Dense | 16 | ReLU |
| 4 | Dense | 8 | ReLU |
| 5 | Dense (output) | 3 | Softmax |

The normalization layer is adapted on training data and embedded into the model — no external preprocessing parameters needed at inference time.

#### Training Configuration

- **Optimizer:** Adam (lr = 1e-3)
- **Loss:** Sparse Categorical Cross-Entropy
- **Batch size:** 32 | **Max epochs:** 100
- **Early stopping:** patience = 5 on `val_loss`, `restore_best_weights=True`

#### Evaluation Results

**Overall test accuracy: 95.70%** on 3,322 held-out samples.

| Class | Precision | Recall | F1-Score | Support |
|---|---|---|---|---|
| 0 – Normal | 0.9746 | 0.9706 | 0.9726 | 1699 |
| 1 – Warning | 0.9331 | 0.9882 | 0.9598 | 931 |
| 2 – Critical | 0.9472 | 0.8815 | 0.9132 | 692 |
| **Macro avg** | **0.9516** | **0.9468** | **0.9485** | 3322 |

**Confusion matrix:**

| Actual \ Predicted | Normal (0) | Warning (1) | Critical (2) |
|---|---|---|---|
| Normal (0) | **1649** | 22 | 28 |
| Warning (1) | 5 | **920** | 6 |
| Critical (2) | 38 | 44 | **610** |

#### Firmware Integration

The quantized `.tflite` model is serialized as a C array (`gas_leak_model.h`) and embedded directly into the firmware. At runtime:

1. A dedicated FreeRTOS task with **8 KB stack** and **32 KB static tensor arena** is spawned
2. The task blocks on an event flag until new sensor data is available
3. Averaged `[temperature, humidity, gas]` values are written into the TFLite input tensor
4. The interpreter runs the forward pass; the argmax of the output softmax gives the class
5. The result is written back to the shared sensor structure (mutex-protected) and consumed by the UI, WebSocket dashboard, and MQTT publisher

**Live serial monitor output (Normal state):**

```
Published telemetry data to CoreIOT
Sensor raw - Temp: 29.36, Humi: 48.63, Gas: 116.0000, Baseline: 118.7600
Sensor Avg - Temp: 29.36, Humi: 48.63, Gas: 116.0000, Ratio: 0.9768, Predicted: 0
Sensor raw - Temp: 29.35, Humi: 48.60, Gas: 109.0000, Baseline: 118.7600
Sensor Avg - Temp: 29.35, Humi: 48.60, Gas: 109.0000, Ratio: 0.9178, Predicted: 0
```

---

## 4. Experimental Results

| Metric | Result |
|---|---|
| TinyML test accuracy | **95.70%** |
| Macro F1-Score | **0.9485** |
| Warning class recall | **0.9882** (highest — critical for safety) |
| Critical class recall | **0.8815** (primary area for improvement) |
| WebSocket update latency | Low (push-based, no polling overhead) |
| Cloud publish interval | Continuous, per sensor cycle |
| ESP-NOW fallback | Functional, zero code change in consumers |

---

## 5. Group Organization & Git Contribution

| Member | ID | Primary Responsibilities |
|---|---|---|
| Nguyễn Minh Hiển | 2310998 | RTOS architecture, CoreIOT integration, overall system design |
| Lê Văn Đình Huy | 2311160 | Web server, TinyML firmware integration |
| Bành Huỳnh Minh Huy | 2311118 | UI subsystem (LCD, NeoPixel, LED, Buzzer, Relay), sensor subsystem, ESP-NOW |


---

## 6. Conclusions

This project successfully implements a complete, multi-layered IoT architecture on the ESP32-S3 (YoloUNO) platform. Key outcomes:

- **FreeRTOS** enables deterministic, concurrent execution of five independent subsystems without global variable coupling
- **TinyML** achieves 95.70% classification accuracy with a model compact enough to be embedded as a firmware C array, enabling autonomous alerting with zero cloud latency
- **WebSocket + mDNS** provides a seamless local monitoring experience without IP configuration
- **ESP-NOW fallback** ensures the system maintains local situational awareness even when internet connectivity is lost
- **ThingsBoard Rule Chains** externalize alarm threshold logic, making the system reconfigurable without reflashing firmware

### Future Work

- Improve Critical class recall (currently 88.15%) via additional data collection or class-weighted loss
- Pin FreeRTOS tasks to specific ESP32-S3 cores for tighter latency guarantees
- Extend the dataset with real-world gas leak scenarios for improved generalization
- Add HTTPS/WSS for encrypted local web server communication

---

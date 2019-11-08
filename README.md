# Electrocardiogram IoT Application

This project prototypes using an embedded IoT device (ESP32) to detect anomalies in human cardiovascular behavior. It uses machine learning in order to classify EKG patterns, and communicates findings with other devices (i.e. an Android application). The prototype also takes advantage of edge-computing in order to process problems which require more work than appropriate for the embedded application. 

## Hardware

1. ESP32-WROOM-32
2. Analog Devices AD8232 Heart Rate Monitor
3. A single LED (optional)

## Software

1. ESP-IDF (Espressif Development Toolchain)
2. FreeRTOS (bundled with ESP-IDF, so no need to get it separately)

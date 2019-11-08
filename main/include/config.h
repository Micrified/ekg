#if !defined(CONFIG_H)
#define CONFIG_H

/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 04/10/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  The configuration header holds configurable device information. Examples i *
 *  nclude the device hostname on WiFi, and the name broadcast over BLE        *
 *                                                                             *
 *******************************************************************************
*/


// The identifier or name of the device for all services
#define DEVICE_SERVICE_NAME				"Somnox-ESP32"


// The identifier for the device over WiFi
#define DEVICE_WIFI_SERVICE_NAME		DEVICE_SERVICE_NAME


// The identifier for the device over Bluetooth Low Energy
#define DEVICE_BLE_SERVICE_NAME			DEVICE_SERVICE_NAME


// The identifier for the device over Bluetooth Classic A2DP
#define DEVICE_A2DP_SERVICE_NAME		DEVICE_SERVICE_NAME


// The interval (in seconds) after which telemetry is sent
#define DEVICE_TELEMETRY_PERIOD			15


/*
 *******************************************************************************
 *                                    Pins                                     *
 *******************************************************************************
*/


// Pin used for the I2S channel select
#define DEVICE_I2S_LRCK_PIN             22


// Pin used for the I2S data in
#define DEVICE_I2S_DATA_PIN             25


// Pin used for the I2S bit clock
#define DEVICE_I2S_BCK_PIN              26


// Pin used for the Bluetooth Low Energy status LED
#define DEVICE_BLE_LED_PIN              19


// Pin used for the WiFi status LED
#define DEVICE_WIFI_LED_PIN             18


/*
 *******************************************************************************
 *                                 Task Memory                                 *
 *******************************************************************************
*/


// Stack size (words) for the BLE task
#define STACK_SIZE_BLE_MANAGER          2048


// Stack size (words) for the WiFi task 
#define STACK_SIZE_WIFI_MANAGER         2048


// Stack size (words) for the telemetry task
#define STACK_SIZE_TELEMETRY_MANAGER    2048


// Stack size (words) for the LED task
#define STACK_SIZE_LED_MANAGER          1024


// Stack size (words) for the stream task
#define STACK_SIZE_STREAM_MANAGER       2048


#endif
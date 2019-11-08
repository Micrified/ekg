#if !defined(CONFIG_H)
#define CONFIG_H

/*
 *******************************************************************************
 *                          (C) Copyright 2019 <None>                          *
 * Created: 08/11/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 * - Sonnya Dellarosa                                                          *
 *                                                                             *
 * Description:                                                                *
 *  Holds configuration data for the RTOS. This includes BLE identifiers, task *
 *   memory size, and more                                                     *
 *                                                                             *
 *******************************************************************************
*/


// The identifier or name of the device for all services
#define DEVICE_SERVICE_NAME				"EKG-ESP32"


// The identifier for the device over Bluetooth Low Energy
#define DEVICE_BLE_SERVICE_NAME			DEVICE_SERVICE_NAME


// The interval (in seconds) after which telemetry is sent
#define DEVICE_TELEMETRY_PERIOD			15


/*
 *******************************************************************************
 *                                    Pins                                     *
 *******************************************************************************
*/


// Pin used for the Bluetooth Low Energy status LED
#define DEVICE_BLE_LED_PIN              19


/*
 *******************************************************************************
 *                                 Task Memory                                 *
 *******************************************************************************
*/


// Stack size (words) for the BLE task
#define STACK_SIZE_BLE_MANAGER          2048


// Stack size (words) for the telemetry task
#define STACK_SIZE_TELEMETRY_MANAGER    2048


// Stack size (words) for the LED task
#define STACK_SIZE_LED_MANAGER          1024


#endif
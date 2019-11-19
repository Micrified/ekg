#if !defined(CONFIG_H)
#define CONFIG_H


#include "driver/adc.h"


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


// The interval (in milliseconds) after which telemetry is sent
#define DEVICE_TELEMETRY_PERIOD			10


/*
 *******************************************************************************
 *                                    Pins                                     *
 *******************************************************************************
*/


// Pin used for the Bluetooth Low Energy status LED
#define DEVICE_BLE_LED_PIN              19

// Pin for reading EKG voltage
#define DEVICE_EKG_PIN					ADC2_CHANNEL_6


/*
 *******************************************************************************
 *                                   Sensors                                   *
 *******************************************************************************
*/


// How many milliseconds the device should wait before checking the sensor
#define DEVICE_SENSOR_POLL_PERIOD_MS    10


/* The amount of sensor readings the device should attempt before pushing the
 * results (if any) to a global buffer. Increasing this means the task is
 * stalled less to update the buffer, but also means data will become available
 * less often. 
 */
#define DEVICE_SENSOR_PUSH_BUF_SIZE     256


// The threshold, at or over which, readings are considered to be R peaks
#define DEVICE_R_PEAK_THRESHOLD         2450


// The threshold, at or which under, readings are considered to be R dips
#define DEVICE_R_DIP_THRESHOLD          930


/*
 *******************************************************************************
 *                                 Task Memory                                 *
 *******************************************************************************
*/


// Stack size (words) for the BLE task
#define STACK_SIZE_BLE_MANAGER          2048


// Stack size (words) for the EKG task
#define STACK_SIZE_EKG_MANAGER          2048


// Stack size (words) for the sample task
#define STACK_SIZE_SAMPLE_MANAGER       1024


#endif
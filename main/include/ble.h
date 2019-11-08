#ifndef BLE_H
#define BLE_H


/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 26/08/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  Wrapper for Bluedroid BLE                                                  *
 *                                                                             *
 *******************************************************************************
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"

#include "sdkconfig.h"

#include "err.h"
#include "ble_profiles.h"		// Custom BLE profiles
#include "tasks.h"
#include "ipc.h"
#include "config.h"


/*
 *******************************************************************************
 *                         External Symbolic Constants                         *
 *******************************************************************************
*/


// The name of the device as seen in the GAP advertising messages
#define BLE_DEVICE_NAME			DEVICE_BLE_SERVICE_NAME

// The Message-Transmission-Unit to use with clients
#define BLE_MTU_SIZE			512

// The maximum size (in bytes) of the response/indicate message (< MTU size) 
#define BLE_RSP_MSG_MAX_SIZE	20


/*
 *******************************************************************************
 *                              Type Definitions                               *
 *******************************************************************************
*/


// Enumeration of supported Application Profiles
enum {
	/*************************************************************************/
	APP_PROFILE_WIFI,

	/*************************************************************************/
	// The number of application profiles (do not remove)
	APP_PROFILE_COUNT
};


/*
 *******************************************************************************
 *                          External Global Variables                          *
 *******************************************************************************
*/




/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* @brief Initializes the BT Controller in BLE mode, and Bluetooth Stack
 * @note NVS Flash storage should be initialized prior to calling this
*/
esp_err_t ble_init (void);


/* @brief Dispatches a message to the connected BLE device
 * @note Send this only after having called ble_init and knowing a device is 
 *       connected
 * @param
 * - len: The length (size) of the uint8_t byte buffer to send
 * - buffer: The uint8_t buffer pointer
 * @return
 * - ESP_OK: The message was sent
 * - ESP_ERR_INVALID_ARG: The give buffer was null, among other reasons
 * - ESP_ERR_INVALID_SIZE The message given is too large to fit in an MTU
 * - Other errors resulting from esp_ble_gatts_send_indicate are possible
*/
esp_err_t ble_send (size_t len, uint8_t *buffer);


#endif
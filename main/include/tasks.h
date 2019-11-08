#if !defined(TASKS_H)
#define TASKS_H


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
 *  Describes event group flags for system-wide signaling                      *
 *                                                                             *
 *******************************************************************************
*/


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


/*
 *******************************************************************************
 *                             Symbolic Constants                              *
 *******************************************************************************
*/


// BLE Flag-Group
#define FLAG_BLE_CONNECTED          0x0001    // A device is connected via BLE
#define FLAG_BLE_DISCONNECTED       0x0002    // No device is connected via BLE
#define FLAG_BLE_RECV_MSG           0x0004    // Message was received over BLE
#define FLAG_BLE_SEND_MSG           0x0008    // Message ready to send over BLE

// BLE Flag-Group Mask
#define MASK_BLE_FLAGS              0x000F    // Masks all BLE event bits

// WiFi Flag-Group
#define FLAG_WIFI_CONNECTED         0x0010    // WiFi driver is connected
#define FLAG_WIFI_DISCONNECTED      0x0020    // WiFi driver is disconnected
#define FLAG_WIFI_START             0x0040    // Request to connect WiFi
#define FLAG_WIFI_STOP              0x0080    // Request to disconnect WiFi

// WiFi Flag-Group Mask
#define MASK_WIFI_FLAGS             0x00F0    // Masks all WiFi event bits

// Telemetry Flag-Group
#define FLAG_TELEMETRY_START        0x0100    // Request to start telemetry
#define FLAG_TELEMETRY_STOP         0x0200    // Request to stop telemetry
#define FLAG_TELEMETRY_SEND         0x0400    // Request to dispatch telemetry

// Telemetry Flag-Group Mask
#define MASK_TELEMETRY_FLAGS        0x0700    // Masks all Telemetry event bits

// Stream Flag-Group
#define FLAG_STREAM_START           0x0800    // Signal to begin a stream
#define FLAG_STREAM_STOP            0x1000    // Signal to stop a stream
#define FLAG_STREAM_TICK            0x2000    // Timer-triggered stream tick

// Stream Flag-Group Mask
#define MASK_STREAM_FLAGS           0x3800    // Masks all Stream event bits


/*
 *******************************************************************************
 *                          External Global Variables                          *
 *******************************************************************************
*/


/* FreeRTOS Event Group
 * Is a sixteen-bit field for user-defined events. Tasks may wait (block) on
 * certain bit(s). Tasks can also synchronize on bits
 * 
 * This is the event-group for all user-defined system tasks
*/
EventGroupHandle_t g_event_group;


#endif

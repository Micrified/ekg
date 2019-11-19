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


// EKG Flag-Group
#define FLAG_EKG_START              0x0010    // EKG will relay samples
#define FLAG_EKG_STOP               0x0020    // EKG will do nothing
#define FLAG_EKG_TICK               0x0040    // EKG will process sample buffer


// EKG Flag-Group Mask
#define MASK_EKG_FLAGS              0x0070    // Masks all EKG event bits


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

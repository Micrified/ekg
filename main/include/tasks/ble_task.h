#if !defined(BLE_TASK_H)
#define BLE_TASK_H


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
 *  Header for the BLE task. This task interfaces with the BLE driver task and *
 *   notifies other tasks of changes to the connection state. It is also respo *
 *  nsible for writing messages to the GATT characteristic and processing mess *
 *  ages sent from the connected device                                        *
 *                                                                             *
 *******************************************************************************
*/


#include "esp_system.h"
#include "esp_log.h"
#include "err.h"
#include "msg.h"
#include "tasks.h"
#include "ipc.h"
#include "ble.h"


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Global variables (comparator type, comparator value)
extern uint8_t g_cfg_comp;
extern uint16_t g_cfg_val;

// Global variables holding the normal wave training data set
extern uint16_t g_n_periods[20];
extern uint16_t g_n_amplitudes[20];

// Global variables holding the atrial wave training data set
extern uint16_t g_a_periods[10];
extern uint16_t g_a_amplitudes[10];

// Global variables holding the ventrical wave training data set
extern uint16_t g_v_periods[10];
extern uint16_t g_v_amplitudes[10];


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* Automaton responsible for managing events generated by the BLE driver
 *
*/
void task_ble_manager (void *args);


#endif
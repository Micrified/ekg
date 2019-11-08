#if !defined(IPC_H)
#define IPC_H


/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 05/09/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  Defines IPC elements such as queues and message buffers for system-wide ac *
 *  cess                                                                       *
 *                                                                             *
 *******************************************************************************
*/


#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"


/*
 *******************************************************************************
 *                             Symbolic Constants                              *
 *******************************************************************************
*/


// Maximum queue element data size
#define TASK_QUEUE_DATA_MAX         128


// Maximum number of queue elements a queue can hold
#define TASK_QUEUE_CAPACITY			16


// Maximum number of ticks a task should wait before abandoning queue operation
#define TASK_QUEUE_MAX_TICKS		16


/*
 *******************************************************************************
 *                              Type Definitions                               *
 *******************************************************************************
*/


// Describes a variable-size queue element for IPC messages
typedef struct {
	uint8_t id;                            // Identifier (for selecting socket)
	size_t size;                           // Size of the message (in bytes)
	uint8_t data[TASK_QUEUE_DATA_MAX];     // Message buffer
} task_queue_msg_t;


/*
 *******************************************************************************
 *                          External Global Variables                          *
 *******************************************************************************
*/


/* FreeRTOS Bluetooth IPC Message Receive Queue
 * This queue holds raw message data captured by the BLE driver wrapper. It is
 * thread safe
 *
 * Read-By: 
 * - task_wifi_manager: When credentials are supposedly ready
 * Written-By:
 * - sys_ble_wrapper: When any message is received
*/
QueueHandle_t g_ble_rx_queue;


/* FreeRTOS Bluetooth IPC Message Transmit Queue
 * This queue holds raw message data to be transmitted to a (possibly)
 * connected device. It is thread safe
 *
 * Read-By:
 * - task_ble_manager: When any response is to be sent to a device
 * Written-By:
 * - task_wifi_manager: When disconnect occurred
*/
QueueHandle_t g_ble_tx_queue;


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* @brief Initializes all IPC queues exposed in the interface
 * @param None
 * @return
 * - ESP_OK: Success
 * - ESP_ERR_NO_MEM: Insufficient memory
*/
esp_err_t ipc_init (void);


/* @brief Loads the given data buffer into the supplied IPC queue
 *        This function blocks, but will return if the queue cannot
 *        take anymore data. Data is sliced into message-sized chunks
 * @note  This should be thread-safe because queues are thread-safe
 *
 * @param
 * - queue:  The QueueHandle_t queue handle
 * - id:     An identifier (optional) for data association
 * - size:   The size (in bytes) of the data to be enqueued
 * - buffer: The buffer from which the data will be read
 *
 * @return
 * - ESP_OK: Success
 * - ESP_ERR_TIMEOUT: The data could not be enqueued before timeout
*/
esp_err_t ipc_enqueue (QueueHandle_t queue, uint8_t id, size_t size, 
    void *buffer);


#endif

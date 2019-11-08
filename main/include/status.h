#if !defined(STATUS_H)
#define STATUS_H


/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 03/10/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  Holds a function for dispatching a status message. This functionality is u *
 *  sed across several tasks, so it makes sense to dedicate a source file for  *
 *  it                                                                         *
 *                                                                             *
 *******************************************************************************
*/


#include <inttypes.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "tasks.h"
#include "msg.h"
#include "ipc.h"
#include "err.h"


/*
 *******************************************************************************
 *                             Symbolic Constants                              *
 *******************************************************************************
*/


#define SET_STATE_BIT_ATOMIC(bit) {         \
    portENTER_CRITICAL(&g_state_mutex);     \
    g_state_flag |= (bit);                  \
    portEXIT_CRITICAL(&g_state_mutex);      \
}


#define CLEAR_STATE_BIT_ATOMIC(bit) {       \
    portENTER_CRITICAL(&g_state_mutex);     \
    g_state_flag &= ~(bit);                 \
    portEXIT_CRITICAL(&g_state_mutex);      \
}


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Global state data word
extern uint32_t g_state_data;


// Global state flag (may be subject race conditions | see msg.h for bits)
extern uint8_t g_state_flag;


// Global mutex for controlled access to the state flag
extern portMUX_TYPE g_state_mutex;


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* @brief: Dispatches a status message to the outgoing BLE queue
 * 
 * @note:  The message is serialized, and the data placed on an internal queue
 *         and sent to the BLE manager. That task will then send it to the
 *         receiving device (your mobile phone) 
 * 
 * @param:
 *  - task_id: Tag of calling task. Will be used in error log for debugging
 *
*/
void dispatch_status_message (const char *task_tag);


#endif

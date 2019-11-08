#if !defined(TELEMETRY_TASK_H)
#define TELEMETRY_TASK_H


/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 03/10/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  The telemetry task dispatches POST request periodically to a remote sink.  *
 *  This is set by the user, and the task can be toggled on or off             *
 *                                                                             *
 *******************************************************************************
*/


#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "tasks.h"
#include "status.h"


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Global state flag (may be subject race conditions | see msg.h for bits)
extern uint8_t g_state_flag;


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* Automaton responsible for assembling a telemetry message and pushing the
 * data over a socket to a web-server. Is triggered by a software-timer, and
 * only sends if enabled by the user.
*/
void task_telemetry_manager (void *args);


#endif

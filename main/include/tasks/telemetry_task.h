#if !defined(TELEMETRY_TASK_H)
#define TELEMETRY_TASK_H


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
 *  The telemetry task dispatches data periodically. This can be virtually any *
 *   type of telemetry, but will be EKG related                                *
 *                                                                             *
 *******************************************************************************
*/


#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "driver/adc.h"
#include "tasks.h"
#include "status.h"
#include "config.h"


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

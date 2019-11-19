#if !defined(SAMPLE_TASK_H)
#define SAMPLE_TASK_H


/*
 *******************************************************************************
 *                          (C) Copyright 2019 <None>                          *
 * Created: 19/11/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 * - Sonnya Dellarosa                                                          *
 *                                                                             *
 * Description:                                                                *
 *  Sampling tasks reads values from the ADC in bursts, and then processes the *
 *  m to a global data buffer. It should be pinned to a dedicated core as it m *
 *  ust run more or less continuously without interruption                     *
 *                                                                             *
 *******************************************************************************
*/


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_log.h"
#include "err.h"
#include "msg.h"
#include "tasks.h"
#include "ipc.h"
#include "config.h"


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Buffer to hold samples. Periodically updated
extern uint16_t g_sample_buffer[DEVICE_SENSOR_PUSH_BUF_SIZE];


// Global mutex for controlled access to the sensor buffer
extern portMUX_TYPE g_sample_buffer_mutex;


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* Automaton responsible for gathering and processing samples */
void task_sample_manager (void *args);


#endif
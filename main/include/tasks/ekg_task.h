#if !defined(EKG_TASK_H)
#define EKG_TASK_H


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
 *  The EKG task monitors the samples, and locates peaks. It then classifies t *
 *  hem, or transmits them for labelling                                       *
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
#include "classifier.h"


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Buffer to hold samples. Periodically updated
extern uint16_t g_sample_buffer[DEVICE_SENSOR_PUSH_BUF_SIZE];


// Global mutex for controlled access to the sensor buffer
extern portMUX_TYPE g_sample_buffer_mutex;


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


/* Automaton responsible for classifying samples */
void task_ekg_manager (void *args);


#endif
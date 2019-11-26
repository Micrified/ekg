#if !defined(CLASSIFIER_H)
#define CLASSIFIER_H


/*
 *******************************************************************************
 *                          (C) Copyright 2019 <None>                          *
 * Created: 15/11/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 * - Sonnya Dellarosa                                                          *
 *                                                                             *
 * Description:                                                                *
 *  KNN Classifier                                                             *
 *                                                                             *
 *******************************************************************************
*/


#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "esp_system.h"
#include "esp_log.h"
#include "ekg_task.h"


/*
 *******************************************************************************
 *                         External Symbolic Constants                         *
 *******************************************************************************
*/


// Value for K of the KNN classifier
#define 	K_VALUE                                    4


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
 *                              Type Definitions                               *
 *******************************************************************************
*/


// Type describing the labels that can be ascribed to samples
typedef enum {
	SAMPLE_LABEL_UNKNOWN = 0x0,
	SAMPLE_LABEL_NORMAL,
	SAMPLE_LABEL_ATRIAL,
	SAMPLE_LABEL_VENTRICAL
} sample_label_t;


// Structure describing a neighbor
typedef struct {
    sample_label_t label;
    float distance;
} neighbor_t;


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* @brief Classifies a sample using the KNN method.
 *
 * @note Sets of samples cannot be empty.
 *
 * @param
 * - amplitude : Amplitude of the new sample to be classified.
 * - rr_period : RR period of the new sample to be classified.
 *
 * @return Label of the new sample
*/
sample_label_t classify (uint16_t amplitude, uint16_t rr_period);


#endif
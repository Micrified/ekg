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
#include "esp_system.h"
#include "esp_log.h"


/*
 *******************************************************************************
 *                         External Symbolic Constants                         *
 *******************************************************************************
*/


// Size of the training sample, the smaller the faster
#define 	TRAINING_SAMPLE_SIZE                       10

// Value for K of the KNN classifier
#define 	K_VALUE                                    4


/*
 *******************************************************************************
 *                              Type Definitions                               *
 *******************************************************************************
*/


// Structure describing a sample
typedef struct {
    uint16_t amplitude;
    uint16_t period;
    label_type_t label;
    float distance;
} sample_t;


// Types of label
typedef enum {
    LABEL_A = 0,
    LABEL_N,
    LABEL_V,
    LABEL_NONE
} label_t;


// Structure defining a point, for comparison with other points
typedef struct {
    sample_t sample;
    float distance;
} point_t;


// Structures defining data arrays
sample_t training_set_A[TRAINING_SAMPLE_SIZE]
sample_t training_set_N[TRAINING_SAMPLE_SIZE]
sample_t training_set_V[TRAINING_SAMPLE_SIZE]



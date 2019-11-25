#include <math.h>
#include "classifier.h"

// Calculate the distance between two points
float calculate_distance(sample_t a, sample_t b) {
    float distance = 0;
    
    // Calculate Euclidean distance
    distance = sqrt((a.amplitude - b.amplitude) * (a.amplitude - b.amplitude) + 
                 (a.period - b.period) * (a.period - b.period)); 

    // Return the distance
    return distance;
}

// Classifies a sample
label_t classify (struct sample_t sample) {
    label_t label = LABEL_NONE;

    // Store distance to each point
    point_t point_set[TRAINING_SAMPLE_SIZE * 3];
    
    // Fill distances from point to other points
    int i;
    for (i = 0; i < TRAINING_SAMPLE_SIZE; i++) {
        
    }

    // TODO: Classification algorithm
    // Check for distance with every neighbor
    // Find K closest neighbors (K = K_VALUE)

    // Return the label
    return label;
}

// TODO: Add other functions to save samples to the
// training set
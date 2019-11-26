#include "classifier.h"


/*
 *******************************************************************************
 *                        External Function Definitions                        *
 *******************************************************************************
*/


// Calculate the distance between two points
double calculate_distance (uint16_t amplitude_1, uint16_t rr_period_1,
							uint16_t amplitude_2, uint16_t rr_period_2) {
    double distance = 0;
    
    // Calculate Euclidean distance
    distance = sqrt((amplitude_1 - amplitude_2) * (amplitude_1 - amplitude_2) + 
        (rr_period_1 - rr_period_2) * (rr_period_1 - rr_period_2));

    // Return the distance
    return distance;
}


// Comparator function to sort by increasing order of distance 
int compare (const void * s1, const void * s2) {

	neighbor_t *n1 = (neighbor_t *) s1;
	neighbor_t *n2 = (neighbor_t *) s2;

    return (n1->distance - n2->distance); 
}


// Function to generate an array of neighbors
void get_neighbors (uint16_t amplitude, uint16_t rr_period, neighbor_t* neighbors) {
	uint16_t n_amplitude;
	uint16_t n_rr_period;
	sample_label_t n_label;
	double distance;
	int i;

    // Calculate distance with each neighbor
    for (i = 0; i < 40; i++) {

		// Get a neighbor's amplitude, period, and label
		if (i >= 0 && i < 20) {
			n_amplitude = g_n_amplitudes[i];
			n_rr_period = g_n_periods[i];
			n_label = SAMPLE_LABEL_NORMAL;
		}
		else if (i >= 20 && i < 30) {
			n_amplitude = g_a_amplitudes[i-20];
			n_rr_period = g_a_periods[i-20];
			n_label = SAMPLE_LABEL_ATRIAL;
		}
		else if (i >= 30 && i < 40) {
			n_amplitude = g_v_amplitudes[i-30];
			n_rr_period = g_v_periods[i-30];
			n_label = SAMPLE_LABEL_VENTRICAL;
		}

		// Calculate distance between sample and current neighbor
		distance = calculate_distance(amplitude, rr_period, 
			n_amplitude, n_rr_period);

		// Add calculated distance to array
		neighbors[i] = (neighbor_t) {
			.distance = distance,
			.label = n_label
		};
	}
}


// Function to count a label in neighbor array
void count_labels (neighbor_t* neighbors, uint8_t* N, uint8_t* A, uint8_t* V) {
    int i;

    // Start counters at zero
    *N = 0;
    *A = 0;
    *V = 0;

    // Find the most frequent label in the K closest samples
    // use squared values of the order to avoid ties
	for (i = K_VALUE; i > 0 ; i--) {
		if ((neighbors[i]).label == SAMPLE_LABEL_NORMAL) {
            *N += i*i;
        } else if ((neighbors[i]).label == SAMPLE_LABEL_ATRIAL) {
            *A += i*i;
        } else if ((neighbors[i]).label == SAMPLE_LABEL_VENTRICAL) {
            *V += i*i;
        }
	}
}
  

// Classifies a sample
sample_label_t classify (uint16_t amplitude, uint16_t rr_period) {
	sample_label_t label = SAMPLE_LABEL_UNKNOWN;

    // Dynamically allocate neighbor array
    neighbor_t neighbors[40];

    // Variables to store the count of each label
    uint8_t N, A, V;
	
    // Get an array of neighbors
    get_neighbors(amplitude, rr_period, neighbors);

	// Sort distance array by the smallest distance
	qsort(neighbors, 40, sizeof(neighbor_t), compare);

	// Count each label
    count_labels(neighbors, &N, &A, &V);

    // Find the most frequent
    if (N > A && N > V) {
        label = SAMPLE_LABEL_NORMAL;
    } else if (A > N && A > V) {
        label = SAMPLE_LABEL_ATRIAL;
    } else if (V > N && V > A) {
        label = SAMPLE_LABEL_VENTRICAL;
    }

	return label;
}

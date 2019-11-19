#include "ekg_task.h"

/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// The local sample buffer
static uint16_t g_local_sample_buffer[DEVICE_SENSOR_PUSH_BUF_SIZE];


/*
 *******************************************************************************
 *                          Internal Type Definitions                          *
 *******************************************************************************
*/


// Type describing the labels that can be ascribed to samples
typedef enum {
	SAMPLE_LABEL_UNKNOWN,
	SAMPLE_LABEL_NORMAL,
	SAMPLE_LABEL_ATRIAL,
	SAMPLE_LABEL_VENTRICAL
} sample_label_t;


/*
 *******************************************************************************
 *                        Internal Function Definitions                        *
 *******************************************************************************
*/


// Classifies a sample
static sample_label_t classify_knn (uint16_t amplitude, uint16_t rr_period) {
	sample_label_t label = SAMPLE_LABEL_UNKNOWN;

	// TODO: Implement Sonnya's KNN here

	return label;
}


// Dispatches a serialized message containing the amplitude and RR-peak period
static void send_sample (uint16_t amplitude, uint16_t rr_period, uint8_t lab) {
	uint8_t buffer[MSG_BUFFER_MAX] = {0};

	// Construct message body
	msg_body_t body = (msg_body_t) {
		.msg_sample = (msg_sample_data_t) {
			.label     = lab,
			.amplitude = amplitude,
			.period    = rr_period
		}
	};

	// Construct the message
	msg_t message = (msg_t){
		.type = MSG_TYPE_SAMPLE_DATA,
		.body = body
	};

	// Serialize the message
	size_t z = msg_pack(&message, buffer);

	// Enqueue message for transmission
	if (ipc_enqueue(g_ble_tx_queue, 0x0, z, buffer) != ESP_OK) {
		ESP_LOGE("EKG", "Problem pushing message data!");
	}

	return;
}


/*
 *******************************************************************************
 *                            Function Definitions                             *
 *******************************************************************************
*/


void task_ekg_manager (void *args) {
	uint32_t  flags    = 0x0;
	uint8_t   relay    = 0x0;     // Initially not relaying 
	int p1, p2;

	do {

		// Wait indefinitely for a flag to be set (clear all automatically)
		flags = xEventGroupWaitBits(g_event_group, MASK_EKG_FLAGS, pdTRUE,
			pdFALSE, portMAX_DELAY);

		// If the start flag is set: Enable relaying
		if (flags & FLAG_EKG_START) {
			relay = 1;
		}

		// If the stop flag is set: Disable relaying
		if (flags & FLAG_EKG_STOP) {
			relay = 0;
		}

		// If a tick occurred: Copy and process the buffer
		if (flags & FLAG_EKG_TICK) {

			// Lock the mutex and copy over the data
			portENTER_CRITICAL(&g_sample_buffer_mutex);
			memcpy(g_local_sample_buffer, g_sample_buffer, 
				DEVICE_SENSOR_PUSH_BUF_SIZE * sizeof(uint16_t));
			portEXIT_CRITICAL(&g_sample_buffer_mutex);

			// Look for a first peak
			for (p1 = 0; p1 < DEVICE_SENSOR_PUSH_BUF_SIZE; ++p1) {
				if (g_local_sample_buffer[p1] > DEVICE_R_PEAK_THRESHOLD) {
					break;
				}
			}

			// If no peak found, then just continue now
			if (p1 >= DEVICE_SENSOR_PUSH_BUF_SIZE) continue;

			// Search for a second peak
			for (p2 = p1 + 1; p2 < DEVICE_SENSOR_PUSH_BUF_SIZE; ++p2) {
				if (g_local_sample_buffer[p2] > DEVICE_R_PEAK_THRESHOLD) {
					break;
				}
			}

			// If no peak found, then also continue here
			if (p2 >= DEVICE_SENSOR_PUSH_BUF_SIZE) continue;

			// Otherwise compute the features
			uint16_t rr_period    = 10 * (p2 - p1); // Period in ms
			uint16_t amplitude    = g_local_sample_buffer[p2];
			uint8_t  label        = classify_knn(amplitude, rr_period);

			// Send sample (but only if in relay mode)
			if (relay) {
				send_sample(amplitude, rr_period, label);
			}

			// Flash LED to show pulse
		}

	} while (1);


	// Destroy task
	vTaskDelete(NULL);
}

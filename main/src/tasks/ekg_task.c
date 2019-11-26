#include "ekg_task.h"

/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// The local sample buffer
static uint16_t g_local_sample_buffer[DEVICE_SENSOR_PUSH_BUF_SIZE];

// Local variables holding the normal wave training data set
static uint16_t g_local_n_periods[20];
static uint16_t g_local_n_amplitudes[20];

// Local variables holding the atrial wave training data set
static uint16_t g_local_a_periods[10];
static uint16_t g_local_a_amplitudes[10];

// Local variables holding the ventrical wave training data set
static uint16_t g_local_v_periods[10];
static uint16_t g_local_v_amplitudes[10];


/*
 *******************************************************************************
 *                        Internal Function Definitions                        *
 *******************************************************************************
*/

// Classifies a sample
static sample_label_t classify_knn (uint16_t amplitude, uint16_t rr_period) {
	sample_label_t label = SAMPLE_LABEL_UNKNOWN;

	// TODO: Implement Sonnya's KNN here
	label = classify(amplitude, rr_period);

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
		return;
	}

	// Otherwise notify the BLE Manager to send it
	xEventGroupSetBits(g_event_group, FLAG_BLE_SEND_MSG);

	return;
}


// Configurable peak detector. Returns nonzero on peak, else zero
static int isPeak (uint16_t value, uint8_t comp, uint16_t threshold) {
	if (comp) {
		return (value <= threshold);   // comp != 0x0 -> LTE
	} else {
		return (value >= threshold);   // comp == 0x0 -> GTE
	}
}


/*
 *******************************************************************************
 *                            Function Definitions                             *
 *******************************************************************************
*/


#define LED_PIN              19


void task_ekg_manager (void *args) {
	uint32_t  flags    = 0x0;
	uint8_t   relay    = 0x0;     // Initially not relaying
	uint8_t   cfg_comp = 0x0;
	uint16_t  cfg_val  = 2450;    
	int p1, p2;

	// Configure output pin for LED
	gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

	do {

		// Unset LED
		gpio_set_level(LED_PIN, 0);

		// Wait indefinitely for a flag to be set (clear all automatically)
		flags = xEventGroupWaitBits(g_event_group, MASK_EKG_FLAGS | FLAG_BLE_DISCONNECTED, pdFALSE,
			pdFALSE, portMAX_DELAY);

        // Clear the EKG flags
        xEventGroupClearBits(g_event_group, MASK_EKG_FLAGS);

		// If the configuration flag is set: Update local configuration
		if (flags & FLAG_EKG_CONFIGURE) {
			cfg_comp = g_cfg_comp;
			cfg_val  = g_cfg_val;
			memcpy(g_local_n_periods,    g_n_periods,    20 * sizeof(uint16_t));
			memcpy(g_local_n_amplitudes, g_n_amplitudes, 20 * sizeof(uint16_t));
			memcpy(g_local_a_periods,    g_a_periods,    10 * sizeof(uint16_t));
			memcpy(g_local_a_amplitudes, g_a_amplitudes, 10 * sizeof(uint16_t));
			memcpy(g_local_v_periods,    g_v_periods,    10 * sizeof(uint16_t));
			memcpy(g_local_v_amplitudes, g_v_amplitudes, 10 * sizeof(uint16_t));

			// DEBUG: Log the training data
            for (int i = 0; i < 20; ++i) {
                printf("Normal: period = %u amplitude = %u\n", g_local_n_periods[i], g_local_n_amplitudes[i]);
            }
            for (int i = 0; i < 10; ++i) {
            	printf("Atrial: period = %u amplitude = %u\n", g_local_a_periods[i], g_local_a_amplitudes[i]);
            }
            for (int i = 0; i < 10; ++i) {
            	printf("Ventrical: period = %u amplitude = %u\n", g_local_v_periods[i], g_local_v_amplitudes[i]);
            }
		}

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

			// Flash LED to show pulse
 			gpio_set_level(LED_PIN, 1);

			// Lock the mutex and copy over the data
			portENTER_CRITICAL(&g_sample_buffer_mutex);
			memcpy(g_local_sample_buffer, g_sample_buffer, 
				DEVICE_SENSOR_PUSH_BUF_SIZE * sizeof(uint16_t));
			portEXIT_CRITICAL(&g_sample_buffer_mutex);

			// Just output the samples for now
			// for (p1 = 0; p1 < DEVICE_SENSOR_PUSH_BUF_SIZE; ++p1) {
			// 	printf("%u\n", g_local_sample_buffer[p1]);
			// }

			// Look for a first peak
			for (p1 = 0; p1 < DEVICE_SENSOR_PUSH_BUF_SIZE; ++p1) {
				if (isPeak(g_local_sample_buffer[p1], cfg_comp, cfg_val)) {
					break;
				}
			}

			// If no peak found, then just continue now
			if (p1 >= DEVICE_SENSOR_PUSH_BUF_SIZE) continue;

			// Search for a second peak
			for (p2 = p1 + 1; p2 < DEVICE_SENSOR_PUSH_BUF_SIZE; ++p2) {
				if (isPeak(g_local_sample_buffer[p2], cfg_comp, cfg_val)) {
					break;
				}
			}

			// If no peak found, then also continue here
			if (p2 >= DEVICE_SENSOR_PUSH_BUF_SIZE) continue;

			// Otherwise compute the features
			uint16_t rr_period    = 10 * (p2 - p1); // Period in ms
			uint16_t amplitude    = g_local_sample_buffer[p2];
			uint8_t  label        = classify_knn(amplitude, rr_period);

			printf("%u %u %u\n", rr_period, amplitude, label);

			// Send sample (but only if in relay mode)
			if (relay) {
				send_sample(amplitude, rr_period, label);
			}
		}

	} while (1);


	// Destroy task
	vTaskDelete(NULL);
}

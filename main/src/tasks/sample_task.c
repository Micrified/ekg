#include "sample_task.h"


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// The local sample buffer
static uint16_t g_local_sample_buffer[DEVICE_SENSOR_PUSH_BUF_SIZE];


/*
 *******************************************************************************
 *                            Function Definitions                             *
 *******************************************************************************
*/


void task_sample_manager (void *args) {
	int adc_val = 0;

	// The sampling period ~ (100Hz)
	const TickType_t period = 10 / portTICK_PERIOD_MS;

	// Configure ADC (ADC2, pin 14)
	adc2_config_channel_atten(DEVICE_EKG_PIN, ADC_ATTEN_0db);

	// Task loop
	do {

		// Fill the buffer with samples
		for (int i = 0; i < DEVICE_SENSOR_PUSH_BUF_SIZE; ++i) {

			// Delay for fixed sample intervals
			vTaskDelay(period);

			// Read ADC value (no error checking)
			adc2_get_raw(DEVICE_EKG_PIN, ADC_WIDTH_12Bit, &adc_val);

			// Push to buffer
			g_local_sample_buffer[i] = (uint16_t)adc_val;

		}

		// Lock the mutex and copy over the data
		portENTER_CRITICAL(&g_sample_buffer_mutex);
		memcpy(g_sample_buffer, g_local_sample_buffer, 
			DEVICE_SENSOR_PUSH_BUF_SIZE * sizeof(uint16_t));
		portEXIT_CRITICAL(&g_sample_buffer_mutex);

		// Notify the EKG task that new data is available
		xEventGroupSetBits(g_event_group, FLAG_EKG_TICK);

	} while (1);

	// Destroy task
	vTaskDelete(NULL);
}

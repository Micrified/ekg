#include "ipc.h"



/*
 *******************************************************************************
 *                        External Function Definitions                        *
 *******************************************************************************
*/


esp_err_t ipc_init (void) {

	g_ble_rx_queue = xQueueCreate(TASK_QUEUE_CAPACITY, 
		sizeof(task_queue_msg_t));
	g_ble_tx_queue = xQueueCreate(TASK_QUEUE_CAPACITY,
		sizeof(task_queue_msg_t));

	if (NULL == g_ble_tx_queue || NULL == g_ble_rx_queue) {
		return ESP_ERR_NO_MEM;
	}
	return ESP_OK;
}


esp_err_t ipc_enqueue (QueueHandle_t queue, uint8_t id, size_t size, 
    void *buffer) {
	size_t rem = size; off_t offset = 0;
	BaseType_t res = pdPASS;

	// While data remains to be sent and space remains in the queue
	while (rem > 0 && (res == pdPASS)) {

		// Compute message chunk size
		size_t z = (rem > TASK_QUEUE_DATA_MAX ? TASK_QUEUE_DATA_MAX : rem);

		// Build message
		task_queue_msg_t msg = (task_queue_msg_t) {
			.id = id,
			.size = z,
		};
		memcpy(msg.data, buffer + offset, z * sizeof(uint8_t));

		// Enqueue next message chunk
		res = xQueueSendToBack(queue, (void *)&msg, TASK_QUEUE_MAX_TICKS);

		// Update remaining size
		rem -= z;

		// Update offset
		offset += z;
	}

	return (res == pdPASS) ? ESP_OK : ESP_ERR_TIMEOUT;
}

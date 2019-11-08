#include "err.h"


/*
 *******************************************************************************
 *                        External Function Definitions                        *
 *******************************************************************************
*/


/* @brief Suspends a FreeRTOS task with a panic message to standard error
 * @param
 *    - msg: A constant-string message to be printed
 *    - err: An error value of type esp_err_t. If set to ESP_OK, no output from
 *           esp_err_to_name is included
 * @return None
*/
void task_panic (const char *msg, esp_err_t err) {
	if (err != ESP_OK) {
		fprintf(stderr, "Task Panic (%s:%d) | \"%s\" | %s\n",
			__FILE__, __LINE__, msg, esp_err_to_name(err));
	} else {
		fprintf(stderr, "Task Panic (%s:%d) | \"%s\"\n",
			__FILE__, __LINE__, msg);
	}
}
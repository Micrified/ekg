#include "telemetry_task.h"


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/



/*
 *******************************************************************************
 *                        Internal Function Definitions                        *
 *******************************************************************************
*/



/*
 *******************************************************************************
 *                        External Function Definitions                        *
 *******************************************************************************
*/


/* Automaton responsible for assembling a telemetry message and pushing
 * the data to the socket manager for dispatch over WiFi. Is triggered
 * by a software timer that sets the FLAG_TELEMETRY_SEND bit
*/
void task_telemetry_manager (void *args) {
    uint32_t flags;

    /* State Bit Field
     * 0x1: Telemetry is enabled
    */
    uint8_t state = 0x0;

    do {

        // Wait indefinitely on the following flags - don't auto-clear
        flags = xEventGroupWaitBits(g_event_group, 
            MASK_TELEMETRY_FLAGS,
            pdTRUE, pdFALSE, portMAX_DELAY);

        // Clear bits owned by this task
        xEventGroupClearBits(g_event_group, MASK_TELEMETRY_FLAGS);

        // If instructed to start telemetry
        if (flags & FLAG_TELEMETRY_START) {

            // Log event
            ESP_LOGI("TELE", "Telemetry start!");

            // Update local and global state
            state |= 0x1;
            SET_STATE_BIT_ATOMIC(MSG_TYPE_TELEMETRY_ENABLED_BIT);

            // Send a status update message
            dispatch_status_message("TELE");
        }

        // If instructed to stop telemetry
        if (flags & FLAG_TELEMETRY_STOP) {

            // Log event
            ESP_LOGI("TELE", "Telemetry stop!");

            // Update local and global state
            state &= ~0x1;
            CLEAR_STATE_BIT_ATOMIC(MSG_TYPE_TELEMETRY_ENABLED_BIT);

            // Send a status update message
            dispatch_status_message("TELE");
        }

        // If not a telemetry event, don't send
        if ((flags & FLAG_TELEMETRY_SEND) == 0) {
            continue;
        }

        // Do something here ...

    } while (1);

    // Destroy task
    vTaskDelete(NULL);
}

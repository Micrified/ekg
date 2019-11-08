#include "status.h"


/*
 *******************************************************************************
 *                        External Function Definitions                        *
 *******************************************************************************
*/


void dispatch_status_message (const char *task_tag) {
        static uint8_t msg_buffer[MSG_BUFFER_MAX];
        esp_err_t err;
        
        // Prepare message
        msg_t msg = MSG_STATUS(g_state_flag, g_wifi_lan_addr);

        // Pack message
        size_t z = msg_pack(&msg, msg_buffer);

        // Place message on outgoing queue
        if ((err = ipc_enqueue(g_ble_tx_queue, 0x0, z, msg_buffer)) 
            != ESP_OK) {
            ESP_LOGE(task_tag, "Couldn't enqueue message for BLE: %s", 
                E2S(err));
        }

        // Instruct BLE to send a message to device (if possible)
        xEventGroupSetBits(g_event_group, FLAG_BLE_SEND_MSG);
}

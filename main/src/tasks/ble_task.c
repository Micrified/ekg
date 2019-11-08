#include "ble_task.h"


/*
 *******************************************************************************
 *                        Internal Function Definitions                        *
 *******************************************************************************
*/


// Processes instructions received in a message
void instruction_handler (uint8_t instruction) {
    switch (instruction) {

        case INST_TELEMETRY_ENABLE: {
            xEventGroupSetBits(g_event_group, FLAG_TELEMETRY_START);
        }
        break;

        case INST_TELEMETRY_DISABLE: {
            xEventGroupSetBits(g_event_group, FLAG_TELEMETRY_STOP);
        }
        break;

        default:
            ESP_LOGE("BLE", "Unhandled instruction (%X)", instruction);
    }
}


// Processes messages received over BLE
void msg_handler (uint8_t *buffer, size_t size) {
    esp_err_t err;
    msg_t msg;

    // Unpack the message
    if ((err = msg_unpack(&msg, buffer, size)) != ESP_OK) {
        ESP_LOGE("BLE", "Couldn't unpack message: %s", E2S(err));
        return;
    }

    // Take action based on message type
    switch (msg.type) {

        // Message with Instruction
        case MSG_TYPE_INSTRUCTION: {

            // Extract instruction
            uint8_t inst = msg.body.msg_instruction.inst;

            // Log instruction
            ESP_LOGI("BLE", "Instruction: %s", inst_to_str(inst));

            // Take action on the message type
            instruction_handler(inst);
        }
        break;

        default: {
            ESP_LOGW("BLE", "Received unknown message type (%d)",msg.type);
        }
    }
}


/*
 *******************************************************************************
 *                            Function Definitions                             *
 *******************************************************************************
*/


void task_ble_manager (void *args) {
    uint32_t flags;
    esp_err_t err;
    task_queue_msg_t queue_msg;   // Holds messages taken from queues

    /* State Bit Flags 
     * 0x1: Bluetooth Low Energy is connected if set
    */
    uint8_t state = 0x0;

    do {

        // Wait indefinitely until any bit in the group is set
        flags = xEventGroupWaitBits(g_event_group, MASK_BLE_FLAGS, pdTRUE, 
            pdFALSE, portMAX_DELAY);

        // If the connected flag is set, then save state (bit auto-cleared)
        if (flags & FLAG_BLE_CONNECTED) {
        	state |= 0x1;
        }

        // If the disconnected flag is set, then save state (bit auto-cleared)
        if (flags & FLAG_BLE_DISCONNECTED) {
        	state &= ~0x1;

            // Clear the transmit queue (?)

        }

        // If received a message, check the type and perform action on it
        if (flags & FLAG_BLE_RECV_MSG) {

            // While there are messages to process
            while (uxQueueMessagesWaiting(g_ble_rx_queue) > 0) {

                // Dequeue the next message in BLE_RX_QUEUE
                if (xQueueReceive(g_ble_rx_queue, (void *)&queue_msg, 
                    TASK_QUEUE_MAX_TICKS) != pdPASS) {
                    ESP_LOGE("BLE", "Problem receiving data from BLE driver");
                    continue;
                }

                // Process the message
                msg_handler(queue_msg.data, queue_msg.size);
            }

        }

        // If a message is pending to be sent: Acquire and send message
        if (flags & FLAG_BLE_SEND_MSG) {

            // Only send if connected
            if (state & 0x1) {

                // While there are messages to process
                while (uxQueueMessagesWaiting(g_ble_tx_queue) > 0) {

                    // Read next message in BLE_TX_QUEUE for dispatch
                    if (xQueueReceive(g_ble_tx_queue, (void *)&queue_msg, 
                        TASK_QUEUE_MAX_TICKS) != pdPASS) {
                        ESP_LOGE("BLE", 
                            "Send flag set but no message on queue!");
                        continue;
                    }

                    ESP_LOGI("BLE", "Sending a message of %d bytes!", queue_msg.size);

                    if ((err = ble_send(queue_msg.size, queue_msg.data)) 
                        != ESP_OK) {
                        ESP_LOGE("BLE", 
                            "Couldn't send message: %s", E2S(err));
                    }                   
                }


            }
        }

    } while (1);


    // Destroy task
    vTaskDelete(NULL);
}
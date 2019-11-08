#include "wifi_task.h"


/*
 *******************************************************************************
 *                        Internal Function Definitions                        *
 *******************************************************************************
*/


// Initiates a connection to an access point 
esp_err_t utility_wifi_connect (const uint8_t *ssid, const uint8_t *pswd, 
    unsigned int retc) {
    esp_err_t err;

    // Attempt to configure credentials
    if ((err = wifi_config((const char *)ssid, (const char *)pswd, retc))
     != ESP_OK) {
     	ESP_LOGE("WIFI", "Couldn't configure WiFi credentials: %s", E2S(err));
        return err;
    }

    // Start the driver
    if ((err = wifi_start_driver()) != ESP_OK) {
        ESP_LOGE("WIFI", "Couldn't start WiFi driver: %s", E2S(err));
        return err;
    }

    return ESP_OK;
}


/*
 *******************************************************************************
 *                            Function Definitions                             *
 *******************************************************************************
*/


void task_wifi_manager (void *args) {
    uint32_t flags;
    esp_err_t err;

    /* State Bit Flags 
     * 0x1: WiFi connected if set
     * 0x2: Reconnect on disconnect mode is set
    */
    uint8_t state = 0x0;

    do {

        // Wait indefinitely until any bit in the group is set
        flags = xEventGroupWaitBits(g_event_group, MASK_WIFI_FLAGS, pdTRUE,
            pdFALSE, portMAX_DELAY);

        // If the connected flag is set, then save state (bit auto-cleared)
        if (flags & FLAG_WIFI_CONNECTED) {

            // Update local and global state flags
        	state |= 0x1;
            SET_STATE_BIT_ATOMIC(MSG_TYPE_WIFI_CONNECTED_BIT);

            // Update lan address
            g_wifi_lan_addr = get_wifi_lan_addr();

            // Dispatch status message
            dispatch_status_message("WIFI");
        }

        // If the disconnected flag is set, then save state (bit auto-cleared)
        if (flags & FLAG_WIFI_DISCONNECTED) {

            // Update local and global state flags
        	state &= ~0x1;
            CLEAR_STATE_BIT_ATOMIC(MSG_TYPE_WIFI_CONNECTED_BIT);

        	// Stop the WiFi driver
        	if ((err = wifi_stop_driver()) != ESP_OK) {
        		ESP_LOGE("WIFI", "Couldn't stop WiFi driver: %s", E2S(err));
        	}

        	// If an intentional disconnect - clear flag and set WIFI_START
        	if (state & 0x2) {
        		state &= ~0x2;
        		xEventGroupSetBits(g_event_group, FLAG_WIFI_START);
        		continue;
        	}

            // Dispatch status message
            dispatch_status_message("WIFI");
        }

        // If instructed to start WiFi, then attempt to connect
        if (flags & FLAG_WIFI_START) {

            // If already connected, then disconnect from the current AP
            if (state & 0x1) {

            	// Attempt to disconnect
            	if ((err = wifi_disconnect()) != ESP_OK) {
                  	ESP_LOGE("WIFI", "Couldn't disconnect WiFi: %s", E2S(err));     		
                } else {

                	// Set to enable reconnect on next disconnect event
                	state |= 0x2;
                }

                continue;
            }

            // To be safe, global credentials have null-char re-applied
            g_wifi_ssid_buffer[32] = g_wifi_pswd_buffer[64] = '\0';

            // Create the retry-count
            unsigned int retc = 3;

            // Display credentials for debug
            ESP_LOGW("WIFI", "SSID: \"%s\" | PSWD: \"%s\"", g_wifi_ssid_buffer,
            	g_wifi_pswd_buffer);

            // Invoke the manager (it will set the ready flag)
            if ((err = utility_wifi_connect(g_wifi_ssid_buffer, 
            	g_wifi_pswd_buffer, retc)) != ESP_OK) {
                ESP_LOGE("WIFI", "Couldn't connect to WiFi: %s", E2S(err));
                continue;
            }
        }

        // If instructed to stop WiFi, then check if connected and disconnect
        if (flags & FLAG_WIFI_STOP) {

            // If connected, then disconnect from the current AP
            if (state & 0x1) {

                // Attempt to disconnect
                if ((err = wifi_disconnect()) != ESP_OK) {
                    ESP_LOGE("WIFI", "Couldn't disconnect WiFi: %s", E2S(err));
                }

                // The disconnect event will send the status message for us
                continue;
            }

            // Otherwise since already disconnected, we just send a status update
            dispatch_status_message("WIFI");
        }

    } while (1);

    // Destroy task
    vTaskDelete(NULL);
}
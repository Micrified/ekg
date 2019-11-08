#include "telemetry_task.h"


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Internal socket (init as invalid)
static int g_sock = -1;


/*
 *******************************************************************************
 *                        Internal Function Definitions                        *
 *******************************************************************************
*/


// Returns zero on success
int init_telemetry () {

    // Configure socket connection information for (2)
    const struct sockaddr_in sock_descr = {
        .sin_addr.s_addr = htonl(g_telemetry_inet_addr),
        .sin_family = AF_INET,
        .sin_port = htons(g_telemetry_inet_port),
    };

    // (1) Initialize socket
    if ((g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0) {
        ESP_LOGE("TELE", "Could not initialize socket!");
        return -1;
    }

    // (2) Connect socket
    if (connect(g_sock, (struct sockaddr *)&sock_descr, sizeof(sock_descr))
     != 0) {
        ESP_LOGE("TELE", "Could not connect socket!");
        return -1;
    }

    return 0;
}


// Closes and resets the socket
void deinit_telemetry () {
    if (g_sock > 0) {
        close(g_sock);
        g_sock = -1;
    }
}


// Returns: (-1) on error; (0) on socket closure; (>0) on recv (#bytes = value)
int16_t send_telemetry () {
    uint8_t arg_buffer[16];
    static int count = 0;
    size_t sent, size; ssize_t written;

    // Prepare count argument for post request
    snprintf((char *)arg_buffer, 16, "count=%u", ++count);

    // Get pointer to post request buffer
    const uint8_t *data = http_post((const char *)arg_buffer, &size);

    // Write the POST request to the socket
    for (sent = 0; sent < size; sent += written) {
        if ((written = send(g_sock, data + sent, size - sent, 0x0)) < 0) {
            ESP_LOGE("TELE", "Socket error (SEND)!");
            return -1;
        }
        if (written == 0) {
            ESP_LOGW("TELE", "Host socket closure (SEND)!");
            return -1;
        }
    }

    // Return success!
    return 0;
}


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
     * 0x1: WiFi is enabled
     * 0x2: Telemetry is enabled
    */
    uint8_t state = 0x0;

    do {

        // Wait indefinitely on the following flags - don't auto-clear
        flags = xEventGroupWaitBits(g_event_group, 
            MASK_TELEMETRY_FLAGS | FLAG_WIFI_CONNECTED | FLAG_WIFI_DISCONNECTED,
            pdTRUE, pdFALSE, portMAX_DELAY);

        // Clear bits owned by this task
        xEventGroupClearBits(g_event_group, MASK_TELEMETRY_FLAGS);

        // If WiFi is enabled
        if (flags & FLAG_WIFI_CONNECTED) {
            state |= 0x1;
        }

        // If WiFi is disabled
        if (flags & FLAG_WIFI_DISCONNECTED) {
            state &= ~0x1;
        }

        // If instructed to start telemetry
        if (flags & FLAG_TELEMETRY_START) {

            // Log event
            ESP_LOGI("TELE", "Telemetry start!");

            // Update local and global state
            state |= 0x2;
            SET_STATE_BIT_ATOMIC(MSG_TYPE_TELEMETRY_ENABLED_BIT);

            // Send a status update message
            dispatch_status_message("TELE");
        }

        // If instructed to stop telemetry
        if (flags & FLAG_TELEMETRY_STOP) {

            // Log event
            ESP_LOGI("TELE", "Telemetry stop!");

            // Update local and global state
            state &= ~0x2;
            CLEAR_STATE_BIT_ATOMIC(MSG_TYPE_TELEMETRY_ENABLED_BIT);

            // Send a status update message
            dispatch_status_message("TELE");
        }

        // If not a telemetry event, don't send
        if ((flags & FLAG_TELEMETRY_SEND) == 0) {
            continue;
        }

        // If no WiFi or streaming stopped, don't send
        if ((state & 0x1) == 0 || (state & 0x2) == 0) {
            continue;
        }

        // Try to open a socket and sent a POST request
        if (init_telemetry() == 0 && send_telemetry() > 0) {
            ESP_LOGE("TELE", "Telemetry sent!");
        }

        // Always deinit after sending. Telemetry shouldn't maintain connection
        deinit_telemetry();

    } while (1);

    // Destroy task
    vTaskDelete(NULL);
}

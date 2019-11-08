#include "stream_task.h"


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Internal socket (init as invalid)
static int g_sock = -1;


// Holds number of bytes received for rate tracking
uint32_t g_bytes_received;


// Holds data on receive
static uint8_t g_recv_buffer[SOCK_MAX_RECV_SIZE];


/*
 *******************************************************************************
 *                        Internal Function Definitions                        *
 *******************************************************************************
*/


// Returns zero on success
int init_stream () {
	size_t sent, size = 0; ssize_t written = 0;

	// Configure GET request data for (3)
	const uint8_t *data = http_get((const char *)g_stream_url_path, &size);

	// Configure connection information for (2)
	const struct sockaddr_in sock_descr = {
		.sin_addr.s_addr = htonl(g_stream_inet_addr),
		.sin_family = AF_INET,
		.sin_port = htons(g_stream_inet_port),
	};

	// (1) Initialize socket
	if ((g_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0) {
		ESP_LOGE("STREAM", "Could not initialize socket!");
		return -1;
	}

	// (2) Connect socket
	if (connect(g_sock, (struct sockaddr *)&sock_descr, sizeof(sock_descr))
		!= 0) {
		ESP_LOGE("STREAM", "Could not connect socket!");
		return -1;
	}

	// (3) Write GET request to socket
	for (sent = 0; sent < size; sent += written) {
		if ((written = send(g_sock, data + sent, size - sent, 0x0)) < 0) {
			ESP_LOGE("STREAM", "Socket error (SEND)!");
			return -1;
		}
		if (written == 0) {
			ESP_LOGW("STREAM", "Host socket closure (SEND)!");
			return -1;
		}
	}

	// Return success!
	return 0;
}


// Closes and resets the socket 
void deinit_stream () {

	if (g_sock > 0) {
		close(g_sock);
		g_sock = -1;
	}

}


// Returns: (-1) on error; (0) on socket closure; (>0) on recv (#bytes = value)
int16_t recv_stream () {
	int16_t n;

	// On error
	if ((n = recv(g_sock, g_recv_buffer, SOCK_MAX_RECV_SIZE, 0x0)) < 0) {
		ESP_LOGE("STREAM", "Socket error (RECV)!");
		return -1;
	}

	// On socket closure
	if (n == 0) {
		ESP_LOGW("STREAM", "Host socket closure (RECV)!");
		return -1;
	}

	return n;
}


/*
 *******************************************************************************
 *                        External Function Definitions                        *
 *******************************************************************************
*/


void task_stream_manager (void *args) {
	uint32_t flags; int16_t n;
	const TickType_t event_block_time = 8; 
	/* State bits
	 * 0x1: Stream enabled
	 * 0x2: WiFi enabled
	*/
	uint8_t state = 0x0;

	do {

		// Wait for limited time on following flags: NO AUTO-CLEAR
		flags = xEventGroupWaitBits(g_event_group,
			FLAG_WIFI_CONNECTED | FLAG_WIFI_DISCONNECTED | MASK_STREAM_FLAGS,
			pdFALSE, pdFALSE, event_block_time);

		// Clear flags owned by this task
		xEventGroupClearBits(g_event_group, MASK_STREAM_FLAGS);

		// On WiFi connected
		if (flags & FLAG_WIFI_CONNECTED) {
			state |= 0x2;

			if (state & 0x1) {
				ESP_LOGI("STREAM", "Auto-resuming stream ...");
				init_stream();
			}
		}

		// On WiFi disconnected
		if (flags & FLAG_WIFI_DISCONNECTED) {
			state &= ~0x2;

			if (state & 0x1) {
				ESP_LOGI("STREAM", "Auto-halting stream ...");
				deinit_stream();
			}
		}

		// On Stream enabled
		if (flags & FLAG_STREAM_START) {
			if ((state & 0x1) == 0) {
				ESP_LOGI("STREAM", "Streaming enabled");

				// Init stream if WiFi available
				if (state & 0x2) {
					init_stream();
				}

				// Update local and global state
				state |= 0x1;
				SET_STATE_BIT_ATOMIC(MSG_TYPE_STREAM_ENABLED_BIT);
			}

			// Send status message
			dispatch_status_message("STREAM");
		}

		// On Stream disabled
		if (flags & FLAG_STREAM_STOP) {
			if ((state & 0x1) != 0) {
				ESP_LOGI("STREAM", "Streaming disabled");

				// Deinit stream
				deinit_stream();

				// Update local and global state
				state &= ~0x1;
				CLEAR_STATE_BIT_ATOMIC(MSG_TYPE_STREAM_ENABLED_BIT);
			}

			// Send status message
			dispatch_status_message("STREAM");
		}

		// On Stream tick
		if (flags & FLAG_STREAM_TICK) {
			if (state & 0x1) {
				ESP_LOGI("STREAM", "%u bps", g_bytes_received);
				g_bytes_received = 0;
			}
		}

		// If not streaming or no WiFi, return to wait on flags
		if (state != 0x3) {
			continue;
		}

		// Else get next data fragment; on error try restart
		if ((n = recv_stream()) <= 0) {
			deinit_stream();
			init_stream();
		} else {
			g_bytes_received += n;
		}

	} while (1);

}
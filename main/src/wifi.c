#include "wifi.h"

/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 18/08/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  Wrappers for WiFi functionality                                            *
 *                                                                             *
 *******************************************************************************
*/


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// The network configuration currently in use
wifi_config_t g_wifi_cfg;


// Tracks the WiFi state. Defaults to uninitialized
static wifi_state_t g_wifi_state = WIFI_STATE_UNINIT;


// Country details for the WiFi
static wifi_country_t g_country = (wifi_country_t){
	.cc = "NLI",
	.schan = 1,
	.nchan = 13,
	.policy = WIFI_COUNTRY_POLICY_AUTO
};


// The retry count for connecting to an AP
unsigned int g_retry_count;


// The remaining retry count
static unsigned int remaining_retry_count;


// The TCP/IP adapter hostname
char g_hostname[32];


// The IPv4 LAN address assigned to the device (network byte order)
static uint32_t g_lan_addr;


/*
 *******************************************************************************
 *                         Internal Symbolic Constants                         *
 *******************************************************************************
*/


// Handler: WIFI_EVENT_STA_START : WiFi driving is running and in station mode
#define HANDLER_STATION_STARTED(handler_arg, event_data) {                  \
                                                                            \
    /* Set TCP/IP interface hostname */                                     \
    if (tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, g_hostname)        \
        != ESP_OK) {                                                        \
        ESP_LOGE("WiFi-Driver", "Couldn't set hostname!");                  \
    }                                                                       \
                                                                            \
    /* Update global state to WIFI_STATE_DISCONNECTED */                    \
    g_wifi_state = WIFI_STATE_DISCONNECTED;                                 \
                                                                            \
    /* Initialize LwIP to connect to configured network */                  \
    esp_wifi_connect();                                                     \
}


// Handler: WIFI_EVENT_STA_DISCONNECTED : Intentional/unintentional disconnect
#define HANDLER_STATION_DISCONNECTED(handler_arg, event_data) {             \
                                                                            \
    /* If already connected, then toggle event and return */                \
    if (g_wifi_state == WIFI_STATE_CONNECTED) {                             \
        xEventGroupSetBits(g_event_group, FLAG_WIFI_DISCONNECTED);          \
    }                                                                       \
                                                                            \
    /* If not connnected, then unintentional failure: Retry */              \
    if (g_wifi_state == WIFI_STATE_DISCONNECTED) {                          \
        if (remaining_retry_count > 0) {                                    \
            remaining_retry_count--;                                        \
            ESP_LOGW("WiFi-Driver", "Trying to connect again ...");         \
            esp_wifi_connect();                                             \
        } else {                                                            \
            ESP_LOGW("WiFi-Driver", "Failed to connect ....");              \
            xEventGroupSetBits(g_event_group, FLAG_WIFI_DISCONNECTED);      \
        }                                                                   \
    }                                                                       \
                                                                            \
                                                                            \
    /* Mark state as not connected */                                       \
    g_wifi_state = WIFI_STATE_DISCONNECTED;                                 \
    ESP_LOGI("WiFi-Driver", "Disconnected");                                \
}


// Handler: WIFI_EVENT_STA_STOP : IP released, DHCP stopped, TCP/IP closed
#define HANDLER_STATION_STOPPED(handler_arg, event_data) {                  \
                                                                            \
    /* Update global state to WIFI_STATE_CONFIG */                          \
    g_wifi_state = WIFI_STATE_CONFIG;                                       \
}


// Handler: IP_EVENT_STA_GOT_IP : Station assigned IP address by DHCP
#define HANDLER_STATION_GOT_IP(handler_arg, event_data) {                   \
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;             \
                                                                            \
    /* Save IP address */                                                   \
    g_lan_addr = (uint32_t)((event->ip_info).ip.addr);                      \
                                                                            \
    /* Update global state to connected */                                  \
    g_wifi_state = WIFI_STATE_CONNECTED;                                    \
                                                                            \
    /* Extract and print IP, signal connected bit to group */               \
    xEventGroupSetBits(g_event_group, FLAG_WIFI_CONNECTED);                 \
    ESP_LOGI("WiFi-Driver", "IP: %s", ip4addr_ntoa(&event->ip_info.ip));    \
}


/*
 *******************************************************************************
 *                        External Function Definitions                        *
 *******************************************************************************
*/


/*
 * @brief Launches TCP-IP handler task, and WiFi driver task
 *
 * @param
 * - hostname: A string defining the TCP/IP adapter hostname (32-bytes max)
 *
 * @return
 * - ESP_OK: Success
 * - ESP_FAIL: This function was already called
 * - ESP_ERR_NO_MEM: Insufficient resources to initialize WiFi driver task
 * - ESP_ERR_NO_MEM: Out of memory
 * - ESP_ERR_TCPIP_ADAPTER_IF_NOT_READY: interface status error
 * - ESP_ERR_TCPIP_ADAPTER_INVALID_PARAMS: bad hostname
*/
esp_err_t wifi_init (const char *hostname) {
	esp_err_t err;

	// Only init if global state is set to not-yet-initialized
	if (g_wifi_state != WIFI_STATE_UNINIT) {
		return ESP_FAIL;
	}

	// Initialize the TCP/IP adapter (launches handler task)
	tcpip_adapter_init();

	// Clone the default configuration (recommended to always do this)
	wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();

	// Starts WiFi driver task and allocates resources for it
	if ((err = esp_wifi_init(&init_cfg)) != ESP_OK) {
		return err;
	}

	// Save the hostname (max 32 bytes)
	strncpy(g_hostname, hostname, 32);

	// Change the global state to initialized
	g_wifi_state = WIFI_STATE_INIT;

	return ESP_OK;
}


/*
 * @brief Configures the WiFi driver to behave as a station and
 *		  prepares it to connect to a network with the given details
 *        Sets the country-code and other metadata
 * @param
 * - ssid : The human-readable name of the network
 * - pswd : The password
 * - retc : The retry-count
 *
 * @return
 * - ESP_OK: Success
 * - ESP_ERR_WIFI_CONN: WiFi internal error: station or soft-AP control block
 * - ESP_ERR_WIFI_NOT_INIT: wifi_init was not yet called
 * - ESP_FAIL: Current state doesn't allow configuration. Other errors possible
*/
esp_err_t wifi_config (const char *ssid, const char *pswd, unsigned int retc) {
	esp_err_t err;

	// Only update the config if global state is initialized or configured
	if (!(g_wifi_state == WIFI_STATE_INIT || 
		  g_wifi_state == WIFI_STATE_CONFIG)) {
		return ESP_FAIL;
	}

	// Update network configuration details
	strncpy((char *)g_wifi_cfg.sta.ssid, ssid, 32);
	strncpy((char *)g_wifi_cfg.sta.password, pswd, 64);
	g_retry_count = retc;

	// Set the WiFi driver mode (either acting as an AP or station)
	if ((err = esp_wifi_set_mode(WIFI_MODE_STA)) != ESP_OK) {
		return err;
	}

	// Apply the network configuration
	if ((err = esp_wifi_set_config(ESP_IF_WIFI_STA, &g_wifi_cfg)) != ESP_OK) {
		return err;
	}

	// Configure country as The Netherlands (NL), indoors: (I)
	if ((err = esp_wifi_set_country(&g_country)) != ESP_OK) {
		return err;
	}

	// Update the global state to configured
	g_wifi_state = WIFI_STATE_CONFIG;

	return ESP_OK;
}


/*
 * @brief Starts the WiFi driver
 * @param
 *
 * @return 
 * - ESP_OK: Success
 * - ESP_FAIL: Global state is not WIFI_STATE_CONFIG which it must be
 * - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
*/
esp_err_t wifi_start_driver (void) {

	// Global state must be configured or disconnected (already configured)
	if (!(g_wifi_state == WIFI_STATE_CONFIG ||
		  g_wifi_state == WIFI_STATE_DISCONNECTED)) {
		return ESP_FAIL;
	}

	// Set/Reset the retry-count
	remaining_retry_count = g_retry_count;

	return esp_wifi_start();
}


/*
 * @brief Disconnects the WiFi driver from the current AP
 * @param
 * @return
 * - ESP_OK: Success
 * - ESP_FAIL: Global state is not in WIFI_STATE_CONNECTED
*/
esp_err_t wifi_disconnect (void) {
	if (g_wifi_state != WIFI_STATE_CONNECTED) {
		return ESP_FAIL;
	}
	return esp_wifi_disconnect(); // state changes in handler
}


/*
 * @brief Stops the WiFi driver task
 * @param
 * @return
 * - ESP_OK: Success
 * - ESP_FAIL: Global state is not in WIFI_STATE_DISCONNECTED
*/
esp_err_t wifi_stop_driver (void) {
	if (g_wifi_state != WIFI_STATE_DISCONNECTED) {
		return ESP_FAIL;
	}
	return esp_wifi_stop(); // state changes in handler
}


/*
 * @brief Unloads the WiFi driver from memory and frees it
 * @param
 * @return
 * - ESP_OK: Success
 * - ESP_FAIL: Global state is not in WIFI_STATE_CONFIG or 
 *   WIFI_STATE_INIT
*/
esp_err_t wifi_unload (void) {
	esp_err_t err;
	if (g_wifi_state != WIFI_STATE_INIT && g_wifi_state != WIFI_STATE_CONFIG) {
		return ESP_FAIL;
	}
	if ((err = esp_wifi_deinit()) != ESP_OK) {
		return err;
	}

	// Change state
	g_wifi_state = WIFI_STATE_UNINIT;

	return ESP_OK;
}


/*
 * @brief A WiFi handler to be registered with an event loop. 
 * @param
 * - handler_arg: Pointer to handler arguments 
 * - base: Identifies an independent group of events (e.g: WiFi)
 * - id: Identifies a specific event in the groupp (e.g: Got IP)
 * - event_data: Pointer to event data (optional)
 *
 * @return
 *
*/
void wifi_event_handler (void *handler_arg, esp_event_base_t base, int32_t id,
	void *event_data) {

	// WiFi-based events
	if (base == WIFI_EVENT) {
		switch (id) {
			case WIFI_EVENT_STA_START:
				HANDLER_STATION_STARTED(handler_arg, event_data);
				break;

			case WIFI_EVENT_STA_DISCONNECTED:
				HANDLER_STATION_DISCONNECTED(handler_arg, event_data);
				break;

			case WIFI_EVENT_STA_STOP:
				HANDLER_STATION_STOPPED(handler_arg, event_data);
				break;

			default: break;
		}
		return;
	}

	// IP-based events
	if (base == IP_EVENT) {
		switch (id) {
			case IP_EVENT_STA_GOT_IP:
				HANDLER_STATION_GOT_IP(handler_arg, event_data);
				break;

			default: break;
		}
		return;
	}
}


/*
 * @brief Returns a pointer to the current network configuration
 * @return Pointer to the current network configuration. If
 *         none has been set, NULL is returned
*/
const wifi_config_t *get_wifi_cfg (void) {
	if (g_wifi_state < WIFI_STATE_CONFIG) {
		return NULL;
	}
	return &g_wifi_cfg;
}


/*
 * @brief Returns the current WiFi state
 * @return Enumeral representing the internal WiFi state from
 *         the WiFi state-machine
*/
wifi_state_t get_wifi_state (void) {
	return g_wifi_state;
}


/*
 * @brief Returns the IPv4 WiFi LAN address assigned to the device
 * @note Only valid when in connected state (else null)
 * @return 32-bit IPv4 LAN address in network byte order
*/
uint32_t get_wifi_lan_addr (void) {
    if (g_wifi_state == WIFI_STATE_CONNECTED) {
        return g_lan_addr;
    } else {
        return 0x0;
    }
}
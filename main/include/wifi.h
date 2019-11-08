#if !defined(WIFI_H)
#define WIFI_H

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

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "tasks.h"


/*
 *******************************************************************************
 *                              Type Definitions                               *
 *******************************************************************************
*/


// Enumeration describing the possible states of the WiFi connection
typedef enum {
	WIFI_STATE_MIN = 0,			// (Invalid State Boundary)
	WIFI_STATE_UNINIT,			// No WiFi driver task or TCP/IP task exists
	WIFI_STATE_INIT,			// The WiFi driver and TCP/IP tasks are primed
	WIFI_STATE_CONFIG,			// The WiFi driver is pre-configured
	WIFI_STATE_DISCONNECTED,	// The WiFi driver is started but not connected
	WIFI_STATE_CONNECTED,		// The WiFi driver is started and connected
	WIFI_STATE_MAX				// (Invalid State Boundary)
} wifi_state_t;


/*
 *******************************************************************************
 *                            Function Declarations                            *
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
esp_err_t wifi_init (const char *hostname);


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
esp_err_t wifi_config (const char *ssid, const char *pswd, unsigned int retc);


/*
 * @brief Starts the WiFi driver
 * @param
 *
 * @return 
 * - ESP_OK: Success
 * - ESP_FAIL: Global state is not WIFI_STATE_CONFIG which it must be
 * - ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
*/
esp_err_t wifi_start_driver (void);


/*
 * @brief Disconnects the WiFi driver from the current AP
 * @param
 * @return
 * - ESP_OK: Success
 * - ESP_FAIL: Global state is not in WIFI_STATE_CONNECTED
*/
esp_err_t wifi_disconnect (void);


/*
 * @brief Stops the WiFi driver task
 * @param
 * @return
 * - ESP_OK: Success
 * - ESP_FAIL: Global state is not in WIFI_STATE_DISCONNECTED
*/
esp_err_t wifi_stop_driver (void);


/*
 * @brief Unloads the WiFi driver from memory and frees it
 * @param
 * @return
 * - ESP_OK: Success
 * - ESP_FAIL: Global state is not in WIFI_STATE_CONFIG or 
 *   WIFI_STATE_INIT
*/
esp_err_t wifi_unload (void);


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
	void *event_data);


/*
 * @brief Returns a pointer to the current network configuration
 * @return Pointer to the current network configuration. If
 *         none has been set, NULL is returned
*/
const wifi_config_t *get_wifi_cfg (void);


/*
 * @brief Returns the current WiFi state
 * @return Enumeral representing the internal WiFi state from
 *         the WiFi state-machine
*/
wifi_state_t get_wifi_state (void);


/*
 * @brief Returns the IPv4 WiFi LAN address assigned to the device
 * @note Only valid when in connected state (else null)
 * @return 32-bit IPv4 LAN address in network byte order
*/
uint32_t get_wifi_lan_addr (void);


#endif

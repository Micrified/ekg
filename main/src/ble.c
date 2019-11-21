#include "ble.h"


/*
 *******************************************************************************
 *                         Internal Symbolic Constants                         *
 *******************************************************************************
*/


// Masks the config status in the advertising-status bit-field
#define g_adv_config_status_config_flag     (1 << 0)


// Masks the scan response status in the advertising-status bit-field
#define g_adv_config_status_scan_response   (1 << 1)


/*
 *******************************************************************************
 *                        Internal Forward Declarations                        *
 *******************************************************************************
*/


// GAP Event Handler
static void gap_event_handler (esp_gap_ble_cb_event_t, 
    esp_ble_gap_cb_param_t *);


// GATTs Event Handler
static void gatts_event_handler (esp_gatts_cb_event_t, esp_gatt_if_t,
    esp_ble_gatts_cb_param_t *);


/*****************************************************************************/


// Profile event-handler for WiFi
static void gatts_profile_event_handler_wifi (esp_gatts_cb_event_t,
    esp_gatt_if_t, esp_ble_gatts_cb_param_t *);


/*****************************************************************************/


/*
 *******************************************************************************
 *                          Internal Global Variables                          *
 *******************************************************************************
*/


// Internal table mapping esp_gatts_cb_event_t enumeral values to their names
static const char *g_gatts_event_str[] = {
	"ESP_GATTS_REG_EVT",                   // When app is registered okay
	"ESP_GATTS_READ_EVT",                  // When client requests to read
	"ESP_GATTS_WRITE_EVT",                 // When a write operation is received
	"ESP_GATTS_EXEC_WRITE_EVT",            // When a long-write is finished
	"ESP_GATTS_MTU_EVT",                   // When the MTU is successfully set
	"ESP_GATTS_CONF_EVT",                  // When a confirm is received
	"ESP_GATTS_UNREG_EVT",                 // When app ID unregistration is done
	"ESP_GATTS_CREATE_EVT",                // When service creation is done
	"ESP_GATTS_ADD_INCL_SRVC_EVT",         // When add included service is done
	"ESP_GATTS_ADD_CHAR_EVT",              // When characteristic added okay
	"ESP_GATTS_ADD_CHAR_DESCR_EVT",        // When char descriptor added okay
	"ESP_GATTS_DELETE_EVT",                // When a service delete is complete
	"ESP_GATTS_START_EVT",                 // When a service start is complete
	"ESP_GATTS_STOP_EVT",                  // When a service stop is complete
	"ESP_GATTS_CONNECT_EVT",               // When a GATT client connects
	"ESP_GATTS_DISCONNECT_EVT",            // When a GATT client disconnects
	"ESP_GATTS_OPEN_EVT",                  // When connected to a peer
	"ESP_GATTS_CANCEL_OPEN_EVT",           // When disconnected from peer
	"ESP_GATTS_CLOSE_EVT",                 // When the GATT server close done
	"ESP_GATTS_LISTEN_EVT",                // When successfully set to listening
	"ESP_GATTS_CONGEST_EVT",               // When a congest happens (?)
	"ESP_GATTS_RESPONSE_EVT",              // When response sent successfully
	"ESP_GATTS_CREAT_ATTR_TAB_EVT",        // When attribute table created
	"ESP_GATTS_SET_ATTR_VAL_EVT",          // On setting of attr value
	"ESP_GATTS_SEND_SERVICE_CHANGE_EVT"    // On GATT send-service-change
};


// Internal Bit-field marking progression of advertising configuration setup
static uint8_t g_adv_config_status;


// The service UUID that is used the GAP advertising data and scan response
static uint8_t g_service_uuid[32] = {
    /* LSB <------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 
    0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
    //second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 
    0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};


// Universal advertising data structure for GAP
static esp_ble_adv_data_t g_advertising_data = {
    .set_scan_rsp           = false,        // Is a scan response
    .include_name           = true,         // Show device name
    .include_txpower        = true,         // Show transmission power
    .min_interval           = 6,            // Ideal min slave connect interval
    .max_interval           = 16,           // Ideal max slave connect interval
    .appearance             = 0,            // External appearance 
    .manufacturer_len       = 0,            // Manufacturer data length
    .p_manufacturer_data    = NULL,         // Manufacturer data
    .service_data_len       = 0,            // Service data length
    .p_service_data         = NULL,         // Service data pointer
    .service_uuid_len       = sizeof(g_service_uuid), // Service UUID length
    .p_service_uuid         = g_service_uuid,
                                            // Service UUID array pointer
    .flag                   = (ESP_BLE_ADV_FLAG_GEN_DISC | 
                               ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
                                            // Advert discovery mode flags
};


// Universal advertising scan-response structure for GAP
static esp_ble_adv_data_t g_scan_response_data = {
    .set_scan_rsp           = true,         // Is a scan response
    .include_name           = true,         // Show device name
    .include_txpower        = true,         // Show transmission power
    .min_interval           = 6,            // Ideal min slave connect interval
    .max_interval           = 16,           // Ideal max slave connect interval
    .appearance             = 0,            // External appearance 
    .manufacturer_len       = 0,            // Manufacturer data length
    .p_manufacturer_data    = NULL,         // Manufacturer data
    .service_data_len       = 0,            // Service data length
    .p_service_data         = NULL,         // Service data pointer
    .service_uuid_len       = sizeof(g_service_uuid),           // Service UUID length
    .p_service_uuid         = g_service_uuid,
                                            // Service UUID array pointer
    .flag                   = (ESP_BLE_ADV_FLAG_GEN_DISC | 
                               ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
                                            // Advert discovery mode flags
};


// Advertising parameters used in esp_ble_gap_start_advertising (for GAP only)
static esp_ble_adv_params_t g_adv_parameters = {
	.adv_int_min           = 32,                       // Min advert interval
	.adv_int_max           = 64,                       // Max advert interval
	.adv_type              = ADV_TYPE_IND,             // Advertising type
	.own_addr_type         = BLE_ADDR_TYPE_PUBLIC,     // Owner BT addr type
	.channel_map           = ADV_CHNL_ALL,             // Advert channel map
	.adv_filter_policy     = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, // Filter policy
};


// Table of Application Profiles (Add your limited application profile here)
static struct gatts_profile_t g_profile_table[APP_PROFILE_COUNT] = {
    [APP_PROFILE_WIFI] = {
        .gatts_cb = gatts_profile_event_handler_wifi,
        .gatts_if = ESP_GATT_IF_NONE,
    },
};


/*
 *******************************************************************************
 *                  Profile-Specific Characteristic Variables                  *
 *******************************************************************************
*/


// String value WiFi (contains dummy data)
static uint8_t char_str_wifi[] = {0x11,0x22,0x33};


// A property bit-field for the SSID characteristic
esp_gatt_char_prop_t wifi_property;


// A permissions bit-field for the SSID characteristic
esp_gatt_perm_t wifi_permissions;


// The characteristic value for the SSID
static esp_attr_value_t gatts_wifi_char_ssid_value = (esp_attr_value_t) {
    .attr_max_len = GATTS_CHARACTERISTIC_VALUE_LENGTH_MAX,
    .attr_len     = sizeof(char_str_wifi),
    .attr_value   = char_str_wifi
};


// Local variable describing the WiFi SSID buffer
prepare_type_env_t wifi_message_buffer;


/*
 *******************************************************************************
 *                        External Function Definitions                        *
 *******************************************************************************
*/


esp_err_t ble_init (void) {
	esp_err_t err = ESP_OK;

    // Create Bluetooth controller configuration struct with default settings
    esp_bt_controller_config_t btc_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    // Release memory used for classic BT
    if ((err = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT))
     != ESP_OK) {
     	ESP_LOGE("BLE-Driver", "Initialization failed: %s", E2S(err));
     	goto end;
    }

    // Initialize the controller (cfg has stack-size, priority, and baud rate)
    if ((err = esp_bt_controller_init(&btc_cfg)) != ESP_OK) {
    	ESP_LOGE("BLE-Driver", "Initialization failed: %s", E2S(err));
    	goto end;
    }

    // Enable controller in BLE mode (dual is ESP_BT_MODE_BTDM)
    if ((err = esp_bt_controller_enable(ESP_BT_MODE_BLE)) != ESP_OK) {
        ESP_LOGE("BLE-Driver", "Enable BLE mode failed: %s", E2S(err));
        goto end;
    }

    // Initialize the Bluedroid stack (not same as controller)
    if ((err = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE("BLE-Driver", "Bluedroid stack init failed: %s", E2S(err));
        goto end;
    }

    // Enable the Bluedroid stack
    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE("BLE-Driver", "Bluedroid stack enable failed: %s", E2S(err));
        goto end;
    }

    // Register the GATTS event handler
    if ((err = esp_ble_gatts_register_callback(gatts_event_handler)) != ESP_OK) {
        ESP_LOGE("BLE-Driver", "Register GATTS event handler failed: %s", 
        	E2S(err));
        goto end;
    }

    // Register the GAP event handler
    if ((err = esp_ble_gap_register_callback(gap_event_handler)) != ESP_OK) {
        ESP_LOGE("BLE-Driver", "Register GAP event handler failed: %s", 
        	E2S(err));
        goto end;
    }

    /*************************************************************************/

    // Register Application Profile: WiFi
    if ((err = esp_ble_gatts_app_register(APP_PROFILE_WIFI)) != ESP_OK) {
    	ESP_LOGE("BLE-Driver", "Register WiFi Application Profile failed: %s", 
    		E2S(err));
        goto end;
    }

    // Register other profiles here ...

    /*************************************************************************/

    // Configure Message Transmission Unit size (once per connection)
    if ((err = esp_ble_gatt_set_local_mtu(BLE_MTU_SIZE)) != ESP_OK) {
    	ESP_LOGE("BLE-Driver", "Couldn't set local MTU size: %s", E2S(err));
    }

    ESP_LOGI("BLE-Driver", "Ready");

end:
    return err;
}


esp_err_t ble_send (size_t len, uint8_t *buffer) {
	esp_err_t err = ESP_OK;
	struct gatts_profile_t *p = g_profile_table + APP_PROFILE_WIFI;

	// Check for null buffer pointer
	if (buffer == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	// Check for messages with a length exceeding the MTU
	if (len > BLE_RSP_MSG_MAX_SIZE) {
		return ESP_ERR_INVALID_SIZE;
	}

	// Prepare output buffer
	static uint8_t buffer_out[BLE_RSP_MSG_MAX_SIZE] = {0};
	memcpy(buffer_out, buffer, len);

	// Attempt to send as notification (because we set no confirm)
	if ((err = esp_ble_gatts_send_indicate(
		p->gatts_if,
		p->conn_id,
		p->descr_handle,
		BLE_RSP_MSG_MAX_SIZE * sizeof(uint8_t),
		buffer_out,
		false)) != ESP_OK) {
		ESP_LOGE("BLE-Driver", "Couldn't send GATTS notification: %s", 
			E2S(err));
	}

	return err;
}


/*
 *******************************************************************************
 *                         FreeRTOS Task Hook Function                         *
 *******************************************************************************
*/


// Enqueues message and notifies FreeRTOS event group of recevied BLE message
void notify_event_group (size_t size, void *buffer) {
	esp_err_t err;
	const size_t msg_lower_byte_limit = 3;

	// Ignore data under three-bytes
	if (size < msg_lower_byte_limit) {
		ESP_LOGW("BLE-Driver", "Ignoring undersized message (<%d bytes)", 
			msg_lower_byte_limit);
		return;
	}

	// Otherwise put the data on the BLE_RX_QUEUE
	if ((err = ipc_enqueue(g_ble_rx_queue, 0x0, size, buffer)) != ESP_OK) {
		ESP_LOGE("BLE-Driver", "Unable to enqueue received message: %s", 
			E2S(err));
	}

	// Notify the system
	xEventGroupSetBits(g_event_group, FLAG_BLE_RECV_MSG);
}


/*
 *******************************************************************************
 *                        Internal Function Definitions                        *
 *******************************************************************************
*/


// Establishes connection parameters when invoked in ESP_GATTS_CONNECT_EVT
static esp_ble_conn_update_params_t *gatts_set_connection_parameters 
	(esp_ble_gatts_cb_param_t *param,uint8_t profile) {

    // Connection parameters (static so pseudo-global)
	static esp_ble_conn_update_params_t connection_parameters = {
		.min_int = 16,    // x1.25ms = 20ms
		.max_int = 32,    // x1.25ms = 40ms
		.latency = 0,
		.timeout = 400,   // x10ms = 4000ms
	};

    // Copy in the device address field
    memcpy(connection_parameters.bda, param->connect.remote_bda, 
        sizeof(esp_bd_addr_t));

    // Update connection ID 
    g_profile_table[profile].conn_id = param->connect.conn_id;

    return &connection_parameters;
}


// Dispatches an indicate (a notification) to a GATT client if needed
static void gatts_send_indicate_response (esp_gatt_if_t gatts_if, 
	esp_ble_gatts_cb_param_t *param, esp_gatt_char_prop_t char_property,
	uint8_t profile) {
	esp_err_t err;
    bool needs_confirm = false;

	// Response data must be less than the MTU size
	uint8_t data_notify[BLE_RSP_MSG_MAX_SIZE] = {0};
    uint8_t data_indicate[BLE_RSP_MSG_MAX_SIZE] = {0};
    uint8_t *data_out = NULL;

	// Reconstruct the mode flag for notifications
	uint16_t mode = param->write.value[1] << 8 | param->write.value[0];

	// Handle received mode type
	switch (mode) {
		case 0: return;                       // Send no response - exit
		case 1: data_out = data_notify;       // Send response without confirm
				break;
		case 2: needs_confirm = true;         // Send response with confirm
				data_out = data_indicate;
                break;
		default:                              // Unrecognized response mode
			ESP_LOGE("BLE-Driver", "Received an unknown descriptor mode (%u)",
				mode);
			return;
	}

	// Dispatch response
	if ((err = esp_ble_gatts_send_indicate(
		gatts_if, 
		param->write.conn_id,
		g_profile_table[profile].descr_handle, // Updates go in descr
		BLE_RSP_MSG_MAX_SIZE * sizeof(uint8_t),
		data_out,
        needs_confirm)) != ESP_OK) {
		ESP_LOGE("BLE-Driver", "Couldn't send indicate/notify response: %s",
			E2S(err));
	}
}


// Handles long-characteristic write-event
void gatts_write_event_long_handler (esp_gatt_if_t gatts_if, 
	prepare_type_env_t *write_buffer, esp_ble_gatts_cb_param_t *param) {
	esp_gatt_status_t status = ESP_GATT_OK;
	esp_gatt_rsp_t *gatt_rsp = NULL;
	esp_err_t err;

	// Allocate memory for long-write
	if (write_buffer->buffer == NULL) {
		write_buffer->buffer = malloc(GATTS_WRITE_EVENT_MAX_BUF_SIZE * 
			sizeof(uint8_t));
		if (write_buffer->buffer == NULL) {
			status = ESP_GATT_NO_RESOURCES;
			goto respond;
		}
	}

	// Validate offset
	if (param->write.offset > GATTS_WRITE_EVENT_MAX_BUF_SIZE) {
		status = ESP_GATT_INVALID_OFFSET;
		goto respond;
	}

	// Validate length
	if ((param->write.offset + param->write.len) > 
		GATTS_WRITE_EVENT_MAX_BUF_SIZE) {
		status = ESP_GATT_INVALID_ATTR_LEN;
		goto respond;
	}

	// Allocate response
	if ((gatt_rsp = malloc(sizeof(esp_gatt_rsp_t))) == NULL) {
		status = ESP_GATT_NO_RESOURCES;
		goto respond;
	}

	// Set response attributes
	gatt_rsp->attr_value.len = param->write.len;
	gatt_rsp->attr_value.handle = param->write.handle;
	gatt_rsp->attr_value.offset = param->write.offset;
	gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE; // Set auth here

	// Copy parameter write value into response (echo basically)
	memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);


respond:

	// Dispatch response
	if ((err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
		param->write.trans_id, status, gatt_rsp)) != ESP_OK) {
		ESP_LOGE("BLE-Driver", "Couldn't send write-event response: %s",
			E2S(err));
	}

	// Return now if an error occurred before
	if (status != ESP_GATT_OK) {
		ESP_LOGE("BLE-Driver", "Bad status in constructing response");
		return;
	}

	// Free allocated response memory
	free(gatt_rsp);

	// Copy received data into message buffer (after response send)
	memcpy(write_buffer->buffer + param->write.offset, param->write.value,
		param->write.len);

	// Update the buffer length
	write_buffer->len += param->write.len;
}


// Handles all write-events that don't include indications/notify responses
void gatts_write_event_handler (esp_gatt_if_t gatts_if, 
	prepare_type_env_t *write_buffer, esp_ble_gatts_cb_param_t *param) {
	esp_err_t err;

	// If a response isn't needed - exit
	if (param->write.need_rsp == 0) return;

	// Invoke long-write handler if necessary
	if (param->write.is_prep != 0) {
		gatts_write_event_long_handler(gatts_if, write_buffer, param);
		return;
	}

	// Otherwise send response immediately
	if ((err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
		param->write.trans_id, ESP_GATT_OK, NULL)) != ESP_OK) {
		ESP_LOGE("BLE-Driver", "Couldn't send write-event response: %s", 
			E2S(err));
	}

	// Display the data
	ESP_LOGW("BLE-Driver", "Received characteristic write [%u bytes]:",
		param->write.len);
	ESP_LOG_BUFFER_HEX_LEVEL("BLE-Driver", param->write.value, param->write.len, 
		ESP_LOG_WARN);

	// Notify the FreeRTOS event group
	notify_event_group(param->write.len, param->write.value);

	return;
}


// Handles the executive-write message (marks the end of a long-write operation)
void gatts_write_exec_handler (prepare_type_env_t *write_buffer, 
	esp_ble_gatts_cb_param_t *param) {
	
	// Check if the write was cancelled
	if (param->exec_write.exec_write_flag != ESP_GATT_PREP_WRITE_EXEC) {
		ESP_LOGW("BLE-Driver", "A long write operation was cancelled");
	} else {
		ESP_LOGW("BLE-Driver", "Received long-characteristic write [%u bytes]:",
			write_buffer->len);
		ESP_LOG_BUFFER_HEX_LEVEL("BLE-Driver", write_buffer->buffer,
			write_buffer->len, ESP_LOG_WARN);

		// Notify the FreeRTOS event group
		notify_event_group(write_buffer->len, write_buffer->buffer);

	}


	// Reset the write buffer
	if (write_buffer->buffer != NULL) {
		free(write_buffer->buffer);
		write_buffer->buffer = NULL;
	}
	write_buffer->len = 0;
}


/*
 *******************************************************************************
 *                     GAP/GATTS Event Handler Definitions                     *
 *******************************************************************************
*/


// GAP Event Handler
static void gap_event_handler (esp_gap_ble_cb_event_t event, 
    esp_ble_gap_cb_param_t *param) {
	esp_err_t err;

	ESP_LOGI("BLE-Driver", "GAP event: %d", event);
	switch (event) {

		// Event toggled when advertising data is set
		case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT: {

			// Delete the set config_flag. Keep others unchanged
			g_adv_config_status &= (~g_adv_config_status_config_flag);

			// If any other flags are set, abort
			if (g_adv_config_status != 0) {
				break;
			}

			// Otherwise begin advertising
			if ((err = esp_ble_gap_start_advertising(&g_adv_parameters))
				!= ESP_OK) {
				ESP_LOGE("BLE-Driver", 
					"Couldn't set advertising parameters: %s", E2S(err));
            }

            ESP_LOGI("BLE-Driver", "GAP advertising setup complete");
		}
		break;


		// Event toggled when the scan response is set
		case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT: {

			// Delete the scan response flag. Keep others unchanged
			g_adv_config_status &= (~g_adv_config_status_scan_response);

			// If any other flags are set, abort
			if (g_adv_config_status != 0) {
				break;
			}

			// Otherwise begin advertising
			if ((err = esp_ble_gap_start_advertising(&g_adv_parameters))
				!= ESP_OK) {
				ESP_LOGE("BLE-Driver", 
					"Couldn't set advertising parameters: %s", E2S(err));
			}		
		}
		break;


        // Event toggled when advertising has started successfully
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT: {
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            	ESP_LOGE("BLE-Driver", "GAP advertising failed to start");
            } else {
            	ESP_LOGI("BLE-Driver", "GAP advertising started");
            }
        }
        break;


        // Event triggered if the connection parameters were updated 
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT: {
            ESP_LOGI("BLE-Driver", "Connection parameters updated");
        }
        break;


		default: {
			ESP_LOGW("BLE-Driver", "Unknown GAP event: %d", event);
		}
		break;
	}
}


// GATTs Event Handler
static void gatts_event_handler (esp_gatts_cb_event_t event, 
    esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
	int i, cb_count = 0;

	ESP_LOGD("BLE-Driver", "GATTS Event: %s", g_gatts_event_str[event]);

    // If its a registration event - we register the interface for the profile
    if (event == ESP_GATTS_REG_EVT) {

    	// If successful registration, save interface in the table.
    	if (param->reg.status == ESP_GATT_OK) {
    		g_profile_table[param->reg.app_id].gatts_if = gatts_if;
    	} else {
    		ESP_LOGE("BLE-Driver", "GATTS Event: Profile registration failed");
    	}

    	return;
    }


    // Otherwise it's not a registration event. Forward to matching profile
    for (i = 0; i < APP_PROFILE_COUNT; ++i) {

    	// Ignore entries that don't have a matching or available interface
    	if (gatts_if != ESP_GATT_IF_NONE && // if GATT_IF_NONE send to all
    		gatts_if != g_profile_table[i].gatts_if) {
    		continue;
    	}

    	// Ignore entries that don't have an assigned callback
    	if (g_profile_table[i].gatts_cb == NULL) {
    		continue;
    	}

    	// Forward event to profile callback
    	g_profile_table[i].gatts_cb(event, gatts_if, param);

    	// Increment count
    	cb_count++;
    }

    // If it belonged to no profile - issue a warning
    if (cb_count == 0) {
    	ESP_LOGW("BLE-Driver", "GATTS Event: Event is unhandled");
    }
}


/*
 *******************************************************************************
 *                         Profile Handler Definitions                         *
 *******************************************************************************
*/


// Handler for events concerning the GATTS WiFi profile
static void gatts_profile_event_handler_wifi (esp_gatts_cb_event_t event,
    esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    esp_err_t err;

    switch (event) {

        // Event for configuring advertising parameters for the specific profile
        case ESP_GATTS_REG_EVT: {

            // Set as primary service ID
            g_profile_table[APP_PROFILE_WIFI].service_id.is_primary = true;

            // The instance ID distinguishes multiple services with same ID.
            // Since we have only one service, it gets ID 0
            g_profile_table[APP_PROFILE_WIFI].service_id.id.inst_id = 0;

            // Set the length of the ID
            g_profile_table[APP_PROFILE_WIFI].service_id.id.uuid.len = 
            	ESP_UUID_LEN_16;

            // Set the UUID value.
            g_profile_table[APP_PROFILE_WIFI].service_id.id.uuid.uuid.uuid16 = 
            	GATTS_SERVICE_UUID_WIFI;

            // Set the device name
            esp_ble_gap_set_device_name(BLE_DEVICE_NAME);

            // Configure the advertising data
            if ((err = esp_ble_gap_config_adv_data(&g_advertising_data)) 
                != ESP_OK) {
            	ESP_LOGE("BLE-Driver", 
            		"GATTS Profile: Couldn't set advertising data: %s", 
            		E2S(err));
                break;
            }

            // Mark advert data config complete
            g_adv_config_status |= g_adv_config_status_config_flag;

            // Configure the scan response data
            if ((err = esp_ble_gap_config_adv_data(&g_scan_response_data)) 
                != ESP_OK) {
            	ESP_LOGE("BLE-Driver", 
            		"GATTS Profile: Couldn't set scan response: %s", E2S(err));
                break;
            }

            // Mark scan response config complete
            g_adv_config_status |= g_adv_config_status_scan_response;

			// Attempt to create the service and attribute table
			if ((err = esp_ble_gatts_create_service(gatts_if, 
				&g_profile_table[APP_PROFILE_WIFI].service_id,
				GATTS_HANDLE_COUNT_WIFI)) != ESP_OK) {
				ESP_LOGE("BLE-Driver", 
					"GATTS Profile: Failed to set profile attribute table: %s", 
					E2S(err));
			}

			ESP_LOGD("BLE-Driver", "GATTS Profile: Service ID: %u", 
				g_profile_table[APP_PROFILE_WIFI].service_id.id.uuid.uuid.uuid16);
		}
		break;


		// Event where you now add your characteristics after creating profile
		case ESP_GATTS_CREATE_EVT: {

            // Set the service handle
            g_profile_table[APP_PROFILE_WIFI].service_handle = 
            	param->create.service_handle;

            // Set the characteristic UUID length
            g_profile_table[APP_PROFILE_WIFI].char_uuid.len = ESP_UUID_LEN_16;

            // Set the UUID for the characteristic
            g_profile_table[APP_PROFILE_WIFI].char_uuid.uuid.uuid16 = 
                GATTS_CHARACTERISTIC_UUID_WIFI;

			// Try starting the service
            if ((err = esp_ble_gatts_start_service(
            	g_profile_table[APP_PROFILE_WIFI].service_handle)) != ESP_OK) {
            	ESP_LOGE("BLE-Driver", "GATTS Profile: Couldn't start service: %s", 
            		E2S(err));
            	break;
            }

            // Characteristic properties are those shown to the client only
            wifi_property = ESP_GATT_CHAR_PROP_BIT_READ  | 
            				ESP_GATT_CHAR_PROP_BIT_WRITE |
            				ESP_GATT_CHAR_PROP_BIT_NOTIFY;

            // Characteristic permissions are those actually enforced
            wifi_permissions = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE;


            // Add the characteristic; set Attribute Response Control
            if ((err = esp_ble_gatts_add_char(
            	g_profile_table[APP_PROFILE_WIFI].service_handle,
            	&g_profile_table[APP_PROFILE_WIFI].char_uuid,
            	wifi_permissions,
            	wifi_property,
            	&gatts_wifi_char_ssid_value,
            	NULL)) != ESP_OK) { // NULL means event-handling is manual
            	ESP_LOGE("BLE-Driver", 
            		"GATTS Profile: Couldn't add characteristic: %s", E2S(err));
            	break;
            }
		}
		break;


		// Event tripped when the gatts_start_service function succeeds
		case ESP_GATTS_START_EVT: {
			ESP_LOGI("BLE-Driver", "GATTS Profile: Service started");
		}
		break;


		// Event tripped by adding characteristic
		case ESP_GATTS_ADD_CHAR_EVT: {
			uint16_t len; const uint8_t *characteristic_p;

			// Check that the characteristic was added successfully
			if ((err = esp_ble_gatts_get_attr_value(
				param->add_char.attr_handle,
				&len,
				&characteristic_p)) != ESP_OK) {
				ESP_LOGE("BLE-Driver", 
					"GATTS Profile: Add characteristic failed: %s", E2S(err));
				break;
			}

            // Set the characteristic descriptor UUID length
            g_profile_table[APP_PROFILE_WIFI].descr_uuid.len = ESP_UUID_LEN_16;

            // Set the UUID for the characteristic descriptor
            g_profile_table[APP_PROFILE_WIFI].descr_uuid.uuid.uuid16 = 
            	GATTS_CHARACTERISTIC_DESCRIPTOR_UUID_WIFI;

		    // Add the characteristic descriptor if characteristic established
            if ((err = esp_ble_gatts_add_char_descr(
            	g_profile_table[APP_PROFILE_WIFI].service_handle,
            	&g_profile_table[APP_PROFILE_WIFI].descr_uuid,
            	wifi_permissions,
            	NULL,	// Initial value for characteristic descriptor
            	NULL	// Auto response parameter
            	)) != ESP_OK) {
            	ESP_LOGE("BLE-Driver", 
            		"GATTS Profile: Add characteristic descriptor failed: %s", 
            		E2S(err));
            	break;
            }

		}
		break;


        // Event tripped by adding characteristic descriptor
        case ESP_GATTS_ADD_CHAR_DESCR_EVT: {
        	ESP_LOGD("BLE-Driver", 
        		"GATTS Profile: Characterisic descriptor added");

			// Update the descriptor handle in the table
        	g_profile_table[APP_PROFILE_WIFI].descr_handle = 
        		param->add_char_descr.attr_handle;
        	
            // Log the UUID for the descriptor and status
            ESP_LOGD("BLE-Driver", "GATTS Profile: Status = %d, Value = %X\n", 
            	param->add_char.status,
            	g_profile_table[APP_PROFILE_WIFI].descr_uuid.uuid.uuid16);
        }
        break;


        // Event triggered when someone connects to the GATT server
        case ESP_GATTS_CONNECT_EVT: {

            // Connection parameters are offloaded and set here
            esp_ble_conn_update_params_t *conn_p = 
            	gatts_set_connection_parameters(param, APP_PROFILE_WIFI);

			// Apply update to connection parameters
			if ((err = esp_ble_gap_update_conn_params(conn_p)) != ESP_OK) {
				ESP_LOGE("BLE-Driver", 
					"Couldn't update connection parameters: %s", E2S(err));
			}

        	// Note: Only needs to be done ONCE for ALL PROFILES
        	ESP_LOGI("BLE-Driver", "GATTS Profile: Connect event");

        	// Notify: Mark BLE connected
        	xEventGroupSetBits(g_event_group, FLAG_BLE_CONNECTED);
        }
        break;


        // Event tripped by READ operation: Implemented if auto-resp is NULL
        case ESP_GATTS_READ_EVT: {
        	esp_gatt_rsp_t response_data;

        	// Configure response data (with junk payload)

        	// TODO: Actually send back characteristic data
        	memset(&response_data, 0, sizeof(esp_gatt_rsp_t));
        	response_data.attr_value.handle = param->read.handle;
        	response_data.attr_value.len = 4;
        	// memcpy(response_data.attr_value.value, g_characteristic_value_wifi,
        	// 	g_characteristic_value_len * sizeof(uint8_t));
        	response_data.attr_value.value[0] = 0xde;
        	response_data.attr_value.value[1] = 0xed;
        	response_data.attr_value.value[2] = 0xbe;
        	response_data.attr_value.value[3] = 0xef;

        	// Dispatch response
        	if ((err = esp_ble_gatts_send_response(
        		gatts_if,
        		param->read.conn_id,
        		param->read.trans_id,
        		ESP_GATT_OK,
        		&response_data
        		)) != ESP_OK) {
        		ESP_LOGE("BLE-Driver", 
        			"GATTS Profile: READ response failed to send: %s",
        			 E2S(err));
        		break;
        	}
        }
        break;


        // Event tripped by WRITE operation: Implemented if auto-resp is NULL
        case ESP_GATTS_WRITE_EVT: {

        	/* Two kinds of write-requests
        	 * 1. Write Characteristic Value:      (1 MTU ~= 23 bytes)
        	 * 2. Write Long Characteristic Value: (? MTU ~= ?? bytes)
        	 * Request type (2) takes several messages to transmit and receive
        	 * and to confirm it you must perfrom an executive write request
        	 *
        	 * Some messages also require notification or indication which
        	 * is handled first below
        	*/
        	

        	// Send an indication if (1) not a long write (2) is descr message
        	if (param->write.is_prep == 0 && 
        		g_profile_table[APP_PROFILE_WIFI].descr_handle == param->write.handle &&
        		param->write.len == 2
        		) {
        		ESP_LOGD("BLE-Driver", "GATTS Profile: Notifications enabled");
        		// This is sent for a specific characteristic
        		gatts_send_indicate_response(gatts_if, param, wifi_property, 
        			APP_PROFILE_WIFI);
        	} else {
        		ESP_LOGD("BLE-Driver", "GATTS Profile: Notifications disabled");
        	}

        	// Invoke the handler for write-events (characteristic specific)
        	gatts_write_event_handler(gatts_if, &wifi_message_buffer, 
        		param);
        }
        break;


        // Event tripped when a long-write has finished
        case ESP_GATTS_EXEC_WRITE_EVT: {

        	// Try sending some kind of response
        	if ((err = esp_ble_gatts_send_response(
        		gatts_if,
        		param->write.conn_id,
        		param->write.trans_id,
        		ESP_GATT_OK,
        		NULL)) != ESP_OK) {
        		ESP_LOGE("BLE-Driver", 
        			"GATTS Profile: Couldn't send executive write response: %s", 
        			E2S(err));
        	}

        	// Invoke executive write function (long write ended)
        	gatts_write_exec_handler(&wifi_message_buffer, param);
        }
        break;


        // Event tripped by a disconnection
        case ESP_GATTS_DISCONNECT_EVT: {
        	ESP_LOGW("BLE-Driver", "GATTS Profile: A disconnect occurred");

        	// Begin advertising again
        	if ((err = esp_ble_gap_start_advertising(&g_adv_parameters)) 
        		!= ESP_OK) {
        		ESP_LOGE("BLE-Driver", 
        		"GATTS Profile: Advertising failed to begin after disconnect: %s",
        			 E2S(err));
        	}

        	// Notify: Mark BLE disconnected
        	xEventGroupSetBits(g_event_group, FLAG_BLE_DISCONNECTED);
        }
        break;

		default: {
			// ESP_LOGD("BLE-Driver", "GATTS Profile: Unhandled event (%s)", 
			// 	g_gatts_event_str[event]);
		}
		break;
	}
}

#include "a2dp.h"


/*
 *******************************************************************************
 *                          Internal Type Definitions                          *
 *******************************************************************************
*/


// Event for handler "bt_av_hdl_stack_up 
enum {
    BT_APP_EVT_STACK_UP = 0,
};


/*
 *******************************************************************************
 *                        Internal Function Definitions                        *
 *******************************************************************************
*/


// Handler for advertising events with Bluetooth Classic
void bt_app_gap_cb (esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {

	switch (event) {
		case ESP_BT_GAP_AUTH_CMPL_EVT: {

			// On successful authentication
			if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
				ESP_LOGI(BT_AV_TAG, "Authentication to: %s", 
					param->auth_cmpl.device_name);
				esp_log_buffer_hex(BT_AV_TAG, param->auth_cmpl.bda, 
					ESP_BD_ADDR_LEN);
			} else {
				ESP_LOGE(BT_AV_TAG, "Authentication failed (status: %d)",
					param->auth_cmpl.stat);
			}
		}
		break;

		// Event for: Simple Secure Pairing
		case ESP_BT_GAP_CFM_REQ_EVT: {
			ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_CFM_REQ_EVT:" \
				"Can you compare the numeric value: %d", 
				param->cfm_req.num_val);
			esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
		}
		break;

		// Event for: Simple Secure Pairing
		case ESP_BT_GAP_KEY_NOTIF_EVT: {
			ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT: Passkey = %d",
				param->key_notif.passkey);
		}
		break;

		// Event for: Simple Secure Pairing
		case ESP_BT_GAP_KEY_REQ_EVT: {
			ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_KEY_REQ_EVT: Enter the passkey!");
		}
		break;

		default:
			ESP_LOGI(BT_AV_TAG, "Unhandled Event: %d", event);
		break;
	}

	return;
}


// Handler for responding to Bluetooth AV stack events
static void bt_av_hdl_stack_evt (uint16_t event, void *p_param) {
	ESP_LOGD(BT_AV_TAG, "%s event %d", __func__, event);
	char *device_name = NULL;
	esp_err_t err = ESP_OK;
	esp_avrc_rn_evt_cap_mask_t evt_set = {0};

	switch (event) {
		case BT_APP_EVT_STACK_UP: {

			// Configure device name
			device_name = DEVICE_A2DP_SERVICE_NAME;
			esp_bt_dev_set_device_name(device_name);

			// Register GAP callback for Bluetooth Classic
			esp_bt_gap_register_callback(bt_app_gap_cb);

			// Initialize AVRCP controller
			esp_avrc_ct_init();
			esp_avrc_ct_register_callback(bt_app_rc_ct_cb);

			// Initialize AVRCP target
			if ((err = esp_avrc_tg_init()) != ESP_OK) {
				break;
			}

			// Register callback and apply configuration
			esp_avrc_tg_register_callback(bt_app_rc_tg_cb);
			esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_SET, 
				&evt_set, ESP_AVRC_RN_VOLUME_CHANGE);
			if ((err = esp_avrc_tg_set_rn_evt_cap(&evt_set)) != ESP_OK) {
				break;
			}

			// Initialize A2DP sink
			esp_a2d_register_callback(&bt_app_a2d_cb);
			esp_a2d_sink_register_data_callback(bt_app_a2d_data_cb);
			esp_a2d_sink_init();

			// Set discoverable and connectable (wait to be connected)
			esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, 
				ESP_BT_GENERAL_DISCOVERABLE);
		}
		break;

		default:
			ESP_LOGE(BT_AV_TAG, "%s unhandled event %d", __func__, event);
			break;
	}

	if (err != ESP_OK) {
		ESP_LOGE(BT_AV_TAG, "%s error (event %d): %s", __func__, event, 
			E2S(err));
	}
}


/*
 *******************************************************************************
 *                        External Function Definitions                        *
 *******************************************************************************
*/


esp_err_t a2dp_init (void) {
	esp_err_t err = ESP_OK;

	// Create the A2DP application task, along with a IPC message queue
	bt_app_task_start_up();

	// Enqueues and sends a work dispatch message
	bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL,
	 0, NULL);


	// Here we assume Simple Secure Pairing (SSP) is available
	// (Make sure it is in MenuConfig)
	esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
	esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
	esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));

	// Set default parameters for legacy pairing (using fixed PIN code)
	esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
	esp_bt_pin_code_t pin_code;

	// Set the pin-code to my birth year because why not
	pin_code[0] = '1'; pin_code[1] = '9'; pin_code[2] = '9'; pin_code[3] = '6';
	esp_bt_gap_set_pin(pin_type, 4, pin_code);

	return err;
}

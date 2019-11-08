
/*
 *******************************************************************************
 *                         (C) Copyright 2019 <Nobody>                         *
 * Created: 08/11/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 * - Sonnya Dellarosa                                                          *
 *                                                                             *
 * Description:                                                                *
 *  Main file for the EKG application                                          *
 *                                                                             *
 *******************************************************************************
*/


#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_spi_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "nvs_flash.h"
#include "driver/i2s.h"

#include "err.h"
#include "wifi.h"
#include "a2dp.h"
#include "tasks.h"
#include "ipc.h"
#include "msg.h"
#include "http.h"
#include "status.h"
#include "ble_task.h"
#include "wifi_task.h"
#include "stream_task.h"
#include "telemetry_task.h"
#include "led_task.h"
#include "config.h"


/*
 *******************************************************************************
 *                             Symbolic Constants                              *
 *******************************************************************************
*/


// /* Enable the user of Software timers with FreeRTOS */
#define configUSE_TIMERS                1


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Handle for the telemetry software timer
xTimerHandle g_telemetry_timer_handle;


// Handle for the stream software timer
xTimerHandle g_stream_timer_handle;


// Holds the WiFi SSID for all tasks to access (WiFi wrapper will keep own copy)
uint8_t g_wifi_ssid_buffer[32 + 1];


// Holds the WiFi PSWD for all tasks to access (WiFi wrapper will keep own copy)
uint8_t g_wifi_pswd_buffer[64 + 1];


// Holds the WiFi IP address (network byte order)
uint32_t g_wifi_lan_addr;


// Holds the IPv4 address for telemetry (network byte order)
uint32_t g_telemetry_inet_addr;


// Holds the port for telemetry data to be sent over (network byte order)
uint16_t g_telemetry_inet_port;


// Holds the IPv4 address for the data stream (network byte order)
uint32_t g_stream_inet_addr;


// Holds the port for the data stream to be sent over (network byte order)
uint16_t g_stream_inet_port;


// Holds the path for the data stream GET request
uint8_t g_stream_url_path[64 + 1];


// Global state flag (may be subject race conditions | see msg.h for bits)
uint8_t g_state_flag;


// Global mutex for controlled access to the state flag
portMUX_TYPE g_state_mutex = portMUX_INITIALIZER_UNLOCKED;


// I2S configuration
i2s_config_t g_i2s_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_TX,                  // Not using ESP DAC
    .sample_rate             = 44100,
    .bits_per_sample         = 16,
    .channel_format          = I2S_CHANNEL_FMT_RIGHT_LEFT,  // Two-channels
    .communication_format    = I2S_COMM_FORMAT_I2S_MSB,
    .dma_buf_count           = 6,
    .dma_buf_len             = 60,
    .intr_alloc_flags        = 0,                           // Default intr prio
    .tx_desc_auto_clear      = true                         // Auto-clr underflo
};


/*
 *******************************************************************************
 *                        Utility Function Definitions                         *
 *******************************************************************************
*/


// Initializes flash-memory
esp_err_t init_flash (void) {
	esp_err_t err;

	// Attempt initialization
	err = nvs_flash_init();

	// Attempt to handle some errors
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || 
		err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		err = nvs_flash_erase();
		if (err == ESP_OK) {
			err = nvs_flash_init();
		}
	}

	return err;
}


// Initializes I2S driver
esp_err_t init_i2s (void) {
    esp_err_t err = ESP_OK;

    // Pin configuration for 
    i2s_pin_config_t pin_config = {
        .bck_io_num    = DEVICE_I2S_BCK_PIN,
        .ws_io_num     = DEVICE_I2S_LRCK_PIN, // Channel select PIN
        .data_out_num  = DEVICE_I2S_DATA_PIN,
        .data_in_num   = -1
    };

    // Init with no buffer and i2s number 0
    if ((err = i2s_driver_install(0, &g_i2s_config, 0, NULL)) != ESP_OK) {
        return err;
    }

    // Configure I2S pins
    if ((err = i2s_set_pin(0, &pin_config)) != ESP_OK) {
        return err;
    }

    return ESP_OK;
}


/*
 *******************************************************************************
 *                          Timer Callback Functions                           *
 *******************************************************************************
*/


// Callback for the telemetry software timer
void timer_callback_telemetry (TimerHandle_t xTimer) {

    // Toggle any task listening on the dispatch-telemetry flag
    xEventGroupSetBits(g_event_group, FLAG_TELEMETRY_SEND);

}


// Callback for the stream software timer
void timer_callback_stream (TimerHandle_t xTimer) {

    // Toggle the stream task
    xEventGroupSetBits(g_event_group, FLAG_STREAM_TICK);
}


/*
 *******************************************************************************
 *                    Main Application Function Definition                     *
 *******************************************************************************
*/


// Starting point for the application
void app_main (void) {
	esp_err_t err;

	// Obtain and display flash memory size
	size_t flash_size_bytes = spi_flash_get_chip_size();
	printf("Flash (MB): %d\n", flash_size_bytes / (1024 * 1024));

	// Flash MUST be initialized before WiFi now. So we do that first
	if ((err = init_flash()) != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to initialize flash memory: %s", E2S(err));
		return;
	}

    // Next the I2S driver is initialized 
    if ((err = init_i2s()) != ESP_OK) {
        ESP_LOGE("MAIN", "Failed to initialize the I2S driver: %s", E2S(err));
    }

	// Initialize global event group
	g_event_group = xEventGroupCreate();

	// Initialize the event-loop for system-events
	if ((err = esp_event_loop_create_default()) != ESP_OK) {
        ESP_LOGE("MAIN", "Couldn't start default event-loop: %s", E2S(err));
		return;
	}

    /****************************** Init Plumbing *****************************/

    // Init IPC
    if ((err = ipc_init()) != ESP_OK) {
        ESP_LOGE("MAIN", "Couldn't initialize IPC queues: %s", E2S(err));
        return;
    }

	// Init WiFi
	if ((err = wifi_init(DEVICE_WIFI_SERVICE_NAME)) != ESP_OK) {
        ESP_LOGE("MAIN", "Couldn't initialize WiFi: %s", E2S(err));
		return;
	}

    // Register WiFi Handlers (may be moved into wifi_init soon)
    if ((err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
        &wifi_event_handler, NULL)) != ESP_OK) {
        ESP_LOGE("MAIN", "Couldn't register WiFi event handler: %s", E2S(err));
        return;
    }
    if ((err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
        &wifi_event_handler, NULL)) != ESP_OK) {
        ESP_LOGE("MAIN", "Couldn't initialize IP event handler: %s", E2S(err));
    }

	// Start BLE
    if ((err = ble_init()) != ESP_OK) {
        ESP_LOGE("MAIN", "Couldn't initialize BLE handler: %s", E2S(err));
        return;
    }

    // Start Bluetooth Classic A2DP (This must come AFTER ble_init())
    if ((err = a2dp_init()) != ESP_OK) {
        ESP_LOGE("MAIN", "Couldn't initialize A2DP tasks: %s", E2S(err));
        return;
    }


    /***************************** Init User Tasks ****************************/


	// Launch BLE task
    if (xTaskCreate(task_ble_manager, "BLE Manager", STACK_SIZE_BLE_MANAGER, 
        NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE("MAIN", "Couldn't register BLE task: %s", E2S(err));
        return;
    }

    // Launch WiFi task
    if (xTaskCreate(task_wifi_manager, "WiFi Manager", STACK_SIZE_WIFI_MANAGER, 
        NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE("MAIN", "Couldn't register WiFi task: %s", E2S(err));
        return;
    }

    // Launch Telemetry task
    if (xTaskCreate(task_telemetry_manager, "Telemetry Manager", 
        STACK_SIZE_TELEMETRY_MANAGER, NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE("MAIN", "Couldn't register Telemetry task: %s", E2S(err));
        return;
    }

    // Launch LED task
    if (xTaskCreate(task_led_manager, "LED Manager", STACK_SIZE_LED_MANAGER, 
        NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE("MAIN", "Couldn't register LED task: %s", E2S(err));
        return;
    }

    // Launch Stream task
    if (xTaskCreate(task_stream_manager, "Stream Manager", 
        STACK_SIZE_STREAM_MANAGER, NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
        ESP_LOGE("MAIN", "Couldn't register Stream task: %s", E2S(err));
        return;
    }


    /***************************** Init Timer Task ****************************/


    // Initialize the telemetry software timer
    if ((g_telemetry_timer_handle = xTimerCreate(
        "Telemetry Timer-Task",     // Task name
        pdMS_TO_TICKS(DEVICE_TELEMETRY_PERIOD * 1000), // Period (seconds)
        pdTRUE,                     // The timer is periodic and not one-off
        0,                          // Timer-Identifier: Only one so just zero
        timer_callback_telemetry    // Timer callback function
        )) == NULL) {
        ESP_LOGE("MAIN", "Couldn't initialize telemetry software timer!");
    }

    // Initialize the stream software timer (for logging streaming rate)
    if ((g_stream_timer_handle = xTimerCreate(
        "Stream Timer-Task",
        pdMS_TO_TICKS(1000),         // Period of a second
        pdTRUE,
        0,
        timer_callback_stream
        )) == NULL) {
        ESP_LOGE("MAIN", "Couldn't initialize stream software timer!");
    }

    // Start the telemetry software timer
    if (xTimerStart(g_telemetry_timer_handle, 16) != pdPASS) {
        ESP_LOGE("MAIN", "Couldn't start telemetry software timer!");
    }

    // Start the stream timer
    if (xTimerStart(g_stream_timer_handle, 16) != pdPASS) {
        ESP_LOGE("MAIN", "Couldn't start stream software timer!");
    }


    ESP_LOGI("MAIN", "Startup Completed");
}

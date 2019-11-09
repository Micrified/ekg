
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
#include "tasks.h"
#include "ipc.h"
#include "msg.h"
#include "status.h"
#include "ble_task.h"
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


// Global state flag (see msg.h for bits)
uint8_t g_state_flag;


// Global state data word
uint32_t g_state_data;


// Global mutex for controlled access to the state flag
portMUX_TYPE g_state_mutex = portMUX_INITIALIZER_UNLOCKED;


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

	// Initialize global event group
	g_event_group = xEventGroupCreate();

	// Initialize the event-loop for system-events
	if ((err = esp_event_loop_create_default()) != ESP_OK) {
        ESP_LOGE("MAIN", "Couldn't start default event-loop: %s", E2S(err));
		return;
	}

    /****************************** Init Plumbing *****************************/

    // Init IPC
    // if ((err = ipc_init()) != ESP_OK) {
    //     ESP_LOGE("MAIN", "Couldn't initialize IPC queues: %s", E2S(err));
    //     return;
    // }

	// Start BLE
    // if ((err = ble_init()) != ESP_OK) {
    //     ESP_LOGE("MAIN", "Couldn't initialize BLE handler: %s", E2S(err));
    //     return;
    // }


    /***************************** Init User Tasks ****************************/


	// Launch BLE task
    // if (xTaskCreate(task_ble_manager, "BLE Manager", STACK_SIZE_BLE_MANAGER, 
    //     NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
    //     ESP_LOGE("MAIN", "Couldn't register BLE task: %s", E2S(err));
    //     return;
    // }

    // Launch Telemetry task
    // if (xTaskCreate(task_telemetry_manager, "Telemetry Manager", 
    //     STACK_SIZE_TELEMETRY_MANAGER, NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
    //     ESP_LOGE("MAIN", "Couldn't register Telemetry task: %s", E2S(err));
    //     return;
    // }

    // Launch LED task
    // if (xTaskCreate(task_led_manager, "LED Manager", STACK_SIZE_LED_MANAGER, 
    //     NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
    //     ESP_LOGE("MAIN", "Couldn't register LED task: %s", E2S(err));
    //     return;
    // }


    /***************************** Init Timer Task ****************************/


    // // Initialize the telemetry software timer
    // if ((g_telemetry_timer_handle = xTimerCreate(
    //     "Telemetry Timer-Task",     // Task name
    //     pdMS_TO_TICKS(DEVICE_TELEMETRY_PERIOD), // Period (seconds)
    //     pdTRUE,                     // The timer is periodic and not one-off
    //     0,                          // Timer-Identifier: Only one so just zero
    //     timer_callback_telemetry    // Timer callback function
    //     )) == NULL) {
    //     ESP_LOGE("MAIN", "Couldn't initialize telemetry software timer!");
    // }

    // // Start the telemetry software timer
    // if (xTimerStart(g_telemetry_timer_handle, 16) != pdPASS) {
    //     ESP_LOGE("MAIN", "Couldn't start telemetry software timer!");
    // }

    int read_raw;
    adc2_config_channel_atten(DEVICE_EKG_PIN, ADC_ATTEN_DB_0);
    const TickType_t xDelay = 4 / portTICK_PERIOD_MS;

    do {
        esp_err_t r = adc2_get_raw(DEVICE_EKG_PIN, ADC_WIDTH_12Bit, &read_raw);
        if ( r == ESP_OK ) {
            printf("%d\n", read_raw );
        } else if ( r == ESP_ERR_TIMEOUT ) {
            printf("ADC2 used by Wi-Fi.\n");
        }
        vTaskDelay(xDelay);
    } while (1);

    ESP_LOGI("MAIN", "Startup Completed");
}

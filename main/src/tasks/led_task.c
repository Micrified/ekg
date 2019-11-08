#include "led_task.h"


/*
 *******************************************************************************
 *                        External Function Definitions                        *
 *******************************************************************************
*/


void task_led_manager (void *args) {
    uint32_t flags;

    /* State Bit Flags
     * 0x1: Bluetooth connected if set
     * 0x2: WiFi connected if set
    */
    uint8_t state = 0x0;

    // Initialize LED GPIO to output mode
    gpio_pad_select_gpio(DEVICE_BLE_LED_PIN);
    gpio_set_direction(DEVICE_BLE_LED_PIN, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(DEVICE_WIFI_LED_PIN);
    gpio_set_direction(DEVICE_WIFI_LED_PIN, GPIO_MODE_OUTPUT);

    do {

        // Wait indefinitely on following flags - do NOT clear on exit
        flags = xEventGroupWaitBits(g_event_group, 
            FLAG_BLE_CONNECTED | FLAG_BLE_DISCONNECTED | 
            FLAG_WIFI_CONNECTED | FLAG_WIFI_DISCONNECTED,
            pdFALSE, pdFALSE, portMAX_DELAY);

        // If a flag is set, update state. Don't make state bit-OR of flags
        if (flags & FLAG_BLE_CONNECTED)         state |= 0x1;
        if (flags & FLAG_BLE_DISCONNECTED)      state &= ~0x1;
        if (flags & FLAG_WIFI_CONNECTED)        state |= 0x2;
        if (flags & FLAG_WIFI_DISCONNECTED)     state &= ~0x2;

        // Update LEDs
        gpio_set_level(DEVICE_BLE_LED_PIN, state & 0x1);
        gpio_set_level(DEVICE_WIFI_LED_PIN, state & 0x2);

    } while (1);


    // Destroy task
    vTaskDelete(NULL);
}
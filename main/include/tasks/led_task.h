#if !defined(LED_TASK_H)
#define LED_TASK_H


/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 04/10/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  This cute little task is responsible for turning on and off status LEDs. I *
 *  t listens for events related to important functionality (e.g. WiFi, Blueto *
 *  oth) and sets or unsets status LEDs                                        *
 *                                                                             *
 *******************************************************************************
*/


#include <inttypes.h>
#include "driver/gpio.h"
#include "tasks.h"
#include "config.h"


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* Automaton responsible for controlling status LEDs. The current variables
 * with an LED status are as follows: 
 * - Bluetooth: If a device is connected over bluetooth, an LED is set
 * - WiFi: If this device has a WiFi connection, an LED is set
*/
void task_led_manager (void *args);


#endif

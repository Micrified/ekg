#if !defined(LED_TASK_H)
#define LED_TASK_H


/*
 *******************************************************************************
 *                          (C) Copyright 2019 <None>                          *
 * Created: 08/11/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 * - Sonnya Dellarosa                                                          *
 *                                                                             *
 * Description:                                                                *
 *  This task is responsible for turning on and off the status LEDs            *
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
*/
void task_led_manager (void *args);


#endif

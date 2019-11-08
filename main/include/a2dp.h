/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 12/10/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  Contains forward facing functions and supporting callbacks related to Blue *
 *  tooth A2DP. Most of the code here is adapted from the A2DP sink example fr *
 *  om ESP-IDF                                                                 *
 *                                                                             *
 *******************************************************************************
*/


#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "bt_app_av.h"
#include "bt_app_core.h"
#include "config.h"
#include "err.h"


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* @brief Initializes the A2DP tasks and configures callbacks. 
 * @note  This must be called AFTER calling ble_init() because that
 *        will select the correct mode. 
*/
esp_err_t a2dp_init (void);
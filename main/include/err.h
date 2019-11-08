#if !defined(ERR_H)
#define ERR_H


/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 04/09/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  Common functions and macros for abstracting frequently repeated control pa *
 *  tterns                                                                     *
 *                                                                             *
 *******************************************************************************
*/


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"


/*
 *******************************************************************************
 *                             Symbolic Constants                              *
 *******************************************************************************
*/


// Macro: Shorthand for converting ESP error to string
#define E2S(err)		esp_err_to_name((err))


// Macro: Displays message with an ESP error code string
#define ERR(msg,err) {                                      \
    fprintf(stderr, "Error (%s:%d) | \"" msg "\" | %s\n",   \
        __FILE__, __LINE__, esp_err_to_name(err));          \
    fflush(stderr);                                         \
}


// Macro: Displays message with warning string
#define WARN(msg) {                                         \
    fprintf(stderr, "Warning (%s:%d) | \"" msg "\" |\n",    \
        __FILE__, __LINE__);                                \
}


// Macro: Displays success message
#define STATUS(msg) {                                       \
    fprintf(stderr, "Status (%s:%d) | \"" msg "\" |\n",     \
        __FILE__, __LINE__);                                \
}


/*
 *******************************************************************************
 *                       External Function Declarations                        *
 *******************************************************************************
*/


/* @brief Suspends a FreeRTOS task with a panic message to standard error
 * @param
 *    - msg: A constant-string message to be printed
 *    - err: An error value of type esp_err_t. If set to ESP_OK, no output from
 *           esp_err_to_name is included
 * @return None
*/
void task_panic (const char *msg, esp_err_t err);


#endif
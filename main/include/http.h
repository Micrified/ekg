#if !defined(HTTP_H)
#define HTTP_H


/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 17/09/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  Contains format strings and support for sending and receiving HTTP data    *
 *                                                                             *
 *******************************************************************************
*/


#include <stdio.h>
#include <string.h>
#include <inttypes.h>


/*
 *******************************************************************************
 *                             Symbolic Constants                              *
 *******************************************************************************
*/


// Maximum length of an HTTP POST request in bytes
#define HTTP_MAX_POST_SIZE                  256


// Maximum length of an HTTP GET request in bytes
#define HTTP_MAX_GET_SIZE					128


// Product identifier for the user-agent
#define HTTP_USER_AGENT_PRODUCT             "Somnox-ESP32"


// Product version for the user-agent
#define HTTP_USER_AGENT_PRODUCT_VERSION     "0.0"


// Dummy serial code (for possible end-point filtering)
#define PRODUCT_SERIAL                      "abcd1234"



/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* @brief Constructs an HTTP Post request with the given argument form   
 *        and returns a pointer to a buffer containing the encoded request
 *
 * @note This function is NON-REENTRANT. This means you can't call it recursiv-
 *       ly because it has only one internal buffer it can use at any instant
 *
 * @params
 * - args: A string of form "field1=value1&field2=value2&...&fieldN=valueN"
 * - size_p: A pointer to a size_t variable in which the size will be saved, if
 *           non-null
 *
 * @return Pointer to uint8_t buffer containing encoded request. If exceeds
 *         the maximum size then NULL is returned
*/
const uint8_t *http_post (const char *args, size_t *size_p);


/* @brief Constructs an HTTP Get request with the given path
 *        and returns a pointer to a buffer containing the encoded request
 *
 * @note This function is NON-REENTRANT. This means you can't call it recursiv-
 *       ly because it only has one internal buffer it can use at any instant
 *
 * @params
 * - path: The path (e.g. /team-somnox but without the forward-slash)
 * - size_p: Pointer to size_t variable in which the size will be saved if 
 *           non-null
 * 
 * @return Pointer to the uint8_t buffer containing encoded request. If exceeds
 *         the maximum size then NULL is returned
*/
const uint8_t *http_get (const char *path, size_t *size_p);


#endif
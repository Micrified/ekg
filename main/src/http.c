#include "http.h"


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Format string for HTTP POST requests (demonstrative)
const char *g_post_fmt = "POST /telemetry HTTP/1.1\r\n" \
                         "User-Agent: %s/%s\r\n" \
                         "Content-Type: application/x-www-form-urlencoded\r\n" \
                         "Content-Length: %d\r\n" \
                         "\r\n" \
                         "%s\r\n\r\n";


// Format string for HTTP GET requests (demonstative)
const char *g_get_fmt = "GET /%s HTTP/1.1\r\n" \
                        "Connection: keep-alive\r\n" \
                        "Accept: */*\r\n\r\n";


/*
 *******************************************************************************
 *                            Function Definitions                             *
 *******************************************************************************
*/


const uint8_t *http_post (const char *args, size_t *size_p) {
	int n = -1;

	// Static internal buffer
	static uint8_t buffer[HTTP_MAX_POST_SIZE];

	// Compute the Content-Length 
	int content_length = strlen(args);

	// Write the output buffer
	n = snprintf((char *)buffer, HTTP_MAX_POST_SIZE, g_post_fmt, 
		HTTP_USER_AGENT_PRODUCT, HTTP_USER_AGENT_PRODUCT_VERSION, 
		content_length, args);

	// Return NULL if the buffer was insufficiently sized
	if (!(n > 0 && n < HTTP_MAX_POST_SIZE)) {
		return NULL;
	}

	// Update size if specified
	if (size_p != NULL) {
		*size_p = n;
	}

	// Return pointer
	return buffer;
}


const uint8_t *http_get (const char *path, size_t *size_p) {

    // Static internal buffer
    static uint8_t buffer[HTTP_MAX_GET_SIZE];

    // Write the output buffer
    int n = snprintf((char *)buffer, HTTP_MAX_GET_SIZE, g_get_fmt, 
        path);

    // Return NULL if the buffer was insufficiently sized
    if (!(n > 0 && n < HTTP_MAX_POST_SIZE)) {
        return NULL;
    }

    // Update size if specified
    if (size_p != NULL) {
        *size_p = n;
    }

    return buffer;
}
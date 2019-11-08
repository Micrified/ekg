#if !defined(STREAM_TASK)
#define STREAM_TASK


/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 03/10/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  This task manages a stream of data from a TCP/IP socket. It listens to WiF *
 *  i events to know when to init/deinit the stream socket                     *
 *                                                                             *
 *******************************************************************************
*/


#include <inttypes.h>
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "esp_system.h"
#include "err.h"
#include "ipc.h"
#include "tasks.h"
#include "http.h"
#include "msg.h"
#include "status.h"


/*
 *******************************************************************************
 *                             Symbolic Constants                              *
 *******************************************************************************
*/


// Maximum number of bytes that can be read from a socket at an instance
#define SOCK_MAX_RECV_SIZE		64


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Holds the IPv4 address for the data stream (network byte order)
extern uint32_t g_stream_inet_addr;


// Holds port for the data stream to use (network byte order)
extern uint16_t g_stream_inet_port;


// Holds the path for the data stream GET request
extern uint8_t g_stream_url_path[];


// Global state flag (may be subject race conditions | see msg.h for bits)
extern uint8_t g_state_flag;


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* Automaton responsible for managing a stream of data. This task streams when
 * the following requirements are met:
 * 1. WiFi is connected
 * 2. The user has indicated that a stream should begin
 *
 * Should WiFi disconnect, the stream will be halted. On reconnection, it will
 * automatically attempt to resume. Notifications are sent when the streaming
 * state changes. The streaming state is affected by WiFi connectivity.
*/
void task_stream_manager (void *args);


#endif
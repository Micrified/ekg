#if !defined(MSG_H)
#define MSG_H


/*
 *******************************************************************************
 *                          (C) Copyright 2019 Somnox                          *
 * Created: 12/09/2019                                                         *
 *                                                                             *
 * Programmer(s):                                                              *
 * - Charles Randolph                                                          *
 *                                                                             *
 * Description:                                                                *
 *  This message describes the communication protocol used for communication o *
 *  ver BLE. More specifically, these messages are transmitted to the ESP usin *
 *  g long-write operations by the connected client, and responses are transmi *
 *  tted back using GATT notification operations (with a smaller payload)      *
 *                                                                             *
 *******************************************************************************
*/


#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"

/* A message has the following structure
 * 
 * [ HEAD HEAD | TYPE | -> ]
 *
 * I don't include my own CRC check because BLE has it already (if you enabled)
 * The type of the message denotes the length.
 *
 * The header is composed of two 8-bit markers. 
 * The type is a single byte with 8 status bits
 * The rest of the payload is contingent on the type
*/


/*
 *******************************************************************************
 *                         External Symbolic Constants                         *
 *******************************************************************************
*/


// Maximal buffer size for storing packed messages (marker + message size)
#define     MSG_BUFFER_MAX                  (2 + sizeof(msg_t))

// Byte value used for marking message headers
#define 	MSG_BYTE_HEAD                       0xFF 

// Bit for indicating WiFi is connected
#define     MSG_TYPE_WIFI_CONNECTED_BIT         0x01

// Bit for indicating Streaming is enabled
#define     MSG_TYPE_STREAM_ENABLED_BIT         0x02

// Bit for indicating Telemetry is enabled
#define     MSG_TYPE_TELEMETRY_ENABLED_BIT      0x04


// TODO: Define more status bits here


// Macro which quickly creates a status-message with status flag and WiFi addr
#define 	MSG_STATUS(stat, addr)    {   \
    .type = MSG_TYPE_STATUS,              \
    .body = (msg_body_t) {                \
        .msg_status = {                   \
            .status = (stat),             \
            .wifi_addr = (addr)           \
        }                                 \
    }                                     \
}


/*
 *******************************************************************************
 *                              Type Definitions                               *
 *******************************************************************************
*/


// Enumeration describing the type of the message received (treated as 8-bits)
typedef enum {
	MSG_TYPE_STATUS = 0,        // Message contains status bit-field only
	MSG_TYPE_WIFI_DATA,         // Message contains the WiFi SSID and PSWD
    MSG_TYPE_STREAM_DATA,       // Message contains stream host IP and path
    MSG_TYPE_TELEMETRY_DATA,    // Message contains telemetry uplink info
    MSG_TYPE_INSTRUCTION,       // Message contains a device instruction

    MSG_TYPE_MAX                // Upper boundary value for the message type 
} msg_type_t;


// Enumeration describing the type of instructions available (8-bit value)
typedef enum {
    INST_WIFI_ENABLE = 0,       // Instruct device to attempt WiFi connection
    INST_WIFI_DISABLE,          // Instruct device to disconnect from WiFi
    INST_STREAM_ENABLE,         // Instruct device to enable streaming mode
    INST_STREAM_DISABLE,        // Instruct device to stop streaming mode
    INST_TELEMETRY_ENABLE,      // Instruct device to enable telemetry
    INST_TELEMETRY_DISABLE,     // Instruct device to disable telemetry

    INST_TYPE_MAX               // Upper boundary value for instruction type
} msg_instruction_type_t;


// Structure describing a status message (contains single 8-bit status)
typedef struct {
	uint8_t status;             // Status bit-field
    uint32_t wifi_addr;         // LAN address of the device (if connected)
} msg_status_t;


// Structure describing a message containing WiFi connection credentials
typedef struct {
	uint8_t ssid[32];           // SSIDs are maximally 32-bytes long 
	uint8_t pswd[64];           // PSWDs are maximally 64-bytes long
} msg_wifi_data_t;


// Structure describing a message containing streaming data
typedef struct {
    uint32_t addr;         // 32-bit IPv4 network address (network byte order) 
    uint16_t port;         // 16-bit port address (network byte order)
    uint8_t path[64];      // Path for the stream (capped at 64 bytes)
} msg_stream_data_t;


// Structre describing a message containing telemetry data
typedef struct {
    uint32_t addr;         // 32-bit IPv4 network address (network byte order)
    uint16_t port;         // 16-bit port address (network byte order)
} msg_telemetry_data_t;


// Structure describing a message containing an instruction
typedef struct {
    uint8_t inst;          // Holds value of msg_instruction_type_t
} msg_instruction_data_t;


// Union describing a message body in general (used for buffer sizing)
typedef union {
	msg_status_t            msg_status;
	msg_wifi_data_t         msg_wifi_data;
    msg_stream_data_t       msg_stream_data;
    msg_telemetry_data_t    msg_telemetry_data;
    msg_instruction_data_t  msg_instruction;
} msg_body_t;


// Structure describing the general message
typedef struct {
    msg_type_t type;
    msg_body_t body;
} msg_t;


/*
 *******************************************************************************
 *                            Function Declarations                            *
 *******************************************************************************
*/


/* @brief Packs given message into supplied data buffer.
 *
 * @note Buffer must be at least MSG_BUFFER_MAX in size to guarantee a fit
 *
 * @param
 * - msg_t *: Pointer to message structure
 * - void * : Pointer to data buffer in which data will be stored
 *
 * @return Size (in bytes) of the message
*/
size_t msg_pack (msg_t *msg, uint8_t *buffer);


/* @brief Unpacks a buffer containing a serialized message to 
 *        the instance given at the message-type pointer.
 * @param
 * - msg: The pointer to the message type to be filled
 * - buffer: The buffer containing the serialized message
 * - len: The length of the buffer containing the serialized message
 *
 * @return
 * - ESP_OK: The message was successfully decoded
 * - ESP_ERR_INVALID_SIZE: Buffer too small to decode detected message
 * - ESP_ERR_INVALID_STATE: Headers detected but message type unknown
 * - ESP_FAIL: The message markers were not detected
*/
esp_err_t msg_unpack (msg_t *msg, uint8_t *buffer, size_t len);


/* @brief Returns a string describing the instruction type
 * 
 * @param
 * - type: The type to get the description for
 *
 * @return String describing the type. If invalid, "<Invalid>" is returned
*/
const char *inst_to_str (msg_instruction_type_t type);


#endif

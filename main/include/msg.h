#if !defined(MSG_H)
#define MSG_H


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
 *  Communication protocol used for exchanging data over BLE. Contains seriali *
 *  zation and deserialization procedures                                      *
 *                                                                             *
 *******************************************************************************
*/


#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "classifier.h"

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

// TODO: Define more status bits here


// Macro which quickly creates a status-message with status flag and WiFi addr
#define 	MSG_STATUS(stat)    {         \
    .type = MSG_TYPE_STATUS,              \
    .body = (msg_body_t) {                \
        .msg_status = {                   \
            .status = (stat)              \
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
    MSG_TYPE_TRAIN_DATA,        // Message containing all training data
    MSG_TYPE_SAMPLE_DATA,       // Message containing a data sample
    MSG_TYPE_INSTRUCTION,       // Message contains a device instruction

    MSG_TYPE_MAX                // Upper boundary value for the message type 
} msg_type_t;


// Enumeration describing the type of instructions available (8-bit value)
typedef enum {
    INST_EKG_SAMPLE = 0,        // Instruct device to sample EKG data
    INST_EKG_MONITOR,           // Instruct device to monitor user
    INST_EKG_IDLE,              // Instruct device to go into idle mode

    INST_TYPE_MAX               // Upper boundary value for instruction type
} msg_instruction_type_t;


// Structure describing a status message (contains single 8-bit status)
typedef struct {
	uint8_t status;             // Status bit-field
} msg_status_t;


// Structure describing a message containing training data
typedef struct {
    uint16_t n_periods[20];     // Periods of normal waveforms
    uint16_t n_amplitudes[20];  // Amplitudes of normal waveforms
    uint16_t a_periods[10];     // Periods of atrial premature beat
    uint16_t a_amplitudes[10];  // Amplitudes of atrial premature beat
    uint16_t v_periods[10];     // Periods of premature ventricular contractions
    uint16_t v_amplitudes[10];  // Amplitudes of premature ventricular contractions
} msg_train_data_t;


// Structure describing a message containing a data sample
typedef struct {
    uint16_t amplitude;         // Contains the amplitude of the sample
    uint16_t period;            // Contains the period since the last sample
} msg_sample_data_t;


// Structure describing a message containing an instruction
typedef struct {
    uint8_t inst;          // Holds value of msg_instruction_type_t
} msg_instruction_data_t;


// Structure describing a message containing a sample
typedef struct {
    sample_t sample;
} msg_sample_t;


// Union describing a message body in general (used for buffer sizing)
typedef union {
	msg_status_t            msg_status;
    msg_train_data_t        msg_train;
    msg_sample_data_t       msg_sample;
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

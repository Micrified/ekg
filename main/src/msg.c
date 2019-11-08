#include "msg.h"


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Update this table with new message sizes as necessary
size_t g_msg_size_tab[MSG_TYPE_MAX] = {
    [MSG_TYPE_STATUS] = 1 + 4,              // 1B status, 4B addr
    [MSG_TYPE_WIFI_DATA] = 64 + 32,         // 32-byte SSID + 64-byte PSWD
    [MSG_TYPE_STREAM_DATA] = 64 + 4 + 2,    // 4B addr, 2B port, 64B path
    [MSG_TYPE_TELEMETRY_DATA] = 4 + 2,      // 4B addr, 2B port
    [MSG_TYPE_INSTRUCTION] = 1,             // 1B instruction
};


// Update this table as new messages are introduced or removed
const char *g_inst_str_tab[INST_TYPE_MAX] = {
	[INST_WIFI_ENABLE] = "INST_WIFI_ENABLE",
	[INST_WIFI_DISABLE] = "INST_WIFI_DISABLE",
	[INST_STREAM_ENABLE] = "INST_STREAM_ENABLE",
	[INST_STREAM_DISABLE] = "INST_STREAM_DISABLE",
	[INST_TELEMETRY_ENABLE] = "INST_TELEMETRY_ENABLE",
	[INST_TELEMETRY_DISABLE] = "INST_TELEMETRY_DISABLE",
};


/*
 *******************************************************************************
 *                          Message Packing Functions                          *
 *******************************************************************************
*/


// Packs the status message
size_t pack_msg_status (msg_t *msg, uint8_t *buffer) {
	size_t z = 0;

	// Pack status
	buffer[z++] = msg->body.msg_status.status;

	// Pack WiFi address (packed lsb first or little endian)
	buffer[z++] = (msg->body.msg_status.wifi_addr >> 0)  & 0xFF;
	buffer[z++] = (msg->body.msg_status.wifi_addr >> 8)  & 0xFF;
	buffer[z++] = (msg->body.msg_status.wifi_addr >> 16) & 0xFF;
	buffer[z++] = (msg->body.msg_status.wifi_addr >> 24) & 0xFF;

	return z;
}


// Packs the WiFi data message
size_t pack_msg_wifi_data (msg_t *msg, uint8_t *buffer) {
	size_t z = 0;

	// Pack in the SSID and update the size
	memcpy(buffer, msg->body.msg_wifi_data.ssid, 32 * sizeof(uint8_t));
	z += 32;

	// Pack in the PSWD and update the size
	memcpy(buffer + z, msg->body.msg_wifi_data.pswd, 64 * sizeof(uint8_t));
	z += 64;

	return z;
}


// Packs a Stream data message
size_t pack_msg_stream_data (msg_t *msg, uint8_t *buffer) {
	size_t z = 0;

	// Pack the streaming address (packed lsb first or little endian)
	buffer[z++] = (msg->body.msg_stream_data.addr >> 0)  & 0xFF;
	buffer[z++] = (msg->body.msg_stream_data.addr >> 8)  & 0xFF;
	buffer[z++] = (msg->body.msg_stream_data.addr >> 16) & 0xFF;
	buffer[z++] = (msg->body.msg_stream_data.addr >> 24) & 0xFF;

	// Pack the streaming port (packed lsb first or little endian)
	buffer[z++] = (msg->body.msg_stream_data.port >> 0)  & 0xFF;
	buffer[z++] = (msg->body.msg_stream_data.port >> 8)  & 0xFF;

	// Copy the path in
	memcpy(buffer + z, msg->body.msg_stream_data.path, 64 * sizeof(uint8_t));
	z += 64;

	return z;
}


// Packs a Telemetry data message
size_t pack_msg_telemetry_data (msg_t *msg, uint8_t *buffer) {
	size_t z = 0;

	// Pack the telemetry address (packed lsb first or little endian)
	buffer[z++] = (msg->body.msg_telemetry_data.addr >> 0)  & 0xFF;
	buffer[z++] = (msg->body.msg_telemetry_data.addr >> 8)  & 0xFF;
	buffer[z++] = (msg->body.msg_telemetry_data.addr >> 16) & 0xFF;
	buffer[z++] = (msg->body.msg_telemetry_data.addr >> 24) & 0xFF;

	// Pack the telemetry port (packed lsb first or little endian)
	buffer[z++] = (msg->body.msg_telemetry_data.port >> 0)  & 0xFF;
	buffer[z++] = (msg->body.msg_telemetry_data.port >> 8)  & 0xFF;

	return z;
}


// Packs an Instruction data message
size_t pack_msg_instruction (msg_t *msg, uint8_t *buffer) {
	size_t z = 0;

	// Pack the instruction
	buffer[z++] = msg->body.msg_instruction.inst;

	return z;
}


/*
 *******************************************************************************
 *                         Message Unpacking Functions                         *
 *******************************************************************************
*/


// Unpacks the status message
void unpack_msg_status (msg_t *msg, uint8_t *buffer) {
	uint32_t wifi_addr = 0;
	size_t offset = 0;

	// Unpack the status
	msg->body.msg_status.status = buffer[offset++];

	// Unpack the WiFi address (if lsb packed first, unpack starting with msb)
	// Since App sends as big endian, can unpack 'wrong' way and it works
	wifi_addr = buffer[offset++]; wifi_addr <<= 8;
	wifi_addr |= buffer[offset++]; wifi_addr <<= 8;
	wifi_addr |= buffer[offset++]; wifi_addr <<= 8;
	wifi_addr |= buffer[offset++];

	msg->body.msg_status.wifi_addr = wifi_addr;
}


// Unpacks the WiFi-data message
void unpack_msg_wifi_data (msg_t *msg, uint8_t *buffer) {
	size_t offset = 0;

	// Unpack the SSID
	memcpy(msg->body.msg_wifi_data.ssid, buffer, 32 * sizeof(uint8_t));
	offset += 32;

	// Unpack the PSWD
	memcpy(msg->body.msg_wifi_data.pswd, buffer + offset, 64 * sizeof(uint8_t));
}


// Unpacks a Stream data message
void unpack_msg_stream_data (msg_t *msg, uint8_t *buffer) {
	size_t offset = 0;
	uint32_t addr;
	uint16_t port;

	// Unpack the streaming address (if lsb packed first, unpack starting with msb)
	// Since App sends as big endian, can unpack 'wrong' way and it works
	addr = buffer[offset++]; addr <<= 8;
	addr |= buffer[offset++]; addr <<= 8;
	addr |= buffer[offset++]; addr <<= 8;
	addr |= buffer[offset++];
	msg->body.msg_stream_data.addr = addr;

	// Unpack the streaming port (if lsb packed first, unpack starting with msb)
	port = buffer[offset++]; port <<= 8;
	port |= buffer[offset++];
	msg->body.msg_stream_data.port = port;

	// Unpack the path
	memcpy(msg->body.msg_stream_data.path, buffer + offset, 
		64 * sizeof(uint8_t));
}


// Unpacks a Telemetry data message
void unpack_msg_telemetry_data (msg_t *msg, uint8_t *buffer) {
	size_t offset = 0;
	uint32_t addr;
	uint16_t port;

	// Unpack the telemetry address (if lsb packed first, unpack starting with msb)
	// Since App sends as big endian, can unpack 'wrong' way and it works
	addr = buffer[offset++]; addr <<= 8;
	addr |= buffer[offset++]; addr <<= 8;
	addr |= buffer[offset++]; addr <<= 8;
	addr |= buffer[offset++];
	msg->body.msg_telemetry_data.addr = addr;

	// Unpack the telemetry port (if lsb packed first, unpack starting with msb)
	port = buffer[offset++]; port <<= 8;
	port |= buffer[offset++];
	msg->body.msg_telemetry_data.port = port;
}


// Unpacks an Instruction data message
void unpack_msg_instruction (msg_t *msg, uint8_t *buffer) {
	size_t offset = 0;
	uint8_t inst = 0;

	// Unpack the instruction
	inst = buffer[offset];

	// Assign instruction to message
	msg->body.msg_instruction.inst = inst;
}


/*
 *******************************************************************************
 *                        External Function Definitions                        *
 *******************************************************************************
*/


size_t msg_pack (msg_t *msg, uint8_t *buffer) {

	// Size is set to offset 2 for marker bytes + type
	size_t z = 3;

	// Insert leading byte markers (offset += 2)
	buffer[0] = buffer[1] = MSG_BYTE_HEAD;

	// Insert the message type (offset += 1)
	buffer[2] = msg->type;


	// Invoke switched packing procedure
	switch (msg->type) {

		case MSG_TYPE_STATUS: {
			z += pack_msg_status(msg, buffer + z);
		}
		break;

		case MSG_TYPE_WIFI_DATA: {
			z += pack_msg_wifi_data(msg, buffer + z);
		}
		break;

		case MSG_TYPE_STREAM_DATA: {
			z += pack_msg_stream_data(msg, buffer + z);
		}
		break;

		case MSG_TYPE_TELEMETRY_DATA: {
			z += pack_msg_telemetry_data(msg, buffer + z);
		}
		break;

		case MSG_TYPE_INSTRUCTION: {
			z += pack_msg_instruction(msg, buffer + z);
		}
		break;

		default:
		ESP_LOGE("MSG", "Unrecognized message type (%d)", msg->type);
		break;
	}

	return z;
}


esp_err_t msg_unpack (msg_t *msg, uint8_t *buffer, size_t len) {
	static msg_t msg_cpy;
	esp_err_t err = ESP_OK;

	// Clear out the persistent (static) msg_cpy buffer
	memset(&msg_cpy, 0, sizeof(msg_cpy));

	// Set initial known offset (two header bytes + type)
	size_t offset = 3;

	// Length must be at least 3 bytes: [Head,Head,Type]
	if (len < 3) return ESP_ERR_INVALID_SIZE;

	// Check header
	if (buffer[0] != MSG_BYTE_HEAD || buffer[1] != MSG_BYTE_HEAD) {
		return ESP_FAIL;
	}

	// Check type
	if ((msg_cpy.type = buffer[2]) > MSG_TYPE_MAX) {
		return ESP_ERR_INVALID_STATE;
	}

	// Check if enough data remains to parse the type
	if (g_msg_size_tab[msg_cpy.type] > (len - offset)) {
		return ESP_ERR_INVALID_SIZE;
	}

	// Parse on type (TODO: Use a function table instead)
	switch (msg_cpy.type) {
		case MSG_TYPE_STATUS: {
			unpack_msg_status(&msg_cpy, buffer + offset);
		}
		break;

		case MSG_TYPE_WIFI_DATA: {
			unpack_msg_wifi_data(&msg_cpy, buffer + offset);
		}
		break;

		case MSG_TYPE_STREAM_DATA: {
			unpack_msg_stream_data(&msg_cpy, buffer + offset);
		}
		break;

		case MSG_TYPE_TELEMETRY_DATA: {
			unpack_msg_telemetry_data(&msg_cpy, buffer + offset);
		}
		break;

		case MSG_TYPE_INSTRUCTION: {
			unpack_msg_instruction(&msg_cpy, buffer + offset);
		}
		break;

		default:
			err = ESP_FAIL;
		break;
	}


	// Finally update the supplied message pointer
	memcpy(msg, &msg_cpy, sizeof(msg_t));


	return err;
}


const char *inst_to_str (msg_instruction_type_t type) {
	if (type < 0 || type > INST_TYPE_MAX) {
		return "<Invalid>";
	}
	return g_inst_str_tab[type];
}


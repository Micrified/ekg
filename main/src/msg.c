#include "msg.h"


/*
 *******************************************************************************
 *                              Global Variables                               *
 *******************************************************************************
*/


// Update this table with new message sizes as necessary
size_t g_msg_size_tab[MSG_TYPE_MAX] = {
    [MSG_TYPE_STATUS]          = 1,           // 1B status
    [MSG_TYPE_TRAIN_DATA]      = 160,         // 2 * (40 + 20 + 20)
    [MSG_TYPE_SAMPLE_DATA]     = 1 + 2 + 2,   // 1B label + 2B (amp/period)
    [MSG_TYPE_INSTRUCTION]     = 1,           // 1B inst
    [MSG_TYPE_CONFIGURATION]   = 1 + 2,       // 1B comp, 2B value
};


// Update this table as new messages are introduced or removed
const char *g_inst_str_tab[INST_TYPE_MAX] = {
	[INST_EKG_STOP]      = "INST_EKG_STOP",
	[INST_EKG_START]     = "INST_EKG_START",
	[INST_EKG_CONFIGURE] = "INST_EKG_CONFIGURE"
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

	return z;
}


// Packs the Training data
size_t pack_msg_train_data (msg_t *msg, uint8_t *buffer) {
	size_t z = 0;

	// Pack n_periods[20]
	memcpy(buffer + z, msg->body.msg_train.n_periods, 20 * sizeof(uint16_t));
	z += (2 * 20);

	// Pack n_amplitudes[20]
	memcpy(buffer + z, msg->body.msg_train.n_amplitudes, 20 * sizeof(uint16_t));
	z += (2 * 20);


	// Pack a_periods[10]
	memcpy(buffer + z, msg->body.msg_train.a_periods, 10 * sizeof(uint16_t));
	z += (2 * 10);

	// Pack a_amplitudes[10]
	memcpy(buffer + z, msg->body.msg_train.a_amplitudes, 10 * sizeof(uint16_t));
	z += (2 * 10);


	// Pack v_periods[10]
	memcpy(buffer + z, msg->body.msg_train.v_periods, 10 * sizeof(uint16_t));
	z += (2 * 10);

	// Pack v_amplitudes[10]
	memcpy(buffer + z, msg->body.msg_train.v_amplitudes, 10 * sizeof(uint16_t));
	z += (2 * 10);


	return z;
}


// Packs the Sample data
size_t pack_msg_sample_data (msg_t *msg, uint8_t *buffer) {
	size_t z = 0;

	// Pack the label
	buffer[z++] = (msg->body.msg_sample.label);

	// Pack the amplitude
	buffer[z++] = (msg->body.msg_sample.amplitude >> 0) & 0xFF;
	buffer[z++] = (msg->body.msg_sample.amplitude >> 8) & 0xFF;

	// Pack the period
	buffer[z++] = (msg->body.msg_sample.period >> 0) & 0xFF;
	buffer[z++] = (msg->body.msg_sample.period >> 8) & 0xFF;

	return z;
}


// Packs an Instruction data message
size_t pack_msg_instruction (msg_t *msg, uint8_t *buffer) {
	size_t z = 0;

	// Pack the instruction
	buffer[z++] = msg->body.msg_instruction.inst;

	return z;
}


// Packs a Configuration data message
size_t pack_msg_configuration (msg_t *msg, uint8_t *buffer) {
	size_t z = 0;

	// Pack the comparator
	buffer[z++] = msg->body.msg_configuration.cfg_comp;

	// Pack the threshold
	buffer[z++] = (msg->body.msg_configuration.cfg_val >> 0) & 0xFF;
	buffer[z++] = (msg->body.msg_configuration.cfg_val >> 8) & 0xFF;

	return z;
}


/*
 *******************************************************************************
 *                         Message Unpacking Functions                         *
 *******************************************************************************
*/


// Unpacks the status message
void unpack_msg_status (msg_t *msg, uint8_t *buffer) {
	size_t offset = 0;

	// Unpack the status
	msg->body.msg_status.status = buffer[offset++];
}


// Unpacks the training data
void unpack_msg_train_data (msg_t *msg, uint8_t *buffer) {
	size_t offset = 0;

	// Unpack n_periods[20]
	memcpy(msg->body.msg_train.n_periods, buffer + offset, 
		20 * sizeof(uint16_t));
	offset += (2 * 20);

	// Unpack n_amplitudes[20]
	memcpy(msg->body.msg_train.n_amplitudes, buffer + offset, 
		20 * sizeof(uint16_t));
	offset += (2 * 20);

	// Unpack a_periods[10]
	memcpy(msg->body.msg_train.a_periods, buffer + offset,
		10 * sizeof(uint16_t));
	offset += (2 * 10);

	// Unpack a_amplitudes[10]
	memcpy(msg->body.msg_train.a_amplitudes, buffer + offset,
		10 * sizeof(uint16_t));
	offset += (2 * 10);

	// Unpack v_periods[10]
	memcpy(msg->body.msg_train.v_periods, buffer + offset,
		10 * sizeof(uint16_t));
	offset += (2 * 10);

	// Unpack v_amplitudes[10]
	memcpy(msg->body.msg_train.v_amplitudes, buffer + offset,
		10 * sizeof(uint16_t));
	offset += (2 * 10);
}


// Unpacks the sample data
void unpack_msg_sample_data (msg_t *msg, uint8_t *buffer) {
	size_t offset = 0;
	uint8_t label;
	uint16_t amplitude, period;

	// Unpack the label
	label = buffer[offset++];
	msg->body.msg_sample.label = label;

	// Unpack the amplitude
	amplitude = buffer[offset++]; amplitude <<= 8;
	amplitude |= buffer[offset++]; amplitude <<= 8;
	msg->body.msg_sample.amplitude = amplitude;

	// Unpack the period
	period = buffer[offset++]; period <<= 8;
	period |= buffer[offset++]; period <<= 8;
	msg->body.msg_sample.period = period;
}


// Unpacks an Instruction data message
void unpack_msg_instruction (msg_t *msg, uint8_t *buffer) {
	size_t offset = 0;
	uint8_t inst = 0;

	// Unpack the instruction
	inst = buffer[offset++];
	msg->body.msg_instruction.inst = inst;
}


// Unpacks a Configuration data message
void unpack_msg_configuration (msg_t *msg, uint8_t *buffer) {
	size_t offset = 0;
	uint8_t cfg_comp = 0;
	uint16_t cfg_val = 0;

	// Unpack the comparator
	cfg_comp = buffer[offset++];
	msg->body.msg_configuration.cfg_comp = cfg_comp;

	// Unpack the threshold
	cfg_val = buffer[offset++]; cfg_val <<= 8;
	cfg_val |= buffer[offset++];
	msg->body.msg_configuration.cfg_val = cfg_val;
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

		case MSG_TYPE_TRAIN_DATA: {
			z += pack_msg_train_data(msg, buffer + z);
		}
		break;

		case MSG_TYPE_SAMPLE_DATA: {
			z += pack_msg_sample_data(msg, buffer + z);
		}
		break;

		case MSG_TYPE_INSTRUCTION: {
			z += pack_msg_instruction(msg, buffer + z);
		}
		break;

		case MSG_TYPE_CONFIGURATION: {
			z += pack_msg_configuration(msg, buffer + z);
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
	if (len < 3) {
		printf("size check 1: %u < 3\n", len);
		return ESP_ERR_INVALID_SIZE;
	}

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
		printf("size check 2: %u > %u\n", g_msg_size_tab[msg_cpy.type], (len - offset));
		return ESP_ERR_INVALID_SIZE;
	}

	// Parse on type (TODO: Use a function table instead)
	switch (msg_cpy.type) {
		case MSG_TYPE_STATUS: {
			unpack_msg_status(&msg_cpy, buffer + offset);
		}
		break;

		case MSG_TYPE_TRAIN_DATA: {
			unpack_msg_train_data(&msg_cpy, buffer + offset);
		}
		break;

		case MSG_TYPE_SAMPLE_DATA: {
			unpack_msg_sample_data(&msg_cpy, buffer + offset);
		}
		break;

		case MSG_TYPE_INSTRUCTION: {
			unpack_msg_instruction(&msg_cpy, buffer + offset);
		}
		break;

		case MSG_TYPE_CONFIGURATION: {
			unpack_msg_configuration(&msg_cpy, buffer + offset);
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


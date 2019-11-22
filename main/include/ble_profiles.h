#if !defined(BLE_PROFILES_H)
#define BLE_PROFILES_H

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
 *  Implements application profile data                                        *
 *                                                                             *
 *******************************************************************************
*/


/*
 *******************************************************************************
 *                         External Symbolic Constants                         *
 *******************************************************************************
*/


// The number of handles inside the WiFi profile (four because they are)
// 1. Service handle
// 2. Characteristic handle
// 3. Characteristic value handle
// 4. Characteristic descriptor handle
#define GATTS_HANDLE_COUNT_WIFI                     4

// The UUID of the service in the app application profile
#define GATTS_SERVICE_UUID                          0x00FF

// The UUID for the app characteristic
#define GATTS_CHARACTERISTIC_UUID                   0xFF01

// The UUID for the app characteristic descriptor (using the standard CCCD type)
#define GATTS_CHARACTERISTIC_DESCRIPTOR_UUID   \
	ESP_GATT_UUID_CHAR_CLIENT_CONFIG

// Describes the maximum buffer size for a write-event handler (+1 for null t?)
#define GATTS_WRITE_EVENT_MAX_BUF_SIZE              256 + 1

// The maximum length for a characteristic value (in bytes)
#define GATTS_CHARACTERISTIC_VALUE_LENGTH_MAX        64


/*
 *******************************************************************************
 *                              Type Definitions                               *
 *******************************************************************************
*/


// Implementation of a profile: (Supports ONE service with ONE characteristic)
struct gatts_profile_t {
	esp_gatts_cb_t         gatts_cb;       // Callback function for events
	uint16_t               gatts_if;       // GATT interface ID
	uint16_t               app_id;         // Application ID
	uint16_t               conn_id;        // Connection ID
	uint16_t               service_handle; // Service Handle (index)
	esp_gatt_srvc_id_t     service_id;     // Service ID
	uint16_t               char_handle;    // Characteristic Handle (index)
	esp_bt_uuid_t          char_uuid;      // Characteristic UUID
	esp_gatt_perm_t        perm;           // Attribute permissions
	esp_gatt_char_prop_t   property;       // Properties of characteristic
	uint16_t               descr_handle;   // Config descriptor handle (index)
	esp_bt_uuid_t          descr_uuid;     // Config descriptor UUID
};


// Describes a buffer used for holding write-event data
typedef struct {
	uint8_t *buffer;
	size_t len;
} prepare_type_env_t;


/*
 *******************************************************************************
 *                          External Global Variables                          *
 *******************************************************************************
*/

// TODO: See if we want a single characteristic buffer

// The size of the data in the characteristic value
//size_t g_characteristic_value_len;

// This buffer holds the single characteristic value of the WiFi profile
//uint8_t g_characteristic_value_wifi[GATTS_WRITE_EVENT_MAX_BUF_SIZE];



#endif
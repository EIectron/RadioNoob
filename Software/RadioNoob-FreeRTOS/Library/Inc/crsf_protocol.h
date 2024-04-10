#include "m480.h"
#pragma once
#define PACKED __attribute__((packed))

#define CRSF_CRC_POLY 0xd5

#define CRSF_CHANNEL_VALUE_MIN  172 // 987us - actual CRSF min is 0 with E.Limits on
#define CRSF_CHANNEL_VALUE_1000 191
#define CRSF_CHANNEL_VALUE_MID  992
#define CRSF_CHANNEL_VALUE_2000 1792
#define CRSF_CHANNEL_VALUE_MAX  1811 // 2012us - actual CRSF max is 1984 with E.Limits on
#define CRSF_MAX_PACKET_LEN 64
#define CRSF_PACKET_SIZE	26

#define CRSF_SYNC_BYTE 0xC8

#define RCframeLength 22             // length of the RC data packed bytes frame. 16 channels in 11 bits each.
#define LinkStatisticsFrameLength 10 //
#define OpenTXsyncFrameLength 11     //
#define BattSensorFrameLength 8      //
#define VTXcontrolFrameLength 12     //

#define CRSF_PAYLOAD_SIZE_MAX 62
#define CRSF_FRAME_NOT_COUNTED_BYTES 2
#define CRSF_FRAME_SIZE(payload_size) ((payload_size) + 2) // See crsf_header_t.frame_size
#define CRSF_EXT_FRAME_SIZE(payload_size) (CRSF_FRAME_SIZE(payload_size) + 2)
#define CRSF_FRAME_SIZE_MAX (CRSF_PAYLOAD_SIZE_MAX + CRSF_FRAME_NOT_COUNTED_BYTES)
#define CRSF_FRAME_CRC_SIZE 1
#define CRSF_FRAME_LENGTH_EXT_TYPE_CRC 4 // length of Extended Dest/Origin, TYPE and CRC fields combined

#define CRSF_TELEMETRY_LENGTH_INDEX 1
#define CRSF_TELEMETRY_TYPE_INDEX 2
#define CRSF_TELEMETRY_FIELD_ID_INDEX 5
#define CRSF_TELEMETRY_FIELD_CHUNK_INDEX 6
#define CRSF_TELEMETRY_CRC_LENGTH 1
#define CRSF_TELEMETRY_TOTAL_SIZE(x) (x + CRSF_FRAME_LENGTH_EXT_TYPE_CRC)

//////////////////////////////////////////////////////////////

#define CRSF_MSP_REQ_PAYLOAD_SIZE 8
#define CRSF_MSP_RESP_PAYLOAD_SIZE 58
#define CRSF_MSP_MAX_PAYLOAD_SIZE (CRSF_MSP_REQ_PAYLOAD_SIZE > CRSF_MSP_RESP_PAYLOAD_SIZE ? CRSF_MSP_REQ_PAYLOAD_SIZE : CRSF_MSP_RESP_PAYLOAD_SIZE)

typedef enum
{
    CRSF_FRAMETYPE_GPS = 0x02,
    CRSF_FRAMETYPE_VARIO = 0x07,
    CRSF_FRAMETYPE_BATTERY_SENSOR = 0x08,
    CRSF_FRAMETYPE_BARO_ALTITUDE = 0x09,
    CRSF_FRAMETYPE_LINK_STATISTICS = 0x14,
    CRSF_FRAMETYPE_OPENTX_SYNC = 0x10,
    CRSF_FRAMETYPE_RADIO_ID = 0x3A,
    CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16,
    CRSF_FRAMETYPE_ATTITUDE = 0x1E,
    CRSF_FRAMETYPE_FLIGHT_MODE = 0x21,
    // Extended Header Frames, range: 0x28 to 0x96
    CRSF_FRAMETYPE_DEVICE_PING = 0x28,
    CRSF_FRAMETYPE_DEVICE_INFO = 0x29,
    CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B,
    CRSF_FRAMETYPE_PARAMETER_READ = 0x2C,
    CRSF_FRAMETYPE_PARAMETER_WRITE = 0x2D,

    CRSF_FRAMETYPE_ELRS_STATUS = 0x2E, // ELRS good/bad packet count and status flags

    CRSF_FRAMETYPE_COMMAND = 0x32,
    // KISS frames
    CRSF_FRAMETYPE_KISS_REQ  = 0x78,
    CRSF_FRAMETYPE_KISS_RESP = 0x79,
    // MSP commands
    CRSF_FRAMETYPE_MSP_REQ = 0x7A,   // response request using msp sequence as command
    CRSF_FRAMETYPE_MSP_RESP = 0x7B,  // reply with 58 byte chunked binary
    CRSF_FRAMETYPE_MSP_WRITE = 0x7C, // write with 8 byte chunked binary (OpenTX outbound telemetry buffer limit)
    // Ardupilot frames
    CRSF_FRAMETYPE_ARDUPILOT_RESP = 0x80,
} crsf_frame_type_e;

typedef enum {
    SUBCOMMAND_CRSF = 0x10
} crsf_command_e;

typedef enum {
    COMMAND_MODEL_SELECT_ID = 0x05
} crsf_subcommand_e;

enum {
    CRSF_FRAME_TX_MSP_FRAME_SIZE = 58,
    CRSF_FRAME_RX_MSP_FRAME_SIZE = 8,
    CRSF_FRAME_ORIGIN_DEST_SIZE = 2,
};

enum {
    CRSF_FRAME_GPS_PAYLOAD_SIZE = 15,
    CRSF_FRAME_VARIO_PAYLOAD_SIZE = 2,
    CRSF_FRAME_BARO_ALTITUDE_PAYLOAD_SIZE = 4, // TBS version is 2, ELRS is 4 (combines vario)
    CRSF_FRAME_BATTERY_SENSOR_PAYLOAD_SIZE = 8,
    CRSF_FRAME_ATTITUDE_PAYLOAD_SIZE = 6,
    CRSF_FRAME_DEVICE_INFO_PAYLOAD_SIZE = 48,
    CRSF_FRAME_FLIGHT_MODE_PAYLOAD_SIZE = 16,
    CRSF_FRAME_GENERAL_RESP_PAYLOAD_SIZE = CRSF_EXT_FRAME_SIZE(CRSF_FRAME_TX_MSP_FRAME_SIZE)
};

typedef enum
{
    CRSF_ADDRESS_BROADCAST = 0x00,
    CRSF_ADDRESS_USB = 0x10,
    CRSF_ADDRESS_TBS_CORE_PNP_PRO = 0x80,
    CRSF_ADDRESS_RESERVED1 = 0x8A,
    CRSF_ADDRESS_CURRENT_SENSOR = 0xC0,
    CRSF_ADDRESS_GPS = 0xC2,
    CRSF_ADDRESS_TBS_BLACKBOX = 0xC4,
    CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8,
    CRSF_ADDRESS_RESERVED2 = 0xCA,
    CRSF_ADDRESS_RACE_TAG = 0xCC,
    CRSF_ADDRESS_RADIO_TRANSMITTER = 0xEA,
    CRSF_ADDRESS_CRSF_RECEIVER = 0xEC,
    CRSF_ADDRESS_CRSF_TRANSMITTER = 0xEE,
    CRSF_ADDRESS_ELRS_LUA = 0xEF
} crsf_addr_e;

//typedef struct crsf_addr_e asas;

typedef enum
{
    CRSF_UINT8 = 0,
    CRSF_INT8 = 1,
    CRSF_UINT16 = 2,
    CRSF_INT16 = 3,
    CRSF_UINT32 = 4,
    CRSF_INT32 = 5,
    CRSF_UINT64 = 6,
    CRSF_INT64 = 7,
    CRSF_FLOAT = 8,
    CRSF_TEXT_SELECTION = 9,
    CRSF_STRING = 10,
    CRSF_FOLDER = 11,
    CRSF_INFO = 12,
    CRSF_COMMAND = 13,
    CRSF_VTX = 15,
    CRSF_OUT_OF_RANGE = 127,
} crsf_value_type_e;

// These flags are or'ed with the field type above to hide the field from the normal LUA view
#define CRSF_FIELD_HIDDEN       0x80     // marked as hidden in all LUA responses
#define CRSF_FIELD_ELRS_HIDDEN  0x40     // marked as hidden when talking to ELRS specific LUA
#define CRSF_FIELD_TYPE_MASK    ~(CRSF_FIELD_HIDDEN|CRSF_FIELD_ELRS_HIDDEN)

/**
 * Crossfire packed channel structure, each channel is 11 bits
 */
typedef struct crsf_channels_s
{
    unsigned ch0 : 11;
    unsigned ch1 : 11;
    unsigned ch2 : 11;
    unsigned ch3 : 11;
    unsigned ch4 : 11;
    unsigned ch5 : 11;
    unsigned ch6 : 11;
    unsigned ch7 : 11;
    unsigned ch8 : 11;
    unsigned ch9 : 11;
    unsigned ch10 : 11;
    unsigned ch11 : 11;
    unsigned ch12 : 11;
    unsigned ch13 : 11;
    unsigned ch14 : 11;
    unsigned ch15 : 11;
} PACKED crsf_channels_t;

typedef struct deviceInformationPacket_s
{
    uint32_t serialNo;
    uint32_t hardwareVer;
    uint32_t softwareVer;
    uint8_t fieldCnt;          //number of field of params this device has
    uint8_t parameterVersion;
} PACKED deviceInformationPacket_t;

#define DEVICE_INFORMATION_PAYLOAD_LENGTH (sizeof(deviceInformationPacket_t) + strlen(device_name)+1)
#define DEVICE_INFORMATION_LENGTH (sizeof(crsf_ext_header_t) + DEVICE_INFORMATION_PAYLOAD_LENGTH + CRSF_FRAME_CRC_SIZE)
#define DEVICE_INFORMATION_FRAME_SIZE (DEVICE_INFORMATION_PAYLOAD_LENGTH + CRSF_FRAME_LENGTH_EXT_TYPE_CRC)

//CRSF_FRAMETYPE_BATTERY_SENSOR
typedef struct crsf_sensor_battery_s
{
    unsigned voltage : 16;  // mv * 100 BigEndian
    unsigned current : 16;  // ma * 100
    unsigned capacity : 24; // mah
    unsigned remaining : 8; // %
} PACKED crsf_sensor_battery_t;

// CRSF_FRAMETYPE_BARO_ALTITUDE
typedef struct crsf_sensor_baro_vario_s
{
    uint16_t altitude; // Altitude in decimeters + 10000dm, or Altitude in meters if high bit is set, BigEndian
    int16_t verticalspd;  // Vertical speed in cm/s, BigEndian
} PACKED crsf_sensor_baro_vario_t;

typedef struct crsfPayloadGPS_s
{
    uint32_t gps_lat;
    uint32_t gps_long;
    uint8_t gps_sat;
} PACKED crsfGPS_t;

/*
 * 0x14 Link statistics
 * Payload:
 *
 * uint8_t Uplink RSSI Ant. 1 ( dBm * -1 )
 * uint8_t Uplink RSSI Ant. 2 ( dBm * -1 )
 * uint8_t Uplink Package success rate / Link quality ( % )
 * int8_t Uplink SNR ( db )
 * uint8_t Diversity active antenna ( enum ant. 1 = 0, ant. 2 )
 * uint8_t RF Mode ( enum 4fps = 0 , 50fps, 150hz)
 * uint8_t Uplink TX Power ( enum 0mW = 0, 10mW, 25 mW, 100 mW, 500 mW, 1000 mW, 2000mW )
 * uint8_t Downlink RSSI ( dBm * -1 )
 * uint8_t Downlink package success rate / Link quality ( % )
 * int8_t Downlink SNR ( db )
 * Uplink is the connection from the ground to the UAV and downlink the opposite direction.
 */

typedef struct crsfPayloadLinkstatistics_s
{
    uint8_t uplink_RSSI_1;
    uint8_t uplink_RSSI_2;
    uint8_t uplink_Link_quality;
    int8_t uplink_SNR;
    uint8_t active_antenna;
    uint8_t rf_Mode;
    uint8_t uplink_TX_Power;
    uint8_t downlink_RSSI;
    uint8_t downlink_Link_quality;
    int8_t downlink_SNR;
} crsfLinkStatistics_t;

typedef struct crsfPayloadLinkstatistics_s crsfLinkStatistics_t;


/*******************************
  Crossfire
********************************/
typedef enum
{
    TELEM_CRSF_RX_RSSI1 = 1,
    TELEM_CRSF_RX_RSSI2,
    TELEM_CRSF_RX_QUALITY,
    TELEM_CRSF_RX_SNR,
    TELEM_CRSF_RX_ANTENNA,
    TELEM_CRSF_RF_MODE,
    TELEM_CRSF_TX_POWER,
    TELEM_CRSF_TX_RSSI,
    TELEM_CRSF_TX_QUALITY,
    TELEM_CRSF_TX_SNR,
    TELEM_CRSF_BATT_VOLTAGE,
    TELEM_CRSF_BATT_CURRENT,
    TELEM_CRSF_BATT_CAPACITY,
    TELEM_CRSF_GPS_LATITUDE,
    TELEM_CRSF_GPS_LONGITUDE,
    TELEM_CRSF_GPS_GROUND_SPEED,
    TELEM_CRSF_GPS_HEADING,
    TELEM_CRSF_GPS_ALTITUDE,
    TELEM_CRSF_GPS_SATELLITES,
    TELEM_CRSF_ATTITUDE_PITCH,
    TELEM_CRSF_ATTITUDE_ROLL,
    TELEM_CRSF_ATTITUDE_YAW,
    TELEM_CRSF_FLIGHT_MODE,
    TELEM_CRSF_LAST
} crossfire_telem_t;

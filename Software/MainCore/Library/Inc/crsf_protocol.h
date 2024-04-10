#include "m480.h"
#pragma once
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
    int8_t uplink_RSSI_1;
    int8_t uplink_RSSI_2;
    uint8_t uplink_Link_quality;
    int8_t uplink_SNR;
    uint8_t active_antenna;
    uint8_t rf_Mode;
    uint8_t uplink_TX_Power;
    int8_t downlink_RSSI;
    uint8_t downlink_Link_quality;
    int8_t downlink_SNR;
} crsfLinkStatistics_t;


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

    CRSF_FRAMETYPE_ELRS_STATUS = 0x2E, // ELRS good/bad packet count and status flags //***************

    CRSF_FRAMETYPE_COMMAND = 0x32,
    // KISS frames
    CRSF_FRAMETYPE_KISS_REQ = 0x78,
    CRSF_FRAMETYPE_KISS_RESP = 0x79,
    // MSP commands
    CRSF_FRAMETYPE_MSP_REQ = 0x7A,   // response request using msp sequence as command
    CRSF_FRAMETYPE_MSP_RESP = 0x7B,  // reply with 58 byte chunked binary
    CRSF_FRAMETYPE_MSP_WRITE = 0x7C, // write with 8 byte chunked binary (OpenTX outbound telemetry buffer limit)
    // Ardupilot frames
    CRSF_FRAMETYPE_ARDUPILOT_RESP = 0x80,
} crsf_frame_type_e;


typedef struct crsfPayloadGPS_s
{
    uint32_t gps_lat;
    uint32_t gps_long;
    uint8_t gps_sat;
} crsfGPS_t;


typedef struct crsfPayloadBAT_s
{
	int16_t	voltage;
} crsfBAT_t;

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

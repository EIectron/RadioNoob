#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "common.h"
#include "EEPROM.h"
#include "Delay.h"
#include "Inputs.h"
#include "crsf_protocol.h"
#include "ring_buffer.h"
 
#define CRSF_MAX_NAME_LEN 20
#define CRSF_FLGHT_MODE_LEN 14

#define CRSF_MAX_PARAMS 100 // one extra required, max observed is 47 in Diversity Nano RX
#define CRSF_MAX_DEVICES 4

#define CRSF_MAX_CHUNK_SIZE 58 // 64 - header - type - destination - origin
#define CRSF_MAX_CHUNKS 8     // not in specification. Max observed is 3 for Nano RX

// Basic setup
//#define CRSF_MAX_CHANNEL        16
#define CRSF_FRAME_SIZE_MAX     64
#define CRSF_MAX_PARAM_SIZE			256
// Device address & type
#define RADIO_ADDRESS           0xEA
#define ADDR_MODULE             0xEE  //  Crossfire transmitter
#define TYPE_CHANNELS           0x16

// Define RC input limite
#define RC_CHANNEL_MIN 172
#define RC_CHANNEL_MID 991
#define RC_CHANNEL_MAX 1811

// Define AUX channel input limite
#define CRSF_DIGITAL_CHANNEL_MIN 172
#define CRSF_DIGITAL_CHANNEL_MAX 1811

#define CRSF_FRAME_PERIOD_MIN 850   // 1000Hz 1ms, but allow shorter for offset cancellation
#define CRSF_FRAME_PERIOD_MAX 50000 // 25Hz  40ms, but allow longer for offset cancellation

// internal crsf variables
#define CRSF_TIME_NEEDED_PER_FRAME_US 1100 // 700 ms + 400 ms for potential ad-hoc request
#define CRSF_TIME_BETWEEN_FRAMES_US 5000   // 4 ms 250Hz
#define CRSF_PAYLOAD_OFFSET offsetof(crsfFrameDef_t, type)
#define CRSF_MSP_RX_BUF_SIZE 128
#define CRSF_MSP_TX_BUF_SIZE 128
#define CRSF_PACKET_LENGTH 22
#define CRSF_PACKET_SIZE 26
#define CRSF_FRAME_LENGTH 24 // length of type + payload + crc
#define CRSF_CMD_PACKET_SIZE 8
#define LinkStatisticsFrameLength 10 //

// ELRS command
#define ELRS_ADDRESS                    0xEE
#define ELRS_RX_ADDRESS 								0xEC
#define ELRS_BIND_COMMAND               0xFF
#define ELRS_WIFI_COMMAND               0xFE
#define ELRS_PKT_RATE_COMMAND           0x01
#define ELRS_TLM_RATIO_COMMAND          0x02
#define ELRS_SWITCH_MODE_COMMAND        0x03
#define ELRS_MODEL_MATCH_COMMAND        0x04
#define ELRS_POWER_COMMAND 							0x03
#define ELRS_BLE_JOYSTIC_COMMAND        0x11
#define TYPE_SETTINGS_WRITE             0x2D
#define ADDR_RADIO                      0xEA //  Radio Transmitter
#define SERIAL_BAUDRATE 								400000 // testing 3750000//1870000
#define port                            UART1

// Frame Type
#define TYPE_GPS 						0x02
#define TYPE_VARIO 					0x07
#define TYPE_BATTERY 				0x08
#define TYPE_HEARTBEAT 			0x0b
#define TYPE_VTX 						0x0F
#define TYPE_VTX_TELEM 			0x10
#define TYPE_LINK 					0x14
#define TYPE_CHANNELS				0x16
#define TYPE_RX_ID 					0x1C
#define TYPE_TX_ID 					0x1D
#define TYPE_ATTITUDE 			0x1E
#define TYPE_FLIGHT_MODE 		0x21
#define TYPE_PING_DEVICES 	0x28
#define TYPE_DEVICE_INFO 		0x29
#define TYPE_REQUEST_SETTINGS 0x2A
#define TYPE_SETTINGS_ENTRY 	0x2B
#define TYPE_SETTINGS_READ 		0x2C
#define TYPE_SETTINGS_WRITE 	0x2D
#define TYPE_ELRS_INFO 				0x2E
#define TYPE_COMMAND_ID 			0x32
#define TYPE_RADIO_ID 				0x3A

// Frame Subtype
#define UART_SYNC 							0xC8
#define CRSF_SUBCOMMAND 				0x10
#define COMMAND_MODEL_SELECT_ID 0x05

#define TELEMETRY_RX_PACKET_SIZE 64
#define CRSF_MAX_FIXEDID 				 63
#define CRSF_CRC_POLY 					 0xD5

#define CRSF_MAX_PACKET_LEN 64

#define SEND_MSG_BUF_SIZE 64 		// don't send more than one chunk
#define ADDR_BROADCAST 		0x00  //  Broadcast address


#define MODULE_IS_ELRS (module_type == MODULE_ELRS)
#define MODULE_IS_UNKNOWN (module_type == MODULE_UNKNOWN)


typedef struct
{
    uint8_t address;
    uint8_t number_of_params;
    uint8_t params_version;
    uint32_t serial_number;
    uint32_t hardware_id;
    uint32_t firmware_id;
    char name[CRSF_MAX_NAME_LEN];
} crsf_device_t;

typedef enum
{
    MODULE_UNKNOWN,
    MODULE_ELRS,
    MODULE_OTHER,
} module_type_t;

typedef struct
{
    uint8_t update;
    uint8_t bad_pkts;
    uint16_t good_pkts;
    uint8_t flags;
    char flag_info[CRSF_MAX_NAME_LEN];
} elrs_info_t;

enum data_type {
    UINT8          = 0,
    INT8           = 1,
    UINT16         = 2,
    INT16          = 3,
    FLOAT          = 8,
    TEXT_SELECTION = 9,
    STRING         = 10,
    FOLDER         = 11,
    INFO           = 12,
    COMMAND        = 13,
    OUT_OF_RANGE   = 127,
};

typedef struct {
    // common fields
    u8 device;            	 // device index of device parameter belongs to
    u8 id;                	 // Parameter number (starting from 1)
    u8 parent;           	   // Parent folder parameter number of the parent folder, 0 means root
    u8 chunk;
    enum data_type type;  	 // (Parameter type definitions and hidden bit)
    u8 hidden;            	 // set if hidden
    char *name;           	 // Null-terminated string
    char options[25][20];    // size depending on data type
    char value[40];
    u8 status;
    u8 count;

    // field presence depends on type
    u8 default_value;     // size depending on data type. Not present for COMMAND.
    u8 current_value;
    s32 min_value;        // not sent for string type
    s32 max_value;        // not sent for string type
//    s32 step;             // Step size ( type float only otherwise this entry is not sent )
    u8 timeout;           // COMMAND timeout (100ms/count)
//    u8 changed;           // flag if set needed when edit element is de-selected
//    char *max_str;        // Longest choice length for text select
    union {
//        u8 point;             // Decimal point ( type float only otherwise this entry is not sent )
//        u8 text_sel;          // current value index for TEXT_SELECTION type
//        u8 string_max_len;    // String max length ( for string type only )
        u8 status;            // Status for COMMANDs
    } u;
    union {
        char info[40];
        char unit[20];         // Unit ( Null-terminated string / not sent for type string and folder )
    } s;
} crsf_param_t;

enum cmd_status {
    READY               = 0,
    START               = 1,
    PROGRESS            = 2,
    CONFIRMATION_NEEDED = 3,
    CONFIRM             = 4,
    CANCEL              = 5,
    POLL                = 6
};

static struct {
    u32 time;
    u16 timeout;
    u8  dialog;
} command;

enum ParameterID{
    Packet_Rate = 1,
    Telem_Ratio = 2,
    Switch_Mode = 3,
    Model_Match = 4,
    TX_Power 		= 5,
		Max_Power		= 6,
		Dynamic 		= 7,
		Fan_Threshold = 8,
    VTX_Administrator = 9,
		Band	= 10,
		Channel = 11,
		Pwr_Lvl = 12,
		Pitmode = 13,
		Send_VTx	= 14,
    WiFi_Connectivity = 15,
		Enable_Wifi	= 16,
		Enable_Rx_Wifi	= 17,
		BLE_Joystick = 18,
		Bind = 19,
		Bad_Good = 20,
		Info = 21
};

extern RCValues myRCvalues;


/// UART Handling ///
static volatile uint8_t SerialInPacketLen; // length of the CRSF packet as measured
static volatile uint8_t SerialInPacketPtr; // index where we are reading/writing
static volatile bool CRSFframeActive;      // = false; //since we get a copy of the serial data use this flag to know when to ignore it

extern char recv_param_buffer[];
extern char *recv_param_ptr;

void CRSF_begin(void);
void CRSF_PrepareDataPacket(uint8_t packet[], int16_t channels[]);
void CRSF_PrepareCmdPacket(uint8_t packetCmd[], uint8_t command, uint8_t value);
void CRSF_WritePacket(uint8_t packet[], uint8_t packetLength);
void serialEvent(void);


void CRSF_PrepareRCPacket(int16_t (*rc_input)[CRSF_MAX_CHANNEL], ChannelCalibration (*channelCalibs)[NUM_ANALOG_CHANNELS]);
void CRSF_SendChannels(void);
void CRSF_read_param(uint8_t n_param, uint8_t n_chunk, uint8_t target);
void CRSF_changeParam(uint8_t n_param, uint8_t n_chunk);
void CRSF_broadcast_ping(void);
void CRSF_get_elrs_info(uint8_t target);
void CRSF_send_id(uint8_t modelId);
bool ModulIsELRS(void);
uint8_t CRSF_GetDeviceStat(void);
void EnableBLE_Joystick(void);
void CRSF_Update(void);
uint8_t ParamsLoaded(uint8_t addr);

void check_link_state(uint32_t currentMicros);


extern crsfLinkStatistics_t LinkStatistics; // Link Statisitics Stored as Struct


static u8 crsf_crc(const u8 crctab[], const u8 *ptr, u8 len);
u8 crsf_crc8(const u8 *ptr, u8 len);
u8 crsf_crc8_BA(const u8 *ptr, u8 len);
void crsf_crc8_acc(u8 *crc, const u8 val);
void crsf_crc8_BA_acc(u8 *crc, const u8 val);


/* ESP32 Team900
https://github.com/danxdz/simpleTx_esp32/blob/master/src/Simple_TX.ino
        // buildElrsPacket(crsfCmdPacket,X,3);
        // 0 : ELRS status request => ??
        // 1 : Set Lua [Packet Rate]= 0 - 50Hz / 1 - 150Hz / 3 - 250Hz
        // 2 : Set Lua [Telem Ratio]= 0 - off / 1 - 1:128 / 2 - 1:64 / 3 - 1:32 / 4 - 1:16 / 5 - 1:8 / 6 - 1:4 / 7 - 1:2
        // 3 : Set Lua [Switch Mode]=0 -> Hybrid;Wide
        // 4 : Set Lua [Model Match]=0 -> Off;On
        // 5 : Set Lua [TX Power]=0 - 10mW / 1 - 25mW / 2 - 50mW /3 - 100mW/4 - 250mW
        // 6 : Set Lua [Max Power]=0 - 10mW / 1 - 25mW *dont force to change, but change after reboot if last power was greater
        // 7 : Set Lua [Dynamic]=0 -> Off;On;AUX9;AUX10;AUX11;AUX12 -> * @ ttgo screen
        // 8 : Set Lua [VTX Administrator]=0
        // 9 : Set Lua [Band]=0 -> Off;A;B;E;F;R;L
        // 10: Set Lua [Channel]=0 -> 1;2;3;4;5;6;7;8
        // 11: Set Lua [Pwr Lvl]=0 -> -;1;2;3;4;5;6;7;8
        // 12: Set Lua [Pitmode]=0 -> Off;On
        // 13: Set Lua [Send VTx]=0 sending response for [Send VTx] chunk=0 step=2
        // 14: Set Lua [WiFi Connectivity]=0
        // 15: Set Lua [Enable WiFi]=0 sending response for [Enable WiFi] chunk=0 step=0
        // 16: Set Lua [Enable Rx WiFi]=0 sending response for [Enable Rx WiFi] chunk=0 step=2
        // 17: Set Lua [BLE Joystick]=0 sending response for [BLE Joystick] chunk=0 step=0
        //     Set Lua [BLE Joystick]=1 sending response for [BLE Joystick] chunk=0 step=3
        //     Set Lua [BLE Joystick]=2 sending response for [BLE Joystick] chunk=0 step=3
        // 19: Set Lua [Bad/Good]=0
        // 20: Set Lua [2.1.0 EU868]=0 =1 ?? get
*/
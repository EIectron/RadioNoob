#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "common.h"
#include "eeprom.h"
#include "Delay.h"
#include "Inputs.h"
#include "crsf_protocol.h"
#include "ring_buffer.h"


 
// internal crsf variables
#define CRSF_TIME_NEEDED_PER_FRAME_US 	1100 	 // 700 ms + 400 ms for potential ad-hoc request
#define CRSF_TIME_BETWEEN_FRAMES_US	  	5000   // 4 ms 250Hz
#define CRSF_FRAME_PERIOD_MIN 					850    // 1000Hz 1ms, but allow shorter for offset cancellation
#define CRSF_FRAME_PERIOD_MAX 					50000  // 25Hz  40ms, but allow longer for offset cancellation
#define CRSF_MAX_NAME_LEN 							20
#define SERIAL_BAUDRATE 								400000 // testing 3750000//1870000
#define CRSF_PORT                       UART1
#define CRSF_IRQ												UART1_IRQn
#define MODULE_IS_ELRS (module_type == MODULE_ELRS)
#define MODULE_IS_UNKNOWN (module_type == MODULE_UNKNOWN)

#define CRSF_MAX_PARAMS 100 // one extra required, max observed is 47 in Diversity Nano RX
#define CRSF_MAX_DEVICES 4
#define CRSF_MAX_CHANNEL 16


typedef enum
{
    MODUL_FIND,
    CHANNEL_SEND,
    LINK_STATE
} crsf_state_t;


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


/// UART Handling ///
static volatile uint8_t SerialInPacketLen; // length of the CRSF packet as measured
static volatile uint8_t SerialInPacketPtr; // index where we are reading/writing
static volatile bool CRSFframeActive;      // = false; //since we get a copy of the serial data use this flag to know when to ignore it

extern char recv_param_buffer[];
extern char *recv_param_ptr;



void CRSF_Begin(void);
void CRSF_Broadcast_Ping(void);
void CRSF_Get_Elrs_Info(uint8_t target);

void protocol_module_type(module_type_t type);

void CRSF_Update(void);


void CRSF_PrepareDataPacket(uint8_t packet[], int16_t channels[]);
void CRSF_PrepareCmdPacket(uint8_t packetCmd[], uint8_t command, uint8_t value);
void CRSF_WritePacket(uint8_t packet[], uint8_t packetLength);
void serialEvent(void);


void CRSF_SendChannels(void);
void CRSF_read_param(uint8_t n_param, uint8_t n_chunk, uint8_t target);
void CRSF_changeParam(uint8_t n_param, uint8_t n_chunk);


void CRSF_send_id(uint8_t modelId);
uint8_t CRSF_GetDeviceStat(void);
void EnableBLE_Joystick(void);

uint8_t ParamsLoaded(uint8_t addr);

void check_link_state(uint32_t currentMicros);


extern crsfLinkStatistics_t LinkStatistics; // Link Statisitics Stored as Struct


static u8 crsf_crc(const u8 crctab[], const u8 *ptr, u8 len);
u8 crsf_crc8(const u8 *ptr, u8 len);
u8 crsf_crc8_BA(const u8 *ptr, u8 len);
void crsf_crc8_acc(u8 *crc, const u8 val);
void crsf_crc8_BA_acc(u8 *crc, const u8 val);



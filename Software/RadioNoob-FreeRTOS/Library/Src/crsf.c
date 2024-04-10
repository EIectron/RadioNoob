/*
 * This file is part of Simple TX
 *
 * Simple TX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Simple TX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 =======================================================================================================
 * CRSF protocol
 *
 * CRSF protocol uses a single wire half duplex uart connection.
 * The master sends one frame every 4ms and the slave replies between two frames from the master.
 *
 * 420000 baud
 * not inverted
 * 8 Bit
 * 1 Stop bit
 * Big endian
 * ELRS uses crossfire protocol at many different baud rates supported by EdgeTX i.e. 115k, 400k, 921k, 1.87M, 3.75M
 * 115000 bit/s = 14400 byte/s
 * 420000 bit/s = 46667 byte/s (including stop bit) = 21.43us per byte
 * Max frame size is 64 bytes
 * A 64 byte frame plus 1 sync byte can be transmitted in 1393 microseconds.
 *
 * CRSF_TIME_NEEDED_PER_FRAME_US is set conservatively at 1500 microseconds
 *
 * Every frame has the structure:
 * <Device address><Frame length><Type><Payload><CRC>
 *
 * Device address: (uint8_t)
 * Frame length:   length in  bytes including Type (uint8_t)
 * Type:           (uint8_t)
 * CRC:            (uint8_t)
 *
 */

#include "crsf.h"

//#define DEBUG_SYNC

bool powerChangeHasRun = false;
uint32_t clickCurrentMicros = 0;

uint8_t rxConected = 1;
uint8_t txConected = 1;

uint32_t crsfTime = 0;
uint32_t lastCrsfTime = 0;
uint32_t updateInterval = CRSF_TIME_BETWEEN_FRAMES_US;
int32_t correction = 0;
uint32_t updated_interval = 0;
uint32_t update = 0;

uint8_t SerialInBuffer[CRSF_MAX_PACKET_LEN];

crsf_device_t crsf_devices[CRSF_MAX_DEVICES];
elrs_info_t local_info;
elrs_info_t elrs_info;
crsfLinkStatistics_t LinkStatistics;
module_type_t module_type;
crsfGPS_t GPS;
crsf_sensor_battery_t Battery;
extern RingBuffer_t RingBuffer;
extern TableParams myTableParams;
extern RCValues myRCvalues;
static char elrsFlagsInfo[20] = "";

uint32_t tickTime = 0;
uint32_t tickInterval = 2000000; // 2 sec. to check if rx or tx connect/disconnect
uint16_t rates[] = {0, 25, 50, 100, 150, 200};
int16_t rcChannels[CRSF_MAX_CHANNEL];

volatile bool receiveStat = false;
uint8_t temp = 0;

u8 next_param = 0;
u8 next_chunk = 0;
u8 paramUpdate = 0;
crsf_param_t crsf_params[25];

char param_buff[255];
u8 param_ptr = 0;
bool packet_finish = false;

u8 command_stat = 0; //False->Read Param, True->Write Param


char debug_buff[255];


// crc implementation from CRSF protocol document rev7
static uint8_t crc8tab[256] = {
    0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54, 0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
    0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06, 0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
    0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0, 0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
    0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2, 0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
    0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9, 0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
    0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B, 0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
    0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D, 0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
    0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F, 0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
    0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB, 0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
    0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9, 0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
    0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F, 0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
    0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D, 0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
    0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26, 0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
    0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74, 0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
    0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82, 0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
    0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0, 0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9};

// CRC8 implementation with polynom = 0xBA
static const uint8_t crc8tab_BA[256] = {
    0x00, 0xBA, 0xCE, 0x74, 0x26, 0x9C, 0xE8, 0x52, 0x4C, 0xF6, 0x82, 0x38, 0x6A, 0xD0, 0xA4, 0x1E,
    0x98, 0x22, 0x56, 0xEC, 0xBE, 0x04, 0x70, 0xCA, 0xD4, 0x6E, 0x1A, 0xA0, 0xF2, 0x48, 0x3C, 0x86,
    0x8A, 0x30, 0x44, 0xFE, 0xAC, 0x16, 0x62, 0xD8, 0xC6, 0x7C, 0x08, 0xB2, 0xE0, 0x5A, 0x2E, 0x94,
    0x12, 0xA8, 0xDC, 0x66, 0x34, 0x8E, 0xFA, 0x40, 0x5E, 0xE4, 0x90, 0x2A, 0x78, 0xC2, 0xB6, 0x0C,
    0xAE, 0x14, 0x60, 0xDA, 0x88, 0x32, 0x46, 0xFC, 0xE2, 0x58, 0x2C, 0x96, 0xC4, 0x7E, 0x0A, 0xB0,
    0x36, 0x8C, 0xF8, 0x42, 0x10, 0xAA, 0xDE, 0x64, 0x7A, 0xC0, 0xB4, 0x0E, 0x5C, 0xE6, 0x92, 0x28,
    0x24, 0x9E, 0xEA, 0x50, 0x02, 0xB8, 0xCC, 0x76, 0x68, 0xD2, 0xA6, 0x1C, 0x4E, 0xF4, 0x80, 0x3A,
    0xBC, 0x06, 0x72, 0xC8, 0x9A, 0x20, 0x54, 0xEE, 0xF0, 0x4A, 0x3E, 0x84, 0xD6, 0x6C, 0x18, 0xA2,
    0xE6, 0x5C, 0x28, 0x92, 0xC0, 0x7A, 0x0E, 0xB4, 0xAA, 0x10, 0x64, 0xDE, 0x8C, 0x36, 0x42, 0xF8,
    0x7E, 0xC4, 0xB0, 0x0A, 0x58, 0xE2, 0x96, 0x2C, 0x32, 0x88, 0xFC, 0x46, 0x14, 0xAE, 0xDA, 0x60,
    0x6C, 0xD6, 0xA2, 0x18, 0x4A, 0xF0, 0x84, 0x3E, 0x20, 0x9A, 0xEE, 0x54, 0x06, 0xBC, 0xC8, 0x72,
    0xF4, 0x4E, 0x3A, 0x80, 0xD2, 0x68, 0x1C, 0xA6, 0xB8, 0x02, 0x76, 0xCC, 0x9E, 0x24, 0x50, 0xEA,
    0x48, 0xF2, 0x86, 0x3C, 0x6E, 0xD4, 0xA0, 0x1A, 0x04, 0xBE, 0xCA, 0x70, 0x22, 0x98, 0xEC, 0x56,
    0xD0, 0x6A, 0x1E, 0xA4, 0xF6, 0x4C, 0x38, 0x82, 0x9C, 0x26, 0x52, 0xE8, 0xBA, 0x00, 0x74, 0xCE,
    0xC2, 0x78, 0x0C, 0xB6, 0xE4, 0x5E, 0x2A, 0x90, 0x8E, 0x34, 0x40, 0xFA, 0xA8, 0x12, 0x66, 0xDC,
    0x5A, 0xE0, 0x94, 0x2E, 0x7C, 0xC6, 0xB2, 0x08, 0x16, 0xAC, 0xD8, 0x62, 0x30, 0x8A, 0xFE, 0x44};

static uint8_t crsf_crc(const uint8_t crctab[], const uint8_t *ptr, uint8_t len)
{
  uint8_t crc = 0;
  for (uint8_t i = 0; i < len; i++)
  {
    crc = crctab[crc ^ *ptr++];
  }
  return crc;
}
uint8_t crsf_crc8(const uint8_t *ptr, uint8_t len)
{
  return crsf_crc(crc8tab, ptr, len);
}
uint8_t crsf_crc8_BA(const uint8_t *ptr, uint8_t len)
{
  return crsf_crc(crc8tab_BA, ptr, len);
}


void duplex_set_RX(void)
{
	receiveStat	= 1;
//	UART1_DIR_PIN = 0;
//	RingBuffer_flush();
//		while(UART_IS_RX_READY(UART1))
//			UART_READ(UART1);
////			printf("RxBytes: %d\n", UART_READ(UART1););
			
	
//		UART_EnableInt(UART1, UART_INTEN_RDAIEN_Msk);
//		NVIC_EnableIRQ(UART1_IRQn);
}


void duplex_set_TX(void)
{
	receiveStat = 0;
	UART1_DIR_PIN = 1;
}

// Serial begin
void CRSF_Begin() {
		/* Reset UART module */
		SYS_ResetModule(UART1_RST);
    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART1, SERIAL_BAUDRATE);
		RingBuffer_Begin();
		RingBuffer_flush();
		duplex_set_TX();
	
		/*Enable UART Interrupt*/
		// RDAINEN -> Receive Data Available Interrupt Enable
		// RXTOIEN -> Receive Buffer Time-out Interript Enable
		UART_EnableInt(UART1, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);	
		NVIC_EnableIRQ(UART1_IRQn);

}

void protocol_module_type(module_type_t type)
{
  module_type = type;
};

bool ModulIsELRS(void)
{
	if(MODULE_IS_ELRS)
		return true;
	else
		return false;
}

uint8_t CRSF_GetDeviceStat(void)
{
	uint8_t ret = 0;
	if(crsf_devices[0].address == CRSF_ADDRESS_CRSF_TRANSMITTER)
		bitset(ret, 0);
	else
		bitclear(ret, 0);
	
	if(crsf_devices[1].address == CRSF_ADDRESS_CRSF_RECEIVER)
		bitset(ret, 1);
	else
		bitclear(ret, 1);
		
	return ret;
}

uint32_t get_update_interval()
{
  if (correction == 0)
    return updateInterval;
			
  update = updateInterval + correction;
  update = constrain(update, CRSF_FRAME_PERIOD_MIN, CRSF_FRAME_PERIOD_MAX);
  correction = (int32_t)(correction - (update - updateInterval));
  return update;
}

void sync_crsf(int32_t add_delay)
{
  crsfTime = micros();                        // set current micros
  int32_t offset = (crsfTime - lastCrsfTime); // get dif between pckt send
	
  updated_interval = get_update_interval();
// debug timing
#if defined(DEBUG_SYNC)
  if (updated_interval != 20000)
    printf("%u ; %u ; %i ; %u\n", lastCrsfTime, crsfTime, offset, updated_interval);
#endif

  crsfTime += ((updated_interval + add_delay) - offset); // set current micros
  lastCrsfTime = crsfTime;                               // set time that we send last packet
}

void CRSF_write(uint8_t crsfPacket[], uint8_t size, int32_t add_delay)
{

#if defined(debug)
  if (crsfPacket[2] != TYPE_CHANNELS)
    printf("elrs write 0x%x\n", crsfPacket[2]);
#endif
	duplex_set_TX();
  UART_Write(UART1, crsfPacket, size);
	UART_WAIT_TX_EMPTY(UART1);
	duplex_set_RX();

  // set last time packet send
  sync_crsf(add_delay);
}

void CRSF_PrepareRCPacket(int16_t (*rc_input)[CRSF_MAX_CHANNEL], ChannelCalibration (*channelCalibs)[NUM_ANALOG_CHANNELS])
{
	rcChannels[THROTTLE] = mapJoystickCRSFValues((*rc_input)[THROTTLE],  (*channelCalibs)[THROTTLE].min,
																																			 (*channelCalibs)[THROTTLE].mid,
																																			 (*channelCalibs)[THROTTLE].max,
																																				CRSF_CHANNEL_VALUE_MIN,
																																				CRSF_CHANNEL_VALUE_MID,
																																				CRSF_CHANNEL_VALUE_MAX,
																																				true);
	rcChannels[YAW] = mapJoystickCRSFValues((*rc_input)[YAW],  (*channelCalibs)[YAW].min,
																														 (*channelCalibs)[YAW].mid,
																														 (*channelCalibs)[YAW].max,
																															CRSF_CHANNEL_VALUE_MIN,
																															CRSF_CHANNEL_VALUE_MID,
																															CRSF_CHANNEL_VALUE_MAX,
																															false);
	rcChannels[PITCH] = mapJoystickCRSFValues((*rc_input)[PITCH],  (*channelCalibs)[PITCH].min,
																																 (*channelCalibs)[PITCH].mid,
																																 (*channelCalibs)[PITCH].max,
																																	CRSF_CHANNEL_VALUE_MIN,
																																	CRSF_CHANNEL_VALUE_MID,
																																	CRSF_CHANNEL_VALUE_MAX,
																																	true);
	rcChannels[ROLL] = mapJoystickCRSFValues((*rc_input)[ROLL],  (*channelCalibs)[ROLL].min,
																															 (*channelCalibs)[ROLL].mid,
																															 (*channelCalibs)[ROLL].max,
																																CRSF_CHANNEL_VALUE_MIN,
																																CRSF_CHANNEL_VALUE_MID,
																																CRSF_CHANNEL_VALUE_MAX,
																																false);
	rcChannels[AUX1] = mapJoystickCRSFValues((*rc_input)[AUX1],  (*channelCalibs)[AUX1].min,
																															 (*channelCalibs)[AUX1].mid,
																															 (*channelCalibs)[AUX1].max,
																																CRSF_CHANNEL_VALUE_MIN,
																																CRSF_CHANNEL_VALUE_MID,
																																CRSF_CHANNEL_VALUE_MAX,
																																true);
	rcChannels[AUX2] = mapJoystickCRSFValues((*rc_input)[AUX2],  (*channelCalibs)[AUX2].min,
																															 (*channelCalibs)[AUX2].mid,
																															 (*channelCalibs)[AUX2].max,
																																CRSF_CHANNEL_VALUE_MIN,
																																CRSF_CHANNEL_VALUE_MID,
																																CRSF_CHANNEL_VALUE_MAX,
																																true);
	rcChannels[AUX3] = mapJoystickCRSFValues((*rc_input)[AUX3],  (*channelCalibs)[AUX3].min,
																															 (*channelCalibs)[AUX3].mid,
																															 (*channelCalibs)[AUX3].max,
																																CRSF_CHANNEL_VALUE_MIN,
																																CRSF_CHANNEL_VALUE_MID,
																																CRSF_CHANNEL_VALUE_MAX,
																																true);
	rcChannels[AUX4] = mapJoystickCRSFValues((*rc_input)[AUX4],  (*channelCalibs)[AUX4].min,
																															 (*channelCalibs)[AUX4].mid,
																															 (*channelCalibs)[AUX4].max,
																																CRSF_CHANNEL_VALUE_MIN,
																																CRSF_CHANNEL_VALUE_MID,
																																CRSF_CHANNEL_VALUE_MAX,
																																false);
	rcChannels[AUX5] = mapJoystickCRSFValues((*rc_input)[AUX5], 	0,
																																1,
																																2,
																																CRSF_CHANNEL_VALUE_MIN,
																																CRSF_CHANNEL_VALUE_MID,
																																CRSF_CHANNEL_VALUE_MAX,
																																false);
	rcChannels[AUX6] = map((*rc_input)[AUX6], 										0,
																																1,
																																CRSF_CHANNEL_VALUE_MIN,
																																CRSF_CHANNEL_VALUE_MAX);
	rcChannels[AUX7] = mapJoystickCRSFValues((*rc_input)[AUX7], 	0,
																																1,
																																2,
																																CRSF_CHANNEL_VALUE_MIN,
																																CRSF_CHANNEL_VALUE_MID,
																																CRSF_CHANNEL_VALUE_MAX,
																																false);
	rcChannels[AUX8] = map((*rc_input)[AUX8], 										0,
																																1,
																																CRSF_CHANNEL_VALUE_MIN,
																																CRSF_CHANNEL_VALUE_MAX);				

	rcChannels[THROTTLE] = rcChannels[THROTTLE] + (myTableParams.TrimConfig.trm_throttle * 5);
	rcChannels[YAW] = rcChannels[YAW] + (myTableParams.TrimConfig.trm_yaw * 5);
	rcChannels[PITCH] = rcChannels[PITCH] + (myTableParams.TrimConfig.trm_pitch * 5);
	rcChannels[ROLL] = rcChannels[ROLL] + (myTableParams.TrimConfig.trm_roll * 5);
	
}

// prepare data packet
void CRSF_SendChannels(void)
{

  uint8_t crsfPacket[CRSF_PACKET_SIZE];
	
  // packet[0] = UART_SYNC; //Header
  crsfPacket[0] = CRSF_ADDRESS_CRSF_TRANSMITTER; // Header
  crsfPacket[1] = 24;          // length of type (24) + payload + crc
  crsfPacket[2] = CRSF_FRAMETYPE_RC_CHANNELS_PACKED;
  crsfPacket[3] = (uint8_t)(rcChannels[0] & 0x07FF);
  crsfPacket[4] = (uint8_t)((rcChannels[0] & 0x07FF) >> 8 | (rcChannels[1] & 0x07FF) << 3);
  crsfPacket[5] = (uint8_t)((rcChannels[1] & 0x07FF) >> 5 | (rcChannels[2] & 0x07FF) << 6);
  crsfPacket[6] = (uint8_t)((rcChannels[2] & 0x07FF) >> 2);
  crsfPacket[7] = (uint8_t)((rcChannels[2] & 0x07FF) >> 10 | (rcChannels[3] & 0x07FF) << 1);
  crsfPacket[8] = (uint8_t)((rcChannels[3] & 0x07FF) >> 7 | (rcChannels[4] & 0x07FF) << 4);
  crsfPacket[9] = (uint8_t)((rcChannels[4] & 0x07FF) >> 4 | (rcChannels[5] & 0x07FF) << 7);
  crsfPacket[10] = (uint8_t)((rcChannels[5] & 0x07FF) >> 1);
  crsfPacket[11] = (uint8_t)((rcChannels[5] & 0x07FF) >> 9 | (rcChannels[6] & 0x07FF) << 2);
  crsfPacket[12] = (uint8_t)((rcChannels[6] & 0x07FF) >> 6 | (rcChannels[7] & 0x07FF) << 5);
  crsfPacket[13] = (uint8_t)((rcChannels[7] & 0x07FF) >> 3);
  crsfPacket[14] = (uint8_t)((rcChannels[8] & 0x07FF));
  crsfPacket[15] = (uint8_t)((rcChannels[8] & 0x07FF) >> 8 | (rcChannels[9] & 0x07FF) << 3);
  crsfPacket[16] = (uint8_t)((rcChannels[9] & 0x07FF) >> 5 | (rcChannels[10] & 0x07FF) << 6);
  crsfPacket[17] = (uint8_t)((rcChannels[10] & 0x07FF) >> 2);
  crsfPacket[18] = (uint8_t)((rcChannels[10] & 0x07FF) >> 10 | (rcChannels[11] & 0x07FF) << 1);
  crsfPacket[19] = (uint8_t)((rcChannels[11] & 0x07FF) >> 7 | (rcChannels[12] & 0x07FF) << 4);
  crsfPacket[20] = (uint8_t)((rcChannels[12] & 0x07FF) >> 4 | (rcChannels[13] & 0x07FF) << 7);
  crsfPacket[21] = (uint8_t)((rcChannels[13] & 0x07FF) >> 1);
  crsfPacket[22] = (uint8_t)((rcChannels[13] & 0x07FF) >> 9 | (rcChannels[14] & 0x07FF) << 2);
  crsfPacket[23] = (uint8_t)((rcChannels[14] & 0x07FF) >> 6 | (rcChannels[15] & 0x07FF) << 5);
  crsfPacket[24] = (uint8_t)((rcChannels[15] & 0x07FF) >> 3);

  crsfPacket[25] = crsf_crc8(&crsfPacket[2], CRSF_PACKET_SIZE - 3); // CRC

  CRSF_write(crsfPacket, CRSF_PACKET_SIZE, 0);
}

void CRSF_changeParam(uint8_t n_param, uint8_t n_chunk)
{
	printf("Change Param: %d, chunk: %d\n", n_param, n_chunk);
	next_chunk = 0;
  uint8_t packetCmd[8];

  packetCmd[0] = CRSF_ADDRESS_CRSF_TRANSMITTER;
  packetCmd[1] = 6; // length of Command (4) + payload + crc
  packetCmd[2] = CRSF_FRAMETYPE_PARAMETER_WRITE;
  packetCmd[3] = CRSF_ADDRESS_CRSF_TRANSMITTER;
  packetCmd[4] = CRSF_ADDRESS_RADIO_TRANSMITTER;
  packetCmd[5] = n_param;
  packetCmd[6] = n_chunk;
  packetCmd[7] = crsf_crc8(&packetCmd[2], packetCmd[1] - 1);

  CRSF_write(packetCmd, 8, 20000);
	
	Delay(500);
	
	n_chunk = 0;
  CRSF_read_param(n_param, n_chunk, CRSF_ADDRESS_CRSF_TRANSMITTER);
}


/* Request parameter info from known device

n_param, which is the number of the parameter to be read.
n_chunk, which is the chunk number of the parameter.
target, which is the target device or system from which the parameter is to be read.
The function first constructs a packet in the packetCmd array with the following fields:

The device address
The packet length
The packet type
The target device or system
The address of the sender
The parameter number
The chunk number
The packet's cyclic redundancy check (CRC) value

It then sends the packet using the CRSF_write function, passing in the packetCmd array and its length as arguments. 
*/
void CRSF_read_param(uint8_t n_param, uint8_t n_chunk, uint8_t target)
{
  // printf("read param\n");
	printf("Read Param: %d, chunk: %d\n", n_param, n_chunk);

  uint8_t packetCmd[8];

  packetCmd[0] = CRSF_ADDRESS_CRSF_TRANSMITTER;
  packetCmd[1] = 6; // length of Command (4) + payload + crc
  packetCmd[2] = CRSF_FRAMETYPE_PARAMETER_READ;
  packetCmd[3] = target;
  packetCmd[4] = CRSF_ADDRESS_RADIO_TRANSMITTER;
  packetCmd[5] = n_param;
  packetCmd[6] = n_chunk;
  packetCmd[7] = crsf_crc8(&packetCmd[2], packetCmd[1] - 1);

  CRSF_write(packetCmd, 8, 20000);
}


void CRSF_Broadcast_Ping(void)
{
  uint8_t packetCmd[6];

  packetCmd[0] = CRSF_ADDRESS_CRSF_TRANSMITTER;
  packetCmd[1] = 4; // length of Command (4) + payload + crc
  packetCmd[2] = CRSF_FRAMETYPE_DEVICE_PING;
  packetCmd[3] = CRSF_ADDRESS_BROADCAST;
  packetCmd[4] = CRSF_ADDRESS_RADIO_TRANSMITTER;
  packetCmd[5] = crsf_crc8(&packetCmd[2], packetCmd[1] - 1);

  CRSF_write(packetCmd, 6, 0);
}

// request ELRS_info message
void CRSF_Get_Elrs_Info(uint8_t target)
{
  uint8_t packetCmd[8];

  packetCmd[0] = CRSF_ADDRESS_CRSF_TRANSMITTER; // target;
  packetCmd[1] = 6;            // length of Command (4) + payload + crc
  packetCmd[2] = CRSF_FRAMETYPE_PARAMETER_WRITE;
  packetCmd[3] = target;
  packetCmd[4] = CRSF_ADDRESS_RADIO_TRANSMITTER;
  packetCmd[5] = 0;
  packetCmd[6] = 0;
  packetCmd[7] = crsf_crc8(&packetCmd[2], packetCmd[1] - 1);

  CRSF_write(packetCmd, 8, 0);
}


void CRSF_send_id(uint8_t modelId)
{

  uint8_t packetCmd[LinkStatisticsFrameLength];

  packetCmd[0] = CRSF_ADDRESS_CRSF_TRANSMITTER;
  packetCmd[1] = 8;
  packetCmd[2] = CRSF_FRAMETYPE_COMMAND;
  packetCmd[3] = CRSF_ADDRESS_CRSF_TRANSMITTER;
  packetCmd[4] = CRSF_ADDRESS_RADIO_TRANSMITTER;
  packetCmd[5] = SUBCOMMAND_CRSF;
  packetCmd[6] = COMMAND_MODEL_SELECT_ID;
  packetCmd[7] = modelId; // modelID TODO
  packetCmd[8] = crsf_crc8_BA(&packetCmd[2], packetCmd[1] - 2);
  packetCmd[9] = crsf_crc8(&packetCmd[2], packetCmd[1] - 1);

  CRSF_write(packetCmd, LinkStatisticsFrameLength, 0);
}

void check_link_state(uint32_t currentMicros)
{
    uint8_t tmp = LinkStatistics.rf_Mode;
	
    if (txConected > 0)
    {
        if ((int)local_info.good_pkts == 0)
        {
            CRSF_Get_Elrs_Info(CRSF_ADDRESS_CRSF_TRANSMITTER);
        }
        else if ((int)local_info.good_pkts != (int)rates[tmp] && rxConected > 0)
        {
            CRSF_Get_Elrs_Info(CRSF_ADDRESS_CRSF_TRANSMITTER);
        }

        if (rxConected == 0)
        {
            crsf_devices[1].address = 0;
            strlcpy(crsf_devices[1].name, (const char *)"", CRSF_MAX_NAME_LEN);
//            printf("no rx found\n");
        }
        else
        {
        
            if (crsf_devices[1].address == 0)
            {

                CRSF_Broadcast_Ping();
            }
            else
            {
//								printf("read rx info\n"); RX  param özelliginde aktif edilecek.
////								next_param = 1;
////								next_chunk = 0;
////								CRSF_read_param(next_param, next_chunk, ELRS_RX_ADDRESS);
            }
        }
    }
    else
    {
        crsf_devices[0].address = 0;
        strlcpy(crsf_devices[0].name, (const char *)"", CRSF_MAX_NAME_LEN);
        local_info.good_pkts = 0;
				printf("no tx module found\n");
#if defined(debug)
        printf("no tx module found\n");
#endif
    }
    tickTime = currentMicros + tickInterval;
    rxConected = 0;
    txConected = 0;
}


void parse_elrs_info(uint8_t *buffer)
{
  local_info.bad_pkts = buffer[3];                     // bad packet rate (should be 0)
  local_info.good_pkts = (buffer[4] << 8) + buffer[5]; // good packet rate (configured rate)

  // flags bit 0 indicates receiver connected
  // other bits indicate errors - error text in flags_info
  local_info.flags = buffer[6];
  strlcpy(local_info.flag_info, (const char *)&buffer[7], CRSF_MAX_NAME_LEN); // null-terminated text of flags

  local_info.update = elrs_info.update;
  if (memcmp((void *)&elrs_info, (void *)&local_info, sizeof(elrs_info_t) - CRSF_MAX_NAME_LEN))
  {
    if (local_info.flag_info[0] && strncmp(local_info.flag_info, elrs_info.flag_info, CRSF_MAX_NAME_LEN))
    {
      printf("error: %s\n", local_info.flag_info);
      // example: error: Model Mismatch
      // error: [ ! Armed ! ]
    }

    memcpy((void *)&elrs_info, (void *)&local_info, sizeof(elrs_info_t) - CRSF_MAX_NAME_LEN);
    elrs_info.update++;
  }

// example bad_pckts : good_pckts ; flag ; flag_info ; info_update ;
//  0 : 100 ; 5 ; Model Mismatch ; 0
//  0 : 200 ; 8 ; [ ! Armed ! ] ; 0
//   printf("%u : %u ; %u ; %s ; %u\n ",local_info.bad_pkts,local_info.good_pkts,local_info.flags,local_info.flag_info,local_info.update);
}


uint8_t getCrossfireTelemetryValue(uint8_t index, int32_t *value, uint8_t len)
{
  uint8_t result = 0;
  uint8_t *byte = &SerialInBuffer[index];
  *value = (*byte & 0x80) ? -1 : 0;
  for (int i = 0; i < len; i++)
  {
    *value <<= 8;
    if (*byte != 0xff)
      result = 1;
    *value += *byte++;
  }
  return result;
}

//uint8_t count_params_loaded(uint8_t index)
//{

//  int i;
//  for (i = 0; i < crsf_devices[index].number_of_params; i++)
//  {
//    if (crsf_devices[index].address == CRSF_ADDRESS_CRSF_TRANSMITTER)
//      if (menuItems[i].id == 0)
//        break;
//    if (crsf_devices[index].address == CRSF_ADDRESS_CRSF_RECEIVER)
//      if (rx_p[i].id == 0)
//        break;
//  }
//  return i;
//}

void CRSF_Update(void)
{
		uint32_t currentMicros = micros();
		if (currentMicros > tickTime)
		{
			if(next_param == 0)
				check_link_state(currentMicros);
			else
			{
				if(command_stat)
					CRSF_changeParam(next_param, next_chunk);
				else
					CRSF_read_param(next_param, next_chunk, CRSF_ADDRESS_CRSF_TRANSMITTER);

				tickTime = currentMicros + 200000;
			}			
		}
		

		if (currentMicros >= crsfTime)
		{

			CRSF_PrepareRCPacket(&myRCvalues.channelValues, &myTableParams.channels);
			// send crsf channels packet
			CRSF_SendChannels();
			// start receiving at end of each crsf cycle or cmd sent
			serialEvent();

	} // end button filter to send commands
}


uint8_t ParamsLoaded(uint8_t addr)
{
	uint8_t i, a;
	
	if(addr == CRSF_ADDRESS_CRSF_TRANSMITTER)	//Transmitter or receiver params?
		a = 0;
	else
		a = 1;
		
	for(i = 1; i <= crsf_devices[a].number_of_params+1; ++i)
	{
		if(crsf_params[i].id == 0)
			return i;
	}
}


uint32_t parse_u32(const uint8_t *buffer)
{
  return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
}

void parse_device(uint8_t *buffer, crsf_device_t *device)
{
  buffer += 2;
  device->address = (uint8_t)*buffer++;
  strlcpy(device->name, (const char *)buffer, CRSF_MAX_NAME_LEN);
  buffer += strlen((const char *)buffer) + 1;
  device->serial_number = parse_u32(buffer);
  buffer += 4;
  device->hardware_id = parse_u32(buffer);
  buffer += 4;
  device->firmware_id = parse_u32(buffer);
  buffer += 4;
  device->number_of_params = *buffer;
  buffer += 1;
  device->params_version = *buffer;
  if (device->address == CRSF_ADDRESS_CRSF_TRANSMITTER)
  {
    if (device->serial_number == 0x454C5253)
    {
      printf("Module type: elrs\n");
      protocol_module_type(MODULE_ELRS);
    }
    else
    {
      printf("Module type: not elrs\n");
      protocol_module_type(MODULE_OTHER);
    }
  }
   printf("Device Details:\n \
	  Name:%s\n \
		Addr: 0x%X\n \
		Params: %u\n \
		Param Version: %u\n \
		Serial: %u\n \
		FW Id: %u\n \
		HW Id: %u\n",
		device->name,
		device->address,
		device->number_of_params,
		device->params_version,
		device->serial_number,
		device->firmware_id,
		device->hardware_id); 
}


void add_device(uint8_t *buffer)
{

  for (int i = 0; i < CRSF_MAX_DEVICES; i++)
  {
    if (crsf_devices[i].address == buffer[2] //  device already in table
        || crsf_devices[i].address == 0      //  not found, add to table
        || crsf_devices[i].address == CRSF_ADDRESS_RADIO_TRANSMITTER)
    { //  replace deviation device if necessary

#if defined(debug)
      printf("device pong: 0x%x\n", buffer[2]);
#endif
      parse_device(buffer, &crsf_devices[i]);
      break;
    }
  }
  //  no new device added if no more space in table
}

void parse_bytes(enum data_type type, char **buffer, char *dest)
{
  switch (type)
  {
  case UINT8:
    *(u8 *)dest = (u8)(*buffer)[0];
    *buffer += 1;
    break;
  case INT8:
    *(s8 *)dest = (s8)(*buffer)[0];
    *buffer += 1;
    break;
  case UINT16:
    *(u16 *)dest = (u16)(((*buffer)[0] << 8) | (*buffer)[1]);
    *buffer += 2;
    break;
  case INT16:
    *(s16 *)dest = (s16)(((*buffer)[0] << 8) | (*buffer)[1]);
    *buffer += 2;
    break;
  case FLOAT:
    *(s32 *)dest = (s32)(((*buffer)[0] << 24) | ((*buffer)[1] << 16) | ((*buffer)[2] << 8) | (*buffer)[3]);
    *buffer += 4;
    break;
  default:
    break;
  }
}


void parse_param(uint8_t *buffer, uint8_t num_bytes)
{
		u8 param_id = buffer[3];
		char *param_values;
		
		if(next_chunk > 0)
		{
			if(next_chunk != (crsf_params[param_id].chunk - buffer[4]))
				return;
			
				
			memcpy(param_buff+param_ptr, buffer+5, num_bytes-5);
			param_ptr += num_bytes-5;
		}
		else
		{
			
			crsf_params[param_id].device = buffer[2];
			crsf_params[param_id].chunk = buffer[4];
			crsf_params[param_id].parent = buffer[5];
			crsf_params[param_id].type = buffer[6];
			
			memset(param_buff, 0x00, sizeof(param_buff));
			memcpy(param_buff+param_ptr, (buffer+7), num_bytes-5);
			param_ptr += num_bytes-7;
		}
		
		
		
		if(next_chunk < crsf_params[param_id].chunk)
		{
			next_chunk++;
			return;
		}
		
		if(next_chunk != (crsf_params[param_id].chunk - buffer[4]))
			return;
		
				
		paramUpdate = crsf_params[param_id].id == param_id;
	  crsf_params[param_id].id = buffer[3];

	
   if(!paramUpdate)
    {
//      crsf_params[param_id].hidden = *recv_param_ptr++ & 0x80;
      crsf_params[param_id].name = (char *)malloc(strlen(param_buff) + 1);
//      strncpy(crsf_params[param_id].name, (const char *)recv_param_ptr, strlen(recv_param_ptr) + 1);
			strncpy(crsf_params[param_id].name, (const char *)param_buff, strlen(param_buff) + 1);
//      recv_param_ptr += strlen(recv_param_ptr) + 1;
    }
    else
    {
//      if (crsf_params[param_id].hidden != (*recv_param_ptr & 0x80))
//            params_loaded = 0;   // if item becomes hidden others may also, so reload all params
//        crsf_params[param_id].hidden = *recv_param_ptr++ & 0x80;
//        recv_param_ptr += strlen(recv_param_ptr) + 1;
    }
		
    u8 count;
		u8 len;
		u16 start;
		u16 stop;
		
    switch(crsf_params[param_id].type)
    {

       case UINT8:
       case INT8:
       case UINT16:
       case INT16:
       case FLOAT:
//				 printf("FLOAT Type %s\n", crsf_params[param_id].name);
//        parse_bytes(crsf_params[param_id].type, &recv_param_ptr, (char *)&crsf_params[param_id].value);
//        parse_bytes(crsf_params[param_id].type, &recv_param_ptr, (char *)&crsf_params[param_id].min_value);
//        parse_bytes(crsf_params[param_id].type, &recv_param_ptr, (char *)&crsf_params[param_id].max_value);
//        parse_bytes(crsf_params[param_id].type, &recv_param_ptr, (char *)&crsf_params[param_id].default_value);
//        if (crsf_params[param_id].type == FLOAT) {
//            parse_bytes(UINT8, &recv_param_ptr, (char *)&crsf_params[param_id].u.point);
//            parse_bytes(FLOAT, &recv_param_ptr, (char *)&crsf_params[param_id].step);
//        } else if (*recv_param_ptr) {
//            const u8 length = strlen(recv_param_ptr) + 1;
//            if (!paramUpdate) crsf_params[param_id].s.unit = malloc(length);
//            strncpy(crsf_params[param_id].s.unit, (const char *)recv_param_ptr, length);
//        }
       break;
			 
       case TEXT_SELECTION:
						start = strlen(param_buff);
						stop = 0;
            count = 0;
            for (u8 i = 0; i< param_ptr - (start + 7); i++)
            {
							if (param_buff[i+start] == ';')
							{
								len = (i - stop);
								strlcpy(crsf_params[param_id].options[count], (const char *)(param_buff + (start + stop + 1)), len);
								stop = i;
								count += 1;
							}
            }

						len = strlen(param_buff + (start + stop + 1));
            strlcpy(crsf_params[param_id].options[count], (const char *)(param_buff + (start + stop + 1)), len + 1);		

						start = start + stop + 1 + len + 1;
						
//						printf("Field Value: %02X, %c\n", param_buff[start + 0], param_buff[start + 0]);
//						printf("Min Value: %02X, %c\n", param_buff[start + 1], param_buff[start + 1]);
//						printf("Max Value: %02X, %c\n", param_buff[start + 2], param_buff[start + 2]);
//						printf("Default Value: %02X, %c\n", param_buff[start + 3], param_buff[start + 3]);
						
						if(param_buff[start + 4] == ' ')
						{
							len = strlen(param_buff + start + 5);
							strlcpy(crsf_params[param_id].value, (const char *)param_buff+start + 5, len + 1);
//							for(uint8_t i = 0; i<len; i++)
//								printf("Param: %d\t%02X\t%c\n", param_id, param_buff[start + 5 + i], param_buff[start + 5 + i]);
						}
					
						

						
//						printf("\n\n\n");

            crsf_params[param_id].count = count;
						
						char *values = (char * )malloc(5);
						memcpy(values, param_buff + start, 4);

            parse_bytes(UINT8,  (char **)&values, (char *)&crsf_params[param_id].status);
            parse_bytes(UINT8,  (char **)&values, (char *)&crsf_params[param_id].min_value);
            parse_bytes(UINT8,  (char **)&values, (char *)&crsf_params[param_id].max_value); // don't use incorrect parameter->max_value
            parse_bytes(UINT8,  (char **)&values, (char *)&crsf_params[param_id].default_value);
						
						free(values);
        break;

        case INFO:
					start = strlen(param_buff) + 1;
					len = strlen(param_buff+ start) + 1;
					strlcpy(crsf_params[param_id].value, (const char *)param_buff+start, len);
				break;

        case STRING:
//        {
//            const char *value;
//            value = recv_param_ptr;
//            recv_param_ptr += strlen(value) + 1;
//            parse_bytes(UINT8, &recv_param_ptr, (char *)&crsf_params[param_id].u.string_max_len);

//            // No string re-sizing so allocate max length for value
//            if (!paramUpdate)crsf_params[param_id].value = malloc(crsf_params[param_id].u.string_max_len+1);
//            strncpy(crsf_params[param_id].value, value, crsf_params[param_id].u.string_max_len+1);
//        }
				break;

        case COMMAND:
				{
					start = strlen(param_buff) + 1;
					len = strlen(param_buff+ start) + 1;

					char *values = (char * )malloc(3);
					memcpy(values, param_buff + start, 2);
          parse_bytes(UINT8, &values, (char *)&crsf_params[param_id].u.status);
          parse_bytes(UINT8, &values, (char *)&crsf_params[param_id].timeout);

          strlcpy(crsf_params[param_id].s.info, (const char *)param_buff+start+2, len);
					
//					printf("%s\n%s\n", crsf_params[param_id].name, crsf_params[param_id].s.info);

          command.time = 0;
          switch (crsf_params[param_id].u.status) {
          case PROGRESS:
              command.time = millis();
              // FALLTHROUGH
          case CONFIRMATION_NEEDED:
              command.dialog = 1;
              break;
          case READY:
              command.dialog = 2;
              break;
          }
				}
        break;
				
        case FOLDER:
//					printf("%s\n", crsf_params[param_id].name);
//          printf("[i] Folder Parameter does not support right now:/\n");
//          return;
        break;
				
        case OUT_OF_RANGE:
					break;
				
        default:
            break;
    }


    // printf("\rDevice: %X\n \
    //         \rId: %X\n    \
    //         \rParent: %d\n \
    //         \rType: %X\n \
    //         \rName: %s\n \
    //         \rValue: %s\n\n",
    //         crsf_params[param_id].device,
    //         crsf_params[param_id].id,
    //         crsf_params[param_id].parent,
    //         crsf_params[param_id].type,
    //         crsf_params[param_id].name,
    //         crsf_params[param_id].status);
//		printf("\n\n");
//		for(uint8_t i = 0; i<param_ptr; i++)
//			printf("%02X ", param_buff[i]);
//			
//		printf("\n\n");
		
		next_chunk = 0;
    param_ptr = 0;
        

//    for(int i = 0; i<=crsf_params[param_id].count; i++)
//    {
//        printf("%s\n", crsf_params[param_id].options[i]);
//			  printf("%s\n", crsf_params[param_id].value);
//    }
		
		
//		if(next_param == crsf_devices[0].number_of_params)
//			while(1);


			
		next_param = ParamsLoaded(buffer[2]);
		
}

void CRSF_serial_rcv(uint8_t *buffer, uint8_t num_bytes)
{

  if ((buffer[0] != CRSF_FRAMETYPE_RADIO_ID) && (buffer[0] != CRSF_FRAMETYPE_LINK_STATISTICS))
  {
//     printf("CRSF FRAMETYPE: 0x%x : L:%u : \n",buffer[0],num_bytes);
  }
  else
  {
    if (buffer[0] != CRSF_FRAMETYPE_RADIO_ID)
    {
      if (buffer[0] == CRSF_FRAMETYPE_LINK_STATISTICS)
        rxConected++;
    }
  }
  switch (buffer[0])
  {
  case CRSF_FRAMETYPE_DEVICE_INFO:
#if !defined(debug)
    printf("DEVICE_INFO\n");
#endif
    add_device(buffer);

    break;

  case CRSF_FRAMETYPE_ELRS_STATUS:
		parse_elrs_info(buffer);
    break;

  case CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY:
#if defined(debug)
    dbout.printf("PARAMETER_SETTINGS_ENTRY\n");
#endif
//		printf("PARAMETER_SETTINGS_ENTRY\n");
//		parse_param(param2, sizeof(param2));
    parse_param(buffer, num_bytes);
    break;

  default:
    break;
  }
}

void serialEvent(void)
{

  while (RingBuffer_available() > 0)
  {		
    if (CRSFframeActive == false)
    {

      unsigned char inChar = RingBuffer_read();
			packet_finish = false;

      if (inChar == CRSF_ADDRESS_RADIO_TRANSMITTER)
      {
        // we got sync, reset write pointer
        SerialInPacketPtr = 0;
        SerialInPacketLen = 0;
        CRSFframeActive = true;
        SerialInBuffer[SerialInPacketPtr] = inChar;
        SerialInPacketPtr++;
      }
    }
    else // frame is active so we do the processing
    {
      // first if things have gone wrong //
      if (SerialInPacketPtr > CRSF_MAX_PACKET_LEN - 1)
      {
        // we reached the maximum allowable packet length, so start again because shit fucked up hey.
        SerialInPacketPtr = 0;
        SerialInPacketLen = 0;
        CRSFframeActive = false;
        printf("bad packet len\n");
        return;
      }
      // special case where we save the expected pkt len to buffer //
      if (SerialInPacketPtr == 1)
      {
        unsigned char const inChar = RingBuffer_read();

        if (inChar <= CRSF_MAX_PACKET_LEN)
        {
          SerialInPacketLen = inChar;
          SerialInBuffer[SerialInPacketPtr] = inChar;
          SerialInPacketPtr++;
        }
        else
        {
          SerialInPacketPtr = 0;
          SerialInPacketLen = 0;
          CRSFframeActive = false;
          return;
        }
      }

      int toRead = (SerialInPacketLen + 2) - SerialInPacketPtr;
			int count = toRead;
			for(uint8_t i = 0; i<count; i++)
				SerialInBuffer[SerialInPacketPtr+i] = RingBuffer_read();
				
      SerialInPacketPtr += count;

      if (SerialInPacketPtr >= (SerialInPacketLen + 2)) // plus 2 because the packlen is referenced from the start of the 'type' flag, IE there are an extra 2 bytes.
      {
        char CalculatedCRC = crsf_crc8(SerialInBuffer + 2, SerialInPacketPtr - 3);
				temp = CalculatedCRC;
        if (CalculatedCRC == SerialInBuffer[SerialInPacketPtr - 1])
        {
          txConected++;

          int32_t value;
          uint8_t id = SerialInBuffer[2];
          CRSF_serial_rcv(SerialInBuffer + 2, SerialInBuffer[1] - 1);
					packet_finish = true;

          if (id == CRSF_FRAMETYPE_BATTERY_SENSOR)
          {
            if (getCrossfireTelemetryValue(3, &value, 2))
            {
							Battery.voltage = value;
            }
          }
          if (id == CRSF_FRAMETYPE_RADIO_ID)
          {
            // print("radio id");
            if (SerialInBuffer[3] == CRSF_ADDRESS_RADIO_TRANSMITTER // 0xEA - radio address
                && SerialInBuffer[5] == CRSF_FRAMETYPE_OPENTX_SYNC  // 0x10 - timing correction frame
            )
            {
              if (getCrossfireTelemetryValue(6, (int32_t *)&updateInterval, 4) &&
                  getCrossfireTelemetryValue(10, (int32_t *)&correction, 4))
              {
                // values are in 10th of micro-seconds
                updateInterval /= 10;
                correction /= 10;
                if (correction >= 0)
                  correction %= updateInterval;
                else
                  correction = -((-correction) % updateInterval);
              }
            }
            if (MODULE_IS_UNKNOWN)
            {
            #if defined(debug)
                          printf("Ping...\n");
                          // protocol_module_type(module_type);
            #endif
              CRSF_Broadcast_Ping();
            }
          }

          if (id == CRSF_FRAMETYPE_LINK_STATISTICS)
          {
            // printf("CRSF_FRAMETYPE_LINK_STATISTICS...%i\n", id);

            if (getCrossfireTelemetryValue(2 + TELEM_CRSF_RX_RSSI1, &value, 1))
            {
              LinkStatistics.uplink_RSSI_1 = value;
            }
            if (getCrossfireTelemetryValue(2 + TELEM_CRSF_RX_RSSI2, &value, 1))
            {
              LinkStatistics.uplink_RSSI_2 = value;
            }
            if (getCrossfireTelemetryValue(2 + TELEM_CRSF_RX_QUALITY, &value, 1))
            {
              LinkStatistics.uplink_Link_quality = value;
            }
            if (getCrossfireTelemetryValue(2 + TELEM_CRSF_RF_MODE, &value, 1))
            {
              LinkStatistics.rf_Mode = value;
            }
            if (getCrossfireTelemetryValue(2 + TELEM_CRSF_TX_POWER, &value, 1))
            {
							//  list =  {'10 mW', '25 mW', '50 mW', '100 mW', '250 mW', '500 mW', '1000 mW', '2000 mW'},
              static const int32_t power_values[] = {0, 10, 25, 100, 500, 1000, 2000, 250, 50};
              // if ((int8_t)value >= (sizeof power_values / sizeof (int32_t)))
              //   continue;
              value = power_values[value];
              LinkStatistics.uplink_TX_Power = value;
            }
            if (getCrossfireTelemetryValue(2 + TELEM_CRSF_TX_RSSI, &value, 1))
            {
              LinkStatistics.downlink_RSSI = value;
            }
            if (getCrossfireTelemetryValue(2 + TELEM_CRSF_TX_QUALITY, &value, 1))
            {
              LinkStatistics.downlink_Link_quality = value;
            }
          
          }
          if (id == CRSF_FRAMETYPE_GPS)
          {
             if (getCrossfireTelemetryValue(3, &value, 4))
            {
              GPS.gps_lat = value;
            }
            if (getCrossfireTelemetryValue(7, &value, 4))
            {
              GPS.gps_long = value; 
            }
            if (getCrossfireTelemetryValue(17, &value, 1))
            {
              GPS.gps_sat = value;
            }
          }


#if defined(DEBUG_PACKETS)
          // output packets to serial for debug
          for (int i = 0; i <= 15; i++)
          {
            dbout.write(SerialInBuffer[i]);
          }
#endif
        }
        else
        {
//          printf("UART CRC failure\n");
//					printf("Calculated CRC: 0x%X, ReceivedCRC: 0x%X\n", CalculatedCRC, SerialInBuffer[SerialInPacketPtr-1]);
//					
//					for(uint8_t i = 0; i<SerialInPacketPtr; i++)
//						printf("0x%X\n", SerialInBuffer[i]);
					
          // cleanup input buffer
          RingBuffer_flush();
          // BadPktsCount++;
        }
        CRSFframeActive = false;
        SerialInPacketPtr = 0;
        SerialInPacketLen = 0;
      }
    }
  }
}


void UART1_IRQHandler(void) {
	uint8_t u8InChar = 0xFF;
	uint32_t intStatus = UART1->INTSTS;
	
//	if((intStatus & UART_INTSTS_RDAINT_Msk) || (intStatus & UART_INTSTS_RXTOINT_Msk)) {
	if((intStatus & UART_INTSTS_RDAINT_Msk) || (intStatus & UART_INTEN_RXTOIEN_Msk)){
		
		while(UART_IS_RX_READY(UART1))
    {
			u8InChar = UART_READ(UART1);
			if(receiveStat == 1)
			{
				__push(&RingBuffer,u8InChar);
			}
		}
	}
}

#ifndef EEPROM_H
#define EEPROM_H

#include "M480.h"
#include "fmc.h"
#include "clk.h"
#include "sys.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"

static const char VERSION[] = "1.0.0";


#define DATA_FLASH_BASE_ADDR				0x0007E000UL // 8 KB
#define DATA_FLASH_END_ADDR					0x0007FFFFUL

#define DATA_FLASH_ENABLE_BIT				0x01
#define DATA_FLASH_EEPROM_BASE      0x7E000  /* Configure Data Flash base address for EEPROM */
#define DATA_FLASH_EEPROM_END       0x7FFFF /* Configure Data Flash end address for EEPROM */
#define DATA_FLASH_EEPROM_SIZE      (DATA_FLASH_EEPROM_END - DATA_FLASH_EEPROM_BASE)    /* Data Flash size for EEPROM */
#define EEPROM_BASE                 (DATA_FLASH_EEPROM_BASE)  /* Set EEPROM base address */
#define EEPROM_SIZE                 (DATA_FLASH_EEPROM_SIZE)  /* Set EEPROM size */


#define COPY_SIZE										1024


typedef struct {
    uint16_t min;  // Minimum deger
		uint16_t mid;	 // Orta deger
    uint16_t max;  // Maksimum deger
} ChannelCalibration;

typedef struct {
		char name[20];
		uint8_t calibStat;
} MainConfig_t;

typedef struct {
		int16_t trm_throttle;
		int16_t trm_yaw;
		int16_t trm_pitch;
		int16_t trm_roll;
} TrimConfig_t;


#define CRSF_MAX_CHANNEL 		16
#define NUM_ANALOG_CHANNELS 8
#define NUM_CHANNELS			  12

typedef struct {
    int16_t channelValues[CRSF_MAX_CHANNEL]; // Kanal degerlerini depolamak için dizi
} RCValues;

typedef struct {
    ChannelCalibration channels[NUM_ANALOG_CHANNELS];
		MainConfig_t MainConfig;
		TrimConfig_t TrimConfig;
} TableParams;



typedef enum {
	FMC_OK = 0,
	FMC_ERR = -1
} FMC_RESP;


FMC_RESP setDataFlashBase(uint32_t DFBA);
FMC_RESP TableWriteToFlash(uint32_t addr, TableParams params);
FMC_RESP TableReadToFlash(uint32_t addr, TableParams *params);

#endif
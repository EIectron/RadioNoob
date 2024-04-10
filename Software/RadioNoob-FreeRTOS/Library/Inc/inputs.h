#ifndef INPUT_H
#define INPUT_H

#include "common.h"
#include "stdbool.h"
#include "delay.h"
#include "eeprom.h"
#include "sound.h"


#define TRM_MIN						(-25)
#define TRM_MAX						25
#define TRM_VALUE					1
#define trm_button_delay	100
#define btn_vib_delay	20
#define SWITCH_HOMES	0x6DB


volatile static uint32_t g_u32AdcIntFlag, g_u32COVNUMFlag = 0;

enum chan_order
{
    ROLL = 0,
    PITCH,
    THROTTLE,
    YAW,
    AUX1, // (CH5)  ARM switch for Expresslrs
    AUX2, // (CH6)  angel / airmode change
    AUX3, // (CH7)  flip after crash
    AUX4, // (CH8)
    AUX5, // (CH9)
    AUX6, // (CH10)
    AUX7, // (CH11)
    AUX8, // (CH12)
};

enum adc_order
{
    ADC_ROLL_CH 		= 2,
		ADC_THROTTLE_CH = 3,
		ADC_PITCH_CH 		= 4,
		ADC_YAW_CH 			= 5,
		ADC_POTL_CH 		= 6,
		ADC_POTR_CH		  = 7,
		ADC_VRd_CH 			= 8,
		ADC_CRe_CH 			= 9,
		ADC_IBat_CH 		= 10,
		ADC_VBat_CH 		= 11
};

enum sw_names{
	SWA = 0,
	SWB = 1,
	SWC = 2,
	//SWCdwn = 3
	SWD = 4,
	SWE = 5,
	//SWEdwn = 6
	SWF = 7,
	SWG = 8,
	//SWGdwn = 9
	SWH = 10,
	SW = 11
};



//const char *chan_names[];
static const char *chan_names[] = {
    "ROLL",
    "PITCH",
    "THROTTLE",
    "YAW",
    "AUX1",
    "AUX2",
    "AUX3",
    "AUX4",
    "AUX5",
    "AUX6",
    "AUX7",
    "AUX8"
};


uint32_t GetSW(uint8_t sw);
void updateButtons(void);
void gpioMixer(void);
void checkMinMaxValues(uint32_t *buffer);
long map(long x, long in_min, long in_max, long out_min, long out_max);
int16_t mapJoystickValues(int16_t val, int16_t lower, int16_t middle, int16_t upper, bool reverse);
void EADC00_IRQHandler(void);
void Get_ADC(void);
float Get_Internal_Temp(void);

#endif

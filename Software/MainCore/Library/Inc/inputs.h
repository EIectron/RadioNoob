#ifndef INPUT_H
#define INPUT_H

#include "common.h"
#include "stdbool.h"
#include "delay.h"
#include "memory_lcd.h"
#include "eeprom.h"
#include "sound.h"


#define TRM_MIN						(-25)
#define TRM_MAX						25
#define TRM_VALUE					1
#define trm_button_delay	100
#define btn_vib_delay	20
#define SWITCH_HOMES	0x6DB

enum adc_channels
{
    ADC_ROLL = 0,
    ADC_PITCH,
    ADC_THROTTLE,
    ADC_YAW,
    ADC_POTL, 
    ADC_POTR, 
    ADC_VRd, 
    ADC_VRe, 
    ADC_IBAT, 
    ADC_VBAT
};

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
int calc_battery_percentage(float voltage);
void updateBattery(void);
void updateButtons(void);
void gpioMixer(void);
void checkMinMaxValues(uint32_t *buffer);
void calibrationAnalogJoysticks(void);
long map(long x, long in_min, long in_max, long out_min, long out_max);
int16_t mapJoystickValues(int16_t val, int16_t lower, int16_t middle, int16_t upper, bool reverse);
void EADC00_IRQHandler(void);
void Get_ADC(void);
float Get_Internal_Temp(void);

#endif

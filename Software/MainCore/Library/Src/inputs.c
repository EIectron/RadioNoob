#include "inputs.h"


extern TableParams myTableParams;
extern RCValues myRCvalues;
extern uint8_t TrmUpdate;

uint32_t adc_val[10] = {1500};

#define R2 40
#define R3 100
#define VOLTAGE_OUT(Vin) (((Vin) * R3) / (R2 + R3)) 
#define VOLTAGE_MAX 4.2
#define VOLTAGE_MIN 3.3
#define ADC_REFERENCE 3300
#define VOLTAGE_TO_ADC(in) ((ADC_REFERENCE * (in)) / 4096)
#define BATTERY_MAX_ADC VOLTAGE_TO_ADC(VOLTAGE_OUT(VOLTAGE_MAX))
#define BATTERY_MIN_ADC VOLTAGE_TO_ADC(VOLTAGE_OUT(VOLTAGE_MIN))

#define BatReadInterval	2000
uint32_t batReadTime = 0;
float vBat = 0.00;


#define 	numReadings	20
#define 	numCh				8
uint32_t	readings[numCh][numReadings];
uint8_t		readIndex;
uint32_t	total[numCh];
uint32_t	average[numCh];


void resetCalibrationValues(void)
{
	for(int i = 0; i<8; ++i)
	{
		myTableParams.channels[i].max = 2000;
		myTableParams.channels[i].mid = 1500;
		myTableParams.channels[i].min = 1000;
	}
}

void resetTrimValues(void)
{
	myTableParams.TrimConfig.trm_throttle = 0;
	myTableParams.TrimConfig.trm_yaw = 0;
	myTableParams.TrimConfig.trm_pitch = 0;
	myTableParams.TrimConfig.trm_roll = 0;
}

void Vibration(uint8_t retry, uint32_t time)
{
	for(uint8_t i = 0; i < retry; i++)
	{
		VIB_MTR_PIN = 1;
		Delay(time);
		VIB_MTR_PIN = 0;
		if(retry > 1)
			Delay(time);
	}
}

uint32_t GetSW(uint8_t sw)
{
  const uint32_t sw_pins[11] = {SW_A_PIN, SW_B_PIN, SW_C_DWN_PIN, SW_C_UP_PIN, SW_D_PIN, SW_E_DWN_PIN, SW_E_UP_PIN, SW_F_PIN, SW_G_DWN_PIN, SW_G_UP_PIN, SW_H_PIN};
	uint32_t sw_value = 0;
	
	for(int i = 0; i<11; i++)
	{
		if(sw_pins[i] == 0)
			bitset(sw_value, i);
		else
			bitclear(sw_value, i);
	}
	// 1	10 1 10 1 10 1 1 = 0x6DB	Defult Pos
	// 10 98 7 65 4 32 1 0
	//0b1,11,1,11,1,11,1,1
	//  H, G,F, E,D, C,B,A  
	if((sw == SWC) || (sw == SWE) || (sw == SWG)) // This is 3Pos Switches
	{
			if((sw_pins[sw] == 1) && (sw_pins[sw+1] == 1))
				return 1;
			else if((sw_pins[sw] == 1) && (sw_pins[sw+1] == 0))
				return 0;
			else if((sw_pins[sw] == 0) && (sw_pins[sw+1] == 1))
				return 2;
	}
	else if(sw == SW)	// Return total sw value
	{
		return sw_value;
	}
	else
	{
		return sw_pins[sw];
	}
}

int calc_battery_percentage(float voltage)
{
    int battery_percentage = 100 * (voltage - VOLTAGE_MIN) / (VOLTAGE_MAX - VOLTAGE_MIN);

    if (battery_percentage < 0)
        battery_percentage = 0;
    if (battery_percentage > 100)
        battery_percentage = 100;

    return battery_percentage;
}


// This not stable?
void updateBattery(void)
{
	if((millis() - batReadTime) > BatReadInterval)
	{
//		CHRG_EN_PIN = 0;
//		BAT_LVL_PIN = 1;
//		Delay(1);
//		BAT_LVL_PIN = 0;
//		CHRG_EN_PIN = 1;
		vBat = (float)(VOLTAGE_OUT(adc_val[ADC_VBAT]) * 2) / 1000;
		batReadTime = millis();
	}
}

// Buton durumunu okumak için fonksiyon
void updateButtons(void) {
	
	if(!T_TRM_INC_PIN){
//		Delay(trm_button_delay);
		myTableParams.TrimConfig.trm_throttle += TRM_VALUE;
		if(myTableParams.TrimConfig.trm_throttle > TRM_MAX){
			myTableParams.TrimConfig.trm_throttle = TRM_MAX;
//			WAVPlayer("/Sounds/03_button_end_beep.wav");
		}
		else
		{
//			WAVPlayer("/Sounds/02_button_beep.wav");
			TrmUpdate = 1;
		}
	}
	else if(!T_TRM_DEC_PIN){
//		Delay(trm_button_delay);
		myTableParams.TrimConfig.trm_throttle -= TRM_VALUE;
		if(myTableParams.TrimConfig.trm_throttle < TRM_MIN){
			myTableParams.TrimConfig.trm_throttle = TRM_MIN;
//			WAVPlayer("/Sounds/03_button_end_beep.wav");
		}
		else
		{
//			WAVPlayer("/Sounds/02_button_beep.wav");
			TrmUpdate = 1;
		}
	}
	
	if(!Y_TRM_INC_PIN){
//		Delay(trm_button_delay);
		myTableParams.TrimConfig.trm_yaw += TRM_VALUE;
		if(myTableParams.TrimConfig.trm_yaw > TRM_MAX){
			myTableParams.TrimConfig.trm_yaw = TRM_MAX;
//			WAVPlayer("/Sounds/03_button_end_beep.wav");
		}
		else
		{
//			WAVPlayer("/Sounds/02_button_beep.wav");
			TrmUpdate = 2;
		}
	}
	else if(!Y_TRM_DEC_PIN){
//		Delay(trm_button_delay);
		myTableParams.TrimConfig.trm_yaw -= TRM_VALUE;
		if(myTableParams.TrimConfig.trm_yaw < TRM_MIN){
			myTableParams.TrimConfig.trm_yaw = TRM_MIN;
//			WAVPlayer("/Sounds/03_button_end_beep.wav");
		}
		else
		{
//			WAVPlayer("/Sounds/02_button_beep.wav");
			TrmUpdate = 2;
		}
	}
	
	if(!P_TRM_INC_PIN){
//		Delay(trm_button_delay);
		myTableParams.TrimConfig.trm_pitch += TRM_VALUE;
		if(myTableParams.TrimConfig.trm_pitch > TRM_MAX){
			myTableParams.TrimConfig.trm_pitch = TRM_MAX;
//			WAVPlayer("/Sounds/03_button_end_beep.wav");
		}
		else
		{
//			WAVPlayer("/Sounds/02_button_beep.wav");
			TrmUpdate = 3;
		}
	}
	else if(!P_TRM_DEC_PIN){
//		Delay(trm_button_delay);
		myTableParams.TrimConfig.trm_pitch -= TRM_VALUE;
		if(myTableParams.TrimConfig.trm_pitch < TRM_MIN){
			myTableParams.TrimConfig.trm_pitch = TRM_MIN;
//			WAVPlayer("/Sounds/03_button_end_beep.wav");
		}
		else
		{
//			WAVPlayer("/Sounds/02_button_beep.wav");
			TrmUpdate = 3;
		}
	}
	
	if(!R_TRM_INC_PIN){
//		Delay(trm_button_delay);
		myTableParams.TrimConfig.trm_roll += TRM_VALUE;
		if(myTableParams.TrimConfig.trm_roll > TRM_MAX){
			myTableParams.TrimConfig.trm_roll = TRM_MAX;
//			WAVPlayer("/Sounds/03_button_end_beep.wav");
		}
		else
		{
//			WAVPlayer("/Sounds/02_button_beep.wav");
			TrmUpdate = 4;
		}
	}
	else if(!R_TRM_DEC_PIN){
//		Delay(trm_button_delay);
		myTableParams.TrimConfig.trm_roll -= TRM_VALUE;
		if(myTableParams.TrimConfig.trm_roll < TRM_MIN){
			myTableParams.TrimConfig.trm_roll = TRM_MIN;
//			WAVPlayer("/Sounds/03_button_end_beep.wav");
		}
		else	
		{
//			WAVPlayer("/Sounds/02_button_beep.wav");
			TrmUpdate = 4;
		}
	}
}

void SmoothAdc(void)
{
	for(uint8_t i = 0; i < numCh; i++)
	{
		// subtract the last reading:
		total[i] = total[i] - readings[i][readIndex]; 
		// read from the sensor:
		readings[i][readIndex] = adc_val[i];
		// add the reading to the total:
		total[i] = total[i] + readings[i][readIndex];
	}
	// advance to the next position in the array:
	readIndex = readIndex + 1;
	
	// if we're at the end of the array...
	if (readIndex >= numReadings) {
		// ...wrap around to the beginning:
		readIndex = 0;
	}
	
	for(uint8_t i = 0; i < numCh; i++)
	{
		// calculate the average:
		average[i] = total[i] / numReadings;
	}
	
}

void gpioMixer(void)
{
	
	SmoothAdc();

	myRCvalues.channelValues[ROLL]  = average[0];
	myRCvalues.channelValues[PITCH] = average[1];
	myRCvalues.channelValues[THROTTLE] = average[2];
	myRCvalues.channelValues[YAW]   = average[3];
	myRCvalues.channelValues[AUX1]  = average[4]; // Pot Left
	myRCvalues.channelValues[AUX2]  = average[5]; // Pot Right
	myRCvalues.channelValues[AUX3]  = average[6];	// Pot Dwn Left
	myRCvalues.channelValues[AUX4]  = average[7]; // Pot Dwn Right
	myRCvalues.channelValues[AUX5]  = GetSW(SWG);
	myRCvalues.channelValues[AUX6]  = GetSW(SWD);
	myRCvalues.channelValues[AUX7]  = GetSW(SWC);
	myRCvalues.channelValues[AUX8]  = GetSW(SWB);

//For debug
//	printf("\033[2J\033[H");
//	printf("THROTTLE: %d\n", myRCvalues.channelValues[THROTTLE]);
//	printf("YAW: %d\n", myRCvalues.channelValues[YAW]);
//	printf("PITCH: %d\n", myRCvalues.channelValues[PITCH]);
//	printf("ROLL: %d\n", myRCvalues.channelValues[ROLL]);
//	printf("AUX1: %d\n", myRCvalues.channelValues[4]);
//	printf("AUX2: %d\n", myRCvalues.channelValues[5]);
//	printf("AUX3: %d\n", myRCvalues.channelValues[6]);
//	printf("AUX4: %d\n", myRCvalues.channelValues[7]);
//	printf("AUX5: %d\n", GetSW(SWG));
//	printf("AUX6: %d\n", GetSW(SWD));
//	printf("AUX7: %d\n", GetSW(SWC));
//	printf("AUX8: %d\n", GetSW(SWB));
//	Delay(50);
}

void checkMinMaxValues(uint32_t *buffer)
{
	for(int i = 0; i<8; i++)
	{
		if(myTableParams.channels[i].max < buffer[i])
			myTableParams.channels[i].max = buffer[i];
		else if(myTableParams.channels[i].min > buffer[i])
			myTableParams.channels[i].min = buffer[i];
	}
}

void calibrationAnalogJoysticks(void)
{
//	printf("\033[2J\033[H");
//	printf("Butun joystickleri max ve min pozisyonlarina getirin\n sonrasinda 'END' butonuna basin\n");
	GFXDisplayPutString(10, 30, &fontConsolas24h, "Butun joystickleri max ve min", BLACK, WHITE);
	GFXDisplayPutString(10, 50, &fontConsolas24h, "pozisyonlarina getirin ", BLACK, WHITE);
	GFXDisplayPutString(10, 70, &fontConsolas24h, "sonrasinda 'END' butonuna basin", BLACK, WHITE);
	GFXDisplayUpdate();
	resetCalibrationValues();
	resetTrimValues();
	while(END_SW_PIN)
	{
		checkMinMaxValues(adc_val);
	}
	Delay(50);
	while(!END_SW_PIN)

	GFXDisplayAllClear();
	GFXDisplayPutString(10, 30, &fontConsolas24h, "Butun stickleri orta konuma", BLACK, WHITE);
	GFXDisplayPutString(10, 50, &fontConsolas24h, "getirin ve 'END' butonuna basin", BLACK, WHITE);
	GFXDisplayUpdate();
//	printf("Butun stickleri orta konuma getirin ve 'END' butonuna basin\n");
	while(END_SW_PIN);
	Delay(50);
	while(!END_SW_PIN)

	for(int i = 0; i<8; i++)
	{
		myTableParams.channels[i].mid = adc_val[i];
	}
		// Kontrol amaçli degerleri yazdirma
	
	GFXDisplayAllClear();
	GFXDisplayPutString(50, 20, &fontConsolas24h, "Channel -> Min, Mid, Max", BLACK, WHITE);
	char *text = (char *)malloc(32);
	for (int i = 0; i < 8; ++i) {
//		printf("%s -> Min: %d, Mid: %d, Max: %d\n", chan_names[i], myTableParams.channels[i].min, myTableParams.channels[i].mid, myTableParams.channels[i].max);
		sprintf(text, "%s ->%d, %d, %d",chan_names[i], myTableParams.channels[i].min, myTableParams.channels[i].mid, myTableParams.channels[i].max);
		GFXDisplayPutString(10, (i*20)+50, &fontConsolas24h, text, BLACK, WHITE);
	}
	free(text);
	GFXDisplayPutString(10, 210, &fontConsolas24h, "Cikmak icin 'END' butonuna basin", WHITE, BLACK);
	GFXDisplayUpdate();
	while(END_SW_PIN);
	Delay(50);
	while(!END_SW_PIN)
	
	GFXDisplayAllClear();
	GFXDisplayPutString(10, 30, &fontConsolas24h, "Kalibrasyon bitmistir", BLACK, WHITE);
	GFXDisplayPutString(10, 50, &fontConsolas24h, "cikmak icin 'END' butonuna basin", BLACK, WHITE);
	GFXDisplayUpdate();
	myTableParams.MainConfig.calibStat = 1;
	
	TableWriteToFlash(DATA_FLASH_BASE_ADDR, myTableParams);
//	printf("Kalibrasyon bitmistir, cikmak icin 'END' butonuna basin\n");
	while(END_SW_PIN);
	Delay(50);
	while(!END_SW_PIN)
	GFXDisplayAllClear();
}

void EADC00_IRQHandler(void)
{
    /* Clear the A/D ADINT0 interrupt flag */
    EADC_CLR_INT_FLAG(EADC0, 0x1);
	
		adc_val[0] = (int32_t)EADC_GET_CONV_DATA(EADC, 0);
		adc_val[1] = (int32_t)EADC_GET_CONV_DATA(EADC, 1);
		adc_val[2] = (int32_t)EADC_GET_CONV_DATA(EADC, 2);
		adc_val[3] = (int32_t)EADC_GET_CONV_DATA(EADC, 3);
		adc_val[4] = (int32_t)EADC_GET_CONV_DATA(EADC, 4);
		adc_val[5] = (int32_t)EADC_GET_CONV_DATA(EADC, 5);
		adc_val[6] = (int32_t)EADC_GET_CONV_DATA(EADC, 6);
		adc_val[7] = (int32_t)EADC_GET_CONV_DATA(EADC, 7);
		adc_val[8] = (int32_t)EADC_GET_CONV_DATA(EADC, 8);
		adc_val[9] = (int32_t)EADC_GET_CONV_DATA(EADC, 9);
	
		SmoothAdc();
}
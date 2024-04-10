#include "inputs.h"

extern volatile uint32_t g_u32AdcIntFlag, g_u32COVNUMFlag;
extern TableParams myTableParams;
extern RCValues myRCvalues;
extern uint8_t TrmUpdate;

uint32_t adc_val[10] = {1500};

#define 	numReadings	20
#define 	numCh				10
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


void gpioMixer(void)
{
	myRCvalues.channelValues[THROTTLE] = average[2];
	myRCvalues.channelValues[YAW]   = average[3];
	myRCvalues.channelValues[PITCH] = average[1];
	myRCvalues.channelValues[ROLL]  = average[0];
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
//	printf("THROTTLE: %d\n", adc_val[2]);
//	printf("YAW: %d\n", adc_val[3]);
//	printf("PITCH: %d\n", adc_val[1]);
//	printf("ROLL: %d\n", adc_val[0]);
//	printf("AUX1: %d\n", adc_val[4]);
//	printf("AUX2: %d\n", adc_val[5]);
//	printf("AUX3: %d\n", adc_val[6]);
//	printf("AUX4: %d\n", adc_val[7]);
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


void Get_ADC(void)
{
		uint32_t adcRaw[10];
		/* Set input mode as single-end and enable the A/D converter */
		EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);

		/* Configure the sample module 0 for analog input channel 2 and enable EPWM0 trigger source */
		
		EADC_ConfigSampleModule(EADC, 0, EADC_TIMER0_TRIGGER, 2);	 //Roll
		EADC_ConfigSampleModule(EADC, 1, EADC_TIMER0_TRIGGER, 4);  //Pitch
		EADC_ConfigSampleModule(EADC, 2, EADC_TIMER0_TRIGGER, 3);	 //Throttle
		EADC_ConfigSampleModule(EADC, 3, EADC_TIMER0_TRIGGER, 5);	 //Yaw
		EADC_ConfigSampleModule(EADC, 4, EADC_TIMER0_TRIGGER, 6);  //PotL
		EADC_ConfigSampleModule(EADC, 5, EADC_TIMER0_TRIGGER, 7);	 //PotR
		EADC_ConfigSampleModule(EADC, 6, EADC_TIMER0_TRIGGER, 8);  //VRd
		EADC_ConfigSampleModule(EADC, 7, EADC_TIMER0_TRIGGER, 9);  //VRe
		EADC_ConfigSampleModule(EADC, 8, EADC_TIMER0_TRIGGER, 10); //IBat
		EADC_ConfigSampleModule(EADC, 9, EADC_TIMER0_TRIGGER, 11); //VBat

		/* Clear the A/D ADINT0 interrupt flag for safe */
		EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);

		/* Enable the sample module 0 interrupt */
		EADC_ENABLE_INT(EADC, BIT0);//Enable sample module A/D ADINT0 interrupt.
		EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, BIT0);//Enable sample module 0 interrupt.
		NVIC_EnableIRQ(EADC00_IRQn);


		/* Reset the EADC indicator and enable EPWM0 channel 0 counter */
		g_u32AdcIntFlag = 0;
		g_u32COVNUMFlag = 0;
		TIMER_Start(TIMER0);
		
		while(1)
		{
				/* Wait ADC interrupt (g_u32AdcIntFlag will be set at IRQ_Handler function) */
				while(g_u32AdcIntFlag == 0);

				/* Reset the ADC interrupt indicator */
				g_u32AdcIntFlag = 0;

				/* Get the conversion result of the sample module 0 */
				adcRaw[g_u32COVNUMFlag - 1] = EADC_GET_CONV_DATA(EADC, g_u32COVNUMFlag - 1);

				if(g_u32COVNUMFlag > 9)
						break;
		}
		/* Disable Timer0 counter */
    TIMER_Stop(TIMER0);

		/* Disable sample module 0 interrupt */
		EADC_DISABLE_SAMPLE_MODULE_INT(EADC, 0, BIT0);
		
		
		for(uint8_t i = 0; i < numCh; i++)
		{
			// subtract the last reading:
			total[i] = total[i] - readings[i][readIndex]; 
			// read from the sensor:
			readings[i][readIndex] = adcRaw[i];
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

float Get_Internal_Temp(void)
{
		int32_t  i32ConversionData;
	/* Set input mode as single-end and enable the A/D converter */
    EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);

    /* Set sample module 17 external sampling time to 0x3F */
    EADC_SetExtendSampleTime(EADC, 17, 0x3F);

    /* Clear the A/D ADINT0 interrupt flag for safe */
    EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);

    /* Enable the sample module 17 interrupt.  */
    EADC_ENABLE_INT(EADC, BIT0);//Enable sample module A/D ADINT0 interrupt.
    EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, BIT17);//Enable sample module 17 interrupt.
    NVIC_EnableIRQ(EADC00_IRQn);

    /* Reset the ADC interrupt indicator and trigger sample module 17 to start A/D conversion */
    g_u32AdcIntFlag = 0;
    EADC_START_CONV(EADC, BIT17);

    /* Wait EADC conversion done */
    while(g_u32AdcIntFlag == 0);

    /* Disable the ADINT0 interrupt */
    EADC_DISABLE_INT(EADC, BIT0);

    /* Get the conversion result of the sample module 17 */
    i32ConversionData = EADC_GET_CONV_DATA(EADC, 17);
    //printf("Conversion result of temperature sensor: 0x%X (%d)\n", i32ConversionData, i32ConversionData);

    /* The equation of converting to real temperature is as below
     * (25+(((float)i32ConversionData/4095*3300)-675)/(-1.83)), 3300 means ADCVREF=3.3V
     * If ADCREF set to 1.6V, the equation should be updated as below
     * (25+(((float)i32ConversionData/4095*1600)-675)/(-1.83)), 1600 means ADCVREF=1.6V
     */
    printf("Current Temperature = %2.1f\n\n", (25+(((float)i32ConversionData/4095*3300)-675)/(-1.83)));
		
		return (25+(((float)i32ConversionData/4095*3300)-675)/(-1.83));
}


void EADC00_IRQHandler(void)
{
    EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);      /* Clear the A/D ADINT0 interrupt flag */
		g_u32AdcIntFlag = 1;
    g_u32COVNUMFlag++;
}
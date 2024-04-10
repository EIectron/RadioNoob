/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Show how to set GPIO pin mode and use pin data input/output control.
 *
 * @copyright (C) 2013~2015 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/

#include "main.h"

extern TableParams myTableParams;
extern RCValues myRCvalues;

void SYS_Init(void);
void UART0_Init(void);
void TIMER0_Init(void);
void TIMER1_Init(void);
void TIMER2_Init(void);
void EPWM0_Init(void);
void GPIO_Init(void);
void SPI_Init(void);
void QEI0_Init(void);
void QEI0_Init_Index(uint32_t index);


int32_t main(void)
{

    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();
	
    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf */
    UART0_Init();
	
		/* Init TIMER0 for EADC */
    TIMER0_Init();
	
		/* Init TIMER1 for Delay */
		TIMER1_Init();
		
		/* Init SPI */
    SPI_Init();
		
		/* Init QEI */
		QEI0_Init_Index(0x08);
		
		/* Init GPIO */
		GPIO_Init();
		
		CRSF_Begin();
		
		printf("RadiolinkAT9C	V:%s\n", VERSION);
    printf("Running on @%dMHz\n", SystemCoreClock/1000000);

		setDataFlashBase(DATA_FLASH_BASE_ADDR);
		
		TableReadToFlash(DATA_FLASH_BASE_ADDR, &myTableParams);
		if(myTableParams.MainConfig.calibStat == 0xFF)
			printf("Fresh EEPROM Calibration!\n");

		printf("Main Task is creating ...\n");
		xTaskCreate(vCheckTask, "Main Task",  mainCHECK_TASK_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);

		printf("LCD Task is creating ...\n");
		xTaskCreate(vLcdTask, "LCD Task",  mainLCD_TASK_STACK_SIZE, NULL, mainLCD_TASK_PRIORITY, NULL);
		
		printf("FreeRTOS is starting ...\n\n");
		
		vTaskStartScheduler();

		for(;;);
		
		/* Application Setup */
		
		return 0;
}


/* FreeRTOS Functions Implemantation*/

void vCheckTask(void *pvParameters) {
	
	/* Task Setup */

	/* Task Loop */
	
	for (;;) 
	{
		Get_ADC();
		gpioMixer();
		CRSF_Update();
		serialEvent();
	}
}

void vLcdTask(void *pvParameters) {
	
	/* Task Setup */
	GUI_Init();
	/* Task Loop */
	
	for (;;) {
//		LED_GREEN_PIN = !LED_GREEN_PIN;
		GUI_Update();
//		TaskStatus_t xTaskStatus;
//    // Görevin bilgilerini al
//    vTaskGetInfo(NULL, &xTaskStatus, pdTRUE, eRunning);
//		printf("Task Name: %s, Task State: %d, Task Priority: %d\n", 
//           xTaskStatus.pcTaskName, xTaskStatus.eCurrentState, xTaskStatus.uxCurrentPriority);
//		vTaskDelay(500);
	}
	
}

void SYS_Init(void)
{
    /* Set XT12_OUT(PF.2) and XT1_IN(PF.3) to input mode */
    SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF3MFP_Msk | SYS_GPF_MFPL_PF2MFP_Msk);
    SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF3MFP_XT1_IN | SYS_GPF_MFPL_PF2MFP_XT1_OUT);
	
		/* Set XT32_OUT(PF.5) and XT1_IN(PF.4) to input mode */
		SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF5MFP_Msk | SYS_GPF_MFPL_PF4MFP_Msk);
    SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF5MFP_X32_IN | SYS_GPF_MFPL_PF4MFP_X32_OUT);

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Switch HCLK clock source to HXT */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HXT,CLK_CLKDIV0_HCLK(1));

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(FREQ_192MHZ);

    /* Set both PCLK0 and PCLK1 as HCLK/2 */
    CLK->PCLKDIV = CLK_PCLKDIV_APB0DIV_DIV2 | CLK_PCLKDIV_APB1DIV_DIV2;

	  //Modul clock settings
    /* Enable UART0 module clock */
    CLK_EnableModuleClock(UART0_MODULE);
		
		/* Enable UART1 module clock */
    CLK_EnableModuleClock(UART1_MODULE);
		
		/* Enable Timer 0 module clock */
    CLK_EnableModuleClock(TMR0_MODULE);
		
		/* Enable TIMER1 module clock */
		CLK_EnableModuleClock(TMR1_MODULE);
		
		/* Enable TIMER2 module clock */
		CLK_EnableModuleClock(TMR2_MODULE);
		
		/* Enable EPWM0 module clock */
    CLK_EnableModuleClock(EPWM0_MODULE);
		
		/* Enable QEI0 module clock */
		CLK_EnableModuleClock(QEI0_MODULE);
		
		/* Enable EADC module clock */
    CLK_EnableModuleClock(EADC_MODULE);
		
		/* Enable SPI0 peripheral clock */
    CLK_EnableModuleClock(SPI0_MODULE);
		
		/* Enable SHD0 clock */
		CLK_EnableModuleClock(SDH0_MODULE);
		
		/* Enable PDMA clock */
		CLK_EnableModuleClock(PDMA_MODULE);
		
		/* Enable DAC module clock */
    CLK_EnableModuleClock(DAC_MODULE);
		
		/* Reset DAC modul */
		SYS_ResetModule(DAC_MODULE);
				
		
		//Clock selections
    /* Select UART module clock source as HXT and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HXT, CLK_CLKDIV0_UART0(1));
		
		/* Select UART module clock source */
		CLK_SetModuleClock(UART1_MODULE, CLK_CLKSEL1_UART0SEL_HXT, CLK_CLKDIV0_UART1(1));
		
		/* Select TIMER0 module clock source as HXT */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HXT, 0);
		
		/* Select TIMER1 module clock source as HXT and UART module clock divider as 1 */
		CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HXT, 0);
		
		/* Select TIMER1 module clock source as HXT and UART module clock divider as 1 */
		CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_HXT, 0);
		
		/* Select EPWM0 module clock source as PCLK0 */
    CLK_SetModuleClock(EPWM0_MODULE, CLK_CLKSEL2_EPWM0SEL_PCLK0, 0);
		
		/* EADC clock source is 96MHz, set divider to 8, EADC clock is 96/8 MHz */
    CLK_SetModuleClock(EADC_MODULE, 0, CLK_CLKDIV0_EADC(8));
		
		/* Select PCLK as the clock source of SPI0 and SPI1 */
    CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL2_SPI0SEL_HXT, MODULE_NoMsk);
		
		/* Select SHD0 clock source */
		CLK_SetModuleClock(SDH0_MODULE, CLK_CLKSEL0_SDH0SEL_PLL, CLK_CLKDIV0_SDH0(10));
		
		/* Reset Module*/
		SYS_ResetModule(SDH0_MODULE);
		
		/* Update System Core Clock */
		/* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
		SystemCoreClockUpdate();
		
		//Multi-Function pins settings
    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPC_MFPH &= ~(SYS_GPC_MFPH_PC12MFP_Msk | SYS_GPC_MFPH_PC11MFP_Msk);
    SYS->GPC_MFPH |= (SYS_GPC_MFPH_PC12MFP_UART0_TXD | SYS_GPC_MFPH_PC11MFP_UART0_RXD);
		
		/* Set GPA multi-function pins for UART1 RXD, TXD and RTS*/
		SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA9MFP_Msk | SYS_GPA_MFPH_PA8MFP_Msk);
    SYS->GPA_MFPH |= (SYS_GPA_MFPH_PA9MFP_UART1_TXD | SYS_GPA_MFPH_PA8MFP_UART1_RXD);
//    SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk);
//    SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA0MFP_UART1_nRTS);
		
		/* Set GPF multi-function pins for ICE_DAT and ICE_CLK */
		SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF1MFP_Msk | SYS_GPF_MFPL_PF0MFP_Msk);
    SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF1MFP_ICE_CLK | SYS_GPF_MFPL_PF0MFP_ICE_DAT);
		
		/* Set PA multi-function pins for QEI0_A, QEI0_B, QEI0_INDEX */
		SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA4MFP_Msk | SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA5MFP_Msk);
		SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA4MFP_QEI0_A | SYS_GPA_MFPL_PA3MFP_QEI0_B | SYS_GPA_MFPL_PA5MFP_QEI0_INDEX);
		
//		/* Enable temperature sensor */
//    SYS->IVSCTL |= SYS_IVSCTL_VTEMPEN_Msk;
		
		
		
				/* Set PB.2 ~ PB.11 to input mode */
    PB->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk | GPIO_MODE_MODE4_Msk | 
									GPIO_MODE_MODE5_Msk | GPIO_MODE_MODE6_Msk | GPIO_MODE_MODE7_Msk | 
									GPIO_MODE_MODE8_Msk | GPIO_MODE_MODE9_Msk | GPIO_MODE_MODE10_Msk | 
									GPIO_MODE_MODE11_Msk);
									
	  /* Configure the GPB2 - GPB3 EADC1 analog input pins.  */
    SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB2MFP_Msk | SYS_GPB_MFPL_PB3MFP_Msk |
                       SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk | 
											 SYS_GPB_MFPL_PB6MFP_Msk | SYS_GPB_MFPL_PB7MFP_Msk );
											 
		/* Configure the GPB8 - GPB11 EADC1 analog input pins.  */									 
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk |
											 SYS_GPB_MFPH_PB10MFP_Msk | SYS_GPB_MFPH_PB11MFP_Msk);				

		SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB2MFP_EADC0_CH2 | SYS_GPB_MFPL_PB3MFP_EADC0_CH3 |
											SYS_GPB_MFPL_PB4MFP_EADC0_CH4 | SYS_GPB_MFPL_PB5MFP_EADC0_CH5 |
											SYS_GPB_MFPL_PB6MFP_EADC0_CH6 | SYS_GPB_MFPL_PB7MFP_EADC0_CH7 );
		SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_EADC0_CH8 | SYS_GPB_MFPH_PB9MFP_EADC0_CH9 |
											SYS_GPB_MFPH_PB10MFP_EADC0_CH10 | SYS_GPB_MFPH_PB11MFP_EADC0_CH11);

		/* Disable the GPB2 - GPB11 digital input path to avoid the leakage current. */
    GPIO_DISABLE_DIGITAL_PATH(PB, BIT11|BIT10|BIT9|BIT8|BIT7|BIT6|BIT5|BIT4|BIT3|BIT2);
		
		/* Enable SPI0 clock pin (PF8) schmitt trigger */
    PF->SMTEN |= GPIO_SMTEN_SMTEN2_Msk;
		/* Enable SPI0 I/O high slew rate */
    GPIO_SetSlewCtl(PF, 0xF, GPIO_SLEWCTL_HIGH);
		/* Setup SPI0 multi-function pins */
		SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF6MFP_Msk);
    SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF6MFP_SPI0_MOSI);
		
		SYS->GPF_MFPH &= ~(SYS_GPF_MFPH_PF8MFP_Msk);
		SYS->GPF_MFPH |= (SYS_GPF_MFPH_PF8MFP_SPI0_CLK);
		
		
		/* Setup SD0 MFP */
		SYS->GPE_MFPL &= ~(SYS_GPE_MFPL_PE7MFP_Msk | 
										 SYS_GPE_MFPL_PE6MFP_Msk | 
										 SYS_GPE_MFPL_PE2MFP_Msk | 
										 SYS_GPE_MFPL_PE3MFP_Msk | 
										 SYS_GPE_MFPL_PE4MFP_Msk | 
										 SYS_GPE_MFPL_PE5MFP_Msk);
	
		SYS->GPE_MFPL |= (SYS_GPE_MFPL_PE7MFP_SD0_CMD | 
											SYS_GPE_MFPL_PE6MFP_SD0_CLK | 
											SYS_GPE_MFPL_PE2MFP_SD0_DAT0 | 
											SYS_GPE_MFPL_PE3MFP_SD0_DAT1 | 
											SYS_GPE_MFPL_PE4MFP_SD0_DAT2 |
											SYS_GPE_MFPL_PE5MFP_SD0_DAT3);
		
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB12MFP_Msk);
		SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB12MFP_SD0_nCD);
		
		
		/* Setup DAC MFP */
		SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB13MFP_Msk);
		SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB13MFP_DAC1_OUT);


    /* Disable digital input path of analog pin DAC1_OUT to prevent leakage */
    GPIO_DISABLE_DIGITAL_PATH(PB, (1ul << 13));

}

void UART0_Init(void)
{
		/* Reset UART module */
		SYS_ResetModule(UART0_RST);
    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}


void TIMER0_Init(void)
{
    /* Set timer0 periodic time-out period is 3us if timer clock is 12 MHz */
    TIMER_SET_CMP_VALUE(TIMER0, 36);//TIMER0->CMP = 36;

    /* Start timer counter in periodic mode and enable timer interrupt trigger EADC */
    TIMER0->CTL = TIMER_PERIODIC_MODE;
    TIMER0->TRGCTL |= TIMER_TRGCTL_TRGEADC_Msk;
}

void TIMER1_Init(void)
{
		/* Open Timer1 in periodic mode, enable interrupt and 2 interrupt ticks per second */
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000000);
    TIMER_EnableInt(TIMER1);
		NVIC_EnableIRQ(TMR1_IRQn);
		TIMER_Start(TIMER1);
}

void EPWM0_Init(void)
{

    /* Set EPWM0 timer clock prescaler */
    EPWM_SET_PRESCALER(EPWM0, 0, 10);

    /* Set up counter type */
    EPWM0->CTL1 &= ~EPWM_CTL1_CNTTYPE0_Msk;

    /* Set EPWM0 timer duty */
    EPWM_SET_CMR(EPWM0, 0, 1000);

    /* Set EPWM0 timer period */
    EPWM_SET_CNR(EPWM0, 0, 2000);

    /* EPWM period point trigger ADC enable */
    EPWM_EnableADCTrigger(EPWM0, 0, EPWM_TRG_ADC_EVEN_PERIOD);

    /* Set output level at zero, compare up, period(center) and compare down of specified channel */
    EPWM_SET_OUTPUT_LEVEL(EPWM0, BIT0, EPWM_OUTPUT_HIGH, EPWM_OUTPUT_LOW, EPWM_OUTPUT_NOTHING, EPWM_OUTPUT_NOTHING);

    /* Enable output of EPWM0 channel 0 */
    EPWM_EnableOutput(EPWM0, BIT0);

}


void GPIO_Init(void)
{
		// Outputs
		GPIO_SetMode(VIB_MTR_PORT, VIBRATION_MTR_BIT, GPIO_MODE_OUTPUT);
		GPIO_SetMode(LED_GREEN_PORT, LED_GREEN_BIT, GPIO_MODE_OUTPUT);
	  GPIO_SetMode(LED_RED_PORT, LED_RED_BIT, GPIO_MODE_OUTPUT);
		GPIO_SetMode(CHRG_PORT, CHRG_EN_BIT, GPIO_MODE_OUTPUT);
		GPIO_SetMode(BAT_LVL_PORT, BAT_LVL_BIT, GPIO_MODE_OUTPUT);
		GPIO_SetMode(DISPLAY_EN_PORT, DISPLAY_EN_BIT, GPIO_MODE_OUTPUT);
	  GPIO_SetMode(SPI0_CS_PORT, SPI0_CS_BIT, GPIO_MODE_OUTPUT);
	  GPIO_SetMode(AUDIO_EN_PORT, AUDIO_EN_BIT, GPIO_MODE_OUTPUT);
		GPIO_SetMode(UART1_DIR_PORT, UART1_DIR_BIT, GPIO_MODE_OUTPUT);
		GPIO_SetMode(DISPLAY_EXCOM_PORT, DISPLAY_EXCOM_BIT, GPIO_MODE_OUTPUT);
	
		// Inputs
		GPIO_SetMode(MODE_SW_PORT, MODE_SW_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(END_SW_PORT, END_SW_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(SW_A_PORT, SW_A_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(SW_B_PORT, SW_B_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(SW_C_PORT, SW_C_UP_BIT | SW_C_DWN_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(SW_D_PORT, SW_D_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(SW_E_PORT, SW_E_UP_BIT | SW_E_DWN_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(SW_F_PORT, SW_F_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(SW_G_PORT, SW_G_UP_BIT | SW_G_DWN_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(SW_H_PORT, SW_H_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(T_TRM_PORT, T_TRM_INC_BIT | T_TRM_DEC_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(Y_TRM_PORT, Y_TRM_INC_BIT | Y_TRM_DEC_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(P_TRM_PORT, P_TRM_INC_BIT | P_TRM_DEC_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(R_TRM_PORT, R_TRM_INC_BIT | R_TRM_DEC_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(CHRG_PORT, CHRG_STDBY_BIT | CHRG_STAT_BIT, GPIO_MODE_INPUT);
		GPIO_SetMode(ENCODER_SW_PORT, ENCODER_SW_BIT, GPIO_MODE_INPUT);
		
		
				
		CHRG_EN_PIN = 1;
		LED_GREEN_PIN = 1;
		LED_RED_PIN = 1;
		VIB_MTR_PIN = 0;
		BAT_LVL_PIN = 0;
		DISPLAY_EN_PIN = 0;
		SPI0_CS_PIN = 0;
		AUDIO_EN = 0;

}


void SPI_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init SPI                                                                                                */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Configure as a master, clock idle low, 8-bit transaction, drive output on falling clock edge and latch input on rising edge. */
    /* Set IP clock divider. SPI clock rate = 1MHz */
    SPI_Open(SPI0, SPI_MASTER, SPI_MODE_0, 8, 2000000); // Max 14 Mhz

    /* Enable the automatic hardware slave select function. Select the SS pin and configure as low-active. */
    //SPI_EnableAutoSS(SPI0, SPI_SS, SPI_SS_ACTIVE_HIGH);
		SPI_DisableAutoSS(SPI0);
		SPI_SET_LSB_FIRST(SPI0);	
		SPI_ENABLE(SPI0);
	
}

void QEI0_Init(void)
{
	/* Set QEI counting mode as X2 Free Counting Mode,
	set maximum counter value and enable IDX, QEA and QEB input */
	QEI_Open(QEI0, QEI_CTL_X2_COMPARE_COUNTING_MODE, 0x20);
	
//	QEI_ENABLE_INDEX_LATCH(QEI0);
	
	/* Start QEI function */
	QEI_Start(QEI0);	
}

void QEI0_Init_Index(uint32_t index)
{
	/* Set QEI counting mode as X2 Free Counting Mode,
	set maximum counter value and enable IDX, QEA and QEB input */
	QEI_Open(QEI0, QEI_CTL_X2_COMPARE_COUNTING_MODE, index);
	
	//QEI_ENABLE_INDEX_LATCH(QEI0);
	
	QEI_ENABLE_NOISE_FILTER(QEI0, QEI_CTL_NFCLKSEL_DIV64);
	
	/* Start QEI function */
	QEI_Start(QEI0);	
	
	QEI_ENABLE_INDEX_RELOAD(QEI0);
	
}


void TMR1_IRQHandler(void)
{
	if(TIMER_GetIntFlag(TIMER1) == 1)
	{
		/* Clear Timer1 time-out interrupt flag */
		TIMER_ClearIntFlag(TIMER1);
		usec++;
		vcom_sec++;
		if(vcom_sec >= (1000000 / EXTCOMIN_FREQ))
		{
			extcom_toggle();
			vcom_sec = 0;
		}
	}
}

#ifndef MAIN_H
#define MAIN_H

/* Device Includes */
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "math.h"

/* User Includes */
#include "common.h"
#include "delay.h"
#include "eeprom.h"
#include "crsf.h"
#include "inputs.h"
#include "memory_lcd.h"
#include "gui.h"


/* Kernel Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#define PLL_CLOCK         192000000UL
#define SYSTEM_TICK_FREQ	1000


/* FreeRTOS Definitions */
#define mainCHECK_TASK_PRIORITY				(tskIDLE_PRIORITY + 3UL)
#define mainLCD_TASK_PRIORITY				  (tskIDLE_PRIORITY + 3UL)

#define mainCHECK_TASK_STACK_SIZE			(512)
#define mainLCD_TASK_STACK_SIZE			  (512)


/* FreeRTOS Functions*/
void vCheckTask(void *pvParameters);
void vLcdTask(void *pvParameters);

void SYS_Init(void);
void UART0_Init(void);
void TIMER0_Init(void);
void TIMER1_Init(void);
void TIMER2_Init(void);
void EPWM0_Init(void);
void GPIO_Init(void);
void SPI_Init(void);


volatile uint32_t vcom_sec = 0;
void TMR1_IRQHandler(void);

#endif /* MAIN_H */
#ifndef MAIN_H
#define MAIN_H

#include "common.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "math.h"

#define PLL_CLOCK         192000000UL
#define SYSTEM_TICK_FREQ	1000

volatile uint32_t vcom_sec = 0;
void TMR1_IRQHandler(void);

#endif /* MAIN_H */
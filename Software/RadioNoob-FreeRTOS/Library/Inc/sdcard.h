#ifndef SDCARD_H
#define SDCARD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "M480.h"
#include "sdh_control.h"
#include "delay.h"

#define FILE_SIZE	12


void SDCard_Init(void);
uint32_t CheckFileSize(void);
void PrintSDCard(char *dir);

void SDH0_IRQHandler(void);	

#endif
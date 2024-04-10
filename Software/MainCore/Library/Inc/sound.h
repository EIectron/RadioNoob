#ifndef SOUND_H
#define SOUND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "M480.h"
#include "sdcard.h"
#include "wav.h"
#include "inputs.h"

#define AUDIO_BUFFER_SIZE  8192 * 8
#define SHIFT_LEVEL 8


void WAVPlayer(char* file_name);
void DAC_Init(uint32_t simple_rate);
void PDMA_Init(uint16_t SampleRate);
void RetriggerPDMA(uint8_t PCM_Buffer_Index);

#endif
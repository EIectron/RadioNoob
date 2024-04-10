#ifndef DELAY_H
#define DELAY_H
#include "M480.h"

extern volatile uint32_t usec;

uint32_t micros(void);
uint32_t millis(void);

void DelayUs(uint32_t us);
void Delay(uint32_t ms);

#endif /* DELAY_H */
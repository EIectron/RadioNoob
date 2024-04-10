#include "delay.h"

volatile uint32_t usec;

uint32_t micros(void)
{
	return usec;
}

uint32_t millis(void)
{
	return  usec / 1000;
}

void DelayUs(uint32_t us)
{
	uint32_t tick_count= micros();
	while((micros() - tick_count) < us){
		__NOP();
	}
}

void Delay(uint32_t ms){
	uint32_t tick_count= micros();
	while((micros() - tick_count) < (ms * 1000)){
		__NOP();
	}
}
#ifndef INC_RINGBUFFER_H_
#define INC_RINGBUFFER_H_

#include "m480.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Define Here what should be the buffer size of the ring buffer
#define UART_BUFFER_SIZE 256

#if (UART_BUFFER_SIZE > 256)
typedef uint16_t rxbuffer_Index_Type;
typedef uint16_t txbuffer_Index_Type;
#else
typedef uint8_t rxbuffer_Index_Type;
typedef uint8_t txbuffer_Index_Type;
#endif

typedef struct{
	volatile uint8_t _buffer[UART_BUFFER_SIZE];
	volatile unsigned int _head;
	volatile unsigned int _tail;
}RingBuffer_t;



void __push(RingBuffer_t *buff,uint8_t a);
uint8_t __pop(RingBuffer_t *buff);


void RingBuffer_Begin(void);
rxbuffer_Index_Type RingBuffer_available(void);
void RingBuffer_flush(void);
uint8_t RingBuffer_read(void);
uint8_t RingBuffer_find(char *);
uint8_t RingBuffer_peek(void);
char* RingBuffer_readString(void);
#endif /* INC_RINGBUFFERK_H_ */
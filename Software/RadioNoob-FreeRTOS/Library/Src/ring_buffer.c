#include "ring_buffer.h"

RingBuffer_t RingBuffer;

void RingBuffer_Begin(void)
{
	RingBuffer_flush();
}

void __push(RingBuffer_t *buff,uint8_t a)
{
	uint8_t __i=(unsigned int)(buff->_head + 1) % UART_BUFFER_SIZE;
	while(__i == buff->_tail);
	buff->_buffer[buff->_head]=a;
	buff->_head=__i;
}

uint8_t __pop(RingBuffer_t *buff)
{
	if(buff->_tail==buff->_head)
	{
		return 0;
	}
	else
	{
		uint8_t __data=buff->_buffer[buff->_tail];
		buff->_tail=(unsigned int)(buff->_tail + 1) % UART_BUFFER_SIZE;
		return __data;
	}
}

rxbuffer_Index_Type RingBuffer_available(void)
{
	 return  ((rxbuffer_Index_Type)(UART_BUFFER_SIZE + RingBuffer._head - RingBuffer._tail)) % UART_BUFFER_SIZE;
}

uint8_t RingBuffer_peek(void)
{
	if(RingBuffer._tail==RingBuffer._head)
	{
		return '\0';
	}
	else
	{
		return RingBuffer._buffer[RingBuffer._tail];
	}
}
char* RingBuffer_readString(void)
{
	rxbuffer_Index_Type datalen=((rxbuffer_Index_Type)(UART_BUFFER_SIZE + RingBuffer._head - RingBuffer._tail)) % UART_BUFFER_SIZE;
	char* buff=malloc(datalen+1);
	for(int i=0;i<datalen;i++)
	{
		buff[i]=__pop(&RingBuffer);
	}
	buff[datalen+1]='\0';
	return buff;
}

void RingBuffer_flush(void)
{
	memset((void *)RingBuffer._buffer, 0x00, sizeof(RingBuffer._buffer));
	RingBuffer._head = 0;
	RingBuffer._tail = 0;
}

uint8_t RingBuffer_read(void)
{
	return __pop(&RingBuffer);
}

uint8_t RingBuffer_find(char *searchString)
{
	const char *buffer = (const char *)RingBuffer._buffer;
	if(strstr(buffer, searchString)!=NULL)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

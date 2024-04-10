
#ifndef INC_WAV_H_
#define INC_WAV_H_

#include <stdint.h>

#define BUFF_LEN    32
#define BUFF_HALF_LEN   (BUFF_LEN/2)

#define PCM_BUFFER_SIZE 1024

typedef struct dma_desc_t
{
    uint32_t ctl;
    uint32_t endsrc;
    uint32_t enddest;
    uint32_t offset;
} DMA_DESC_T;


void PDMA_Reset_SCTable(uint8_t id);

typedef struct 
{
	uint32_t ChunkID;
	uint32_t ChunkSize;
	uint32_t Format;
	uint32_t Subchunk1ID;
	uint32_t Subchunk1Size;
	uint16_t AudioFormat;
	uint16_t NumChannels;
	uint32_t SampleRate;
	uint32_t ByteRate;
	uint16_t BlockAlign;
	uint16_t BitsPersample;
	uint32_t Subchunk2ID;
	uint32_t Subchunk2Size;
} WAV_FormatTypeDef;

#endif /* INC_WAV_H_ */
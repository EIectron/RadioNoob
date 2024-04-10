#include "sound.h"

uint16_t audioBuffer[AUDIO_BUFFER_SIZE];

FIL    wavFileObject;
size_t ReturnSize;

uint32_t aPCMBuffer[3][PCM_BUFFER_SIZE];
uint32_t aWavHeader[11];
uint16_t *pPCMSrcBuffer, *pPCMDesBuffer;

volatile uint8_t u8PCMBufferPlayIdx = 0;
volatile uint8_t u8PCMBuffer_Alt = 0;


void PDMA_IRQHandler(void);
////void DAC_IRQHandler(void);


void RetriggerPDMA(uint8_t PCM_Buffer_Index) {
    PDMA_SetTransferCnt(PDMA, 0, PDMA_WIDTH_16, PCM_BUFFER_SIZE * 2);
    /* transfer width is one word(32 bit) */
    PDMA_SetTransferAddr(PDMA, 0, (uint32_t)&aPCMBuffer[PCM_Buffer_Index][0], PDMA_SAR_INC, (uint32_t)&DAC1->DAT, PDMA_DAR_FIX);

    /* Select channel 0 request source from DAC */
    PDMA_SetTransferMode(PDMA, 0, PDMA_DAC1_TX, FALSE, 0);
    /* Set transfer type and burst size */
    PDMA_SetBurstType(PDMA, 0, PDMA_REQ_SINGLE, PDMA_BURST_1);
}

void PDMA_Init(uint16_t SampleRate) {

    /* Open Channel 0 */
    PDMA_Open(PDMA, BIT0);
    /* Set transfer data width, and transfer count */
    PDMA_SetTransferCnt(PDMA, 0, PDMA_WIDTH_16, PCM_BUFFER_SIZE * 2);
    /* transfer width is one word(32 bit) */
    PDMA_SetTransferAddr(PDMA, 0, (uint32_t)&aPCMBuffer[0][0], PDMA_SAR_INC, (uint32_t)&DAC1->DAT, PDMA_DAR_FIX);

    /* Select channel 0 request source from DAC */
    PDMA_SetTransferMode(PDMA, 0, PDMA_DAC1_TX, FALSE, 0);
    /* Set transfer type and burst size */
    PDMA_SetBurstType(PDMA, 0, PDMA_REQ_SINGLE, PDMA_BURST_1);

    PDMA_EnableInt(PDMA, 0, PDMA_INT_TRANS_DONE);
    NVIC_EnableIRQ(PDMA_IRQn);

    /* Set the timer 0 trigger DAC and enable D/A converter */
    DAC_Open(DAC1, 0, DAC_TIMER2_TRIGGER);
    /* The DAC conversion settling time is 1us */
    DAC_SetDelayTime(DAC1, 1);
    /* Clear the DAC conversion complete finish flag for safe */
    DAC_CLR_INT_FLAG(DAC1, 0);
    /* Enable the PDMA Mode */
    DAC_ENABLE_PDMA(DAC1);
    /* Enable Timer0 counting to start D/A conversion */
    TIMER_Open(TIMER2, TIMER_PERIODIC_MODE, SampleRate);
    TIMER_SetTriggerTarget(TIMER2, TIMER_TRG_TO_DAC);
    TIMER_Start(TIMER2);

}

void WAVPlayer(char* file_name) {
	
    FRESULT res;
    uint32_t u32WavSamplingRate, i;
	
		printf("Playing to: %s\n", file_name); 
		
		AUDIO_EN = 1;

    res = f_open(&wavFileObject, file_name, FA_OPEN_EXISTING | FA_READ);    

    if (res != FR_OK)
    {
        printf("Open file error!\n");
        return;
    }

    // read sampling rate from WAV header
    memset(aWavHeader, 0, sizeof(aWavHeader));
    f_read(&wavFileObject, aWavHeader, 44, &ReturnSize);
    u32WavSamplingRate = aWavHeader[6];

    u8PCMBufferPlayIdx = 0;
    PDMA_Init(u32WavSamplingRate);

    while (1)
    {
        if (u8PCMBuffer_Alt)
        {
					u8PCMBuffer_Alt = 0;

					res = f_read(&wavFileObject, (uint8_t *)&aPCMBuffer[2][0], PCM_BUFFER_SIZE * 4, &ReturnSize);
					pPCMSrcBuffer = (uint16_t *)&aPCMBuffer[2][0];

					for (i = 0; i < PCM_BUFFER_SIZE * 2; i++)
					{
						if (pPCMSrcBuffer[i] & 0x8000)      //signed 16-bit data convert to unsigned 12-bit data
						{
								pPCMSrcBuffer[i] = ~(pPCMSrcBuffer[i] - 1);
								pPCMSrcBuffer[i] =   0x800 - (pPCMSrcBuffer[i] >> 4);
						}
						else
						{
								pPCMSrcBuffer[i] = (pPCMSrcBuffer[i] >> 4) + 0x800;
						}
					}

					if (u8PCMBufferPlayIdx == 1)
					{
						pPCMDesBuffer = (uint16_t *)&aPCMBuffer[0][0];

						for (i = 0; i < PCM_BUFFER_SIZE * 2; i++)
								pPCMDesBuffer[i] = pPCMSrcBuffer[i];

					}
					else
					{
						pPCMDesBuffer = (uint16_t *)&aPCMBuffer[1][0];

						for (i = 0; i < PCM_BUFFER_SIZE * 2; i++)
								pPCMDesBuffer[i] = pPCMSrcBuffer[i];
					}

					if (f_eof(&wavFileObject))
							break;

        }
    }

    printf("Done..\n");
    f_close(&wavFileObject);

    while (u8PCMBuffer_Alt == 0); //waitting for audio data tx over
		AUDIO_EN = 0;
		TIMER_Close(TIMER2);
		PDMA_Close(PDMA);
		DAC_Close(DAC1, 0);
}

void PDMA_IRQHandler(void)
{
    uint32_t u32Status = PDMA_GET_INT_STATUS(PDMA);

    if (u32Status & 0x2)   /* done */
    {
        if (PDMA_GET_TD_STS(PDMA) & BIT0)
        {
            u8PCMBuffer_Alt = 1;
            u8PCMBufferPlayIdx ^= 1;
            RetriggerPDMA(u8PCMBufferPlayIdx);

        }

        PDMA_CLR_TD_FLAG(PDMA, PDMA_TDSTS_TDIF0_Msk);
    }
    else if (u32Status & 0x400)    /* Timeout */
    {
        PDMA_CLR_TMOUT_FLAG(PDMA, PDMA_TDSTS_TDIF0_Msk);
    }

}


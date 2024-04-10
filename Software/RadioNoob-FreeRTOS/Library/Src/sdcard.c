#include "sdcard.h"

FILINFO file_info_array[FILE_SIZE];
uint8_t selected_file_fattrib;

void SDCard_Init(void) {
		
	if (SDH_Open_Disk(SDH0, CardDetect_From_GPIO) == Successful) {
		printf("Open SD Card OK !\r\n");
		if (disk_initialize(SDH0_DRIVE) == RES_OK) {
			printf("Disk Initialize OK !\r\n");
			FS_Show_Features(SD0);
		} else {
			printf("Disk Initialize Err !\r\n");
		}
	} else {
		printf("Open SD Card Err !\r\n");
	}
}

uint32_t CheckFileSize(void) {
	uint32_t count = 0;
	for(int i = 0; i < FILE_SIZE; i++) {
		if (file_info_array[i].fattrib != NULL) {
			count++;
		}
	}
	return count;
}


void PrintSDCard(char *dir)
{
	SDCard_Init();
	FS_Show_Files(dir, file_info_array);
	uint32_t file_size = CheckFileSize();
	
	for(int i = 0; i<file_size; i++)
	{
			printf("%s\n", file_info_array[i].fname);
	}
}



void SDH0_IRQHandler(void)
{
    unsigned int volatile isr;
    unsigned int volatile ier;

    // FMI data abort interrupt
    if (SDH0->GINTSTS & SDH_GINTSTS_DTAIF_Msk)
    {
        /* ResetAllEngine() */
        SDH0->GCTL |= SDH_GCTL_GCTLRST_Msk;
    }

    //----- SD interrupt status
    isr = SDH0->INTSTS;
    if (isr & SDH_INTSTS_BLKDIF_Msk)
    {
        // block down
        SD0.DataReadyFlag = TRUE;
        SDH0->INTSTS = SDH_INTSTS_BLKDIF_Msk;
    }

    if (isr & SDH_INTSTS_CDIF_Msk)   // card detect
    {
        //----- SD interrupt status
        // delay 10 us to sync the GPIO and SDH
        {
					int volatile delay = SystemCoreClock / 1000000 * 10;
					for(; delay > 0UL; delay--)
					{
							__NOP();
					}
					isr = SDH0->INTSTS;
        }

        if (isr & SDH_INTSTS_CDSTS_Msk)
        {
					printf("LOG - SDH0 : Card Removed !\n");
					SD0.IsCardInsert = FALSE;   // SDISR_CD_Card = 1 means card remove for GPIO mode
					memset(&SD0, 0, sizeof(SDH_INFO_T));
        }
        else
        {
					printf("LOG - SDH0 : Card Inserted !\n");
					SD0.IsCardInsert = TRUE;
        }
        SDH0->INTSTS = SDH_INTSTS_CDIF_Msk;
    }

    // CRC error interrupt
    if (isr & SDH_INTSTS_CRCIF_Msk)
    {
        if (!(isr & SDH_INTSTS_CRC16_Msk))
        {
            //printf("***** ISR sdioIntHandler(): CRC_16 error !\n");
            // handle CRC error
        }
        else if (!(isr & SDH_INTSTS_CRC7_Msk))
        {
            if (!SD0.R3Flag)
            {
                //printf("***** ISR sdioIntHandler(): CRC_7 error !\n");
                // handle CRC error
            }
        }
        SDH0->INTSTS = SDH_INTSTS_CRCIF_Msk;      // clear interrupt flag
    }

    if (isr & SDH_INTSTS_DITOIF_Msk)
    {
        printf("LOG - SDH0 : ISR: data in timeout !\n");
        SDH0->INTSTS |= SDH_INTSTS_DITOIF_Msk;
    }

    // Response in timeout interrupt
    if (isr & SDH_INTSTS_RTOIF_Msk)
    {
        printf("LOG - SDH0 : ISR: response in timeout !\n");
        SDH0->INTSTS |= SDH_INTSTS_RTOIF_Msk;
    }
}
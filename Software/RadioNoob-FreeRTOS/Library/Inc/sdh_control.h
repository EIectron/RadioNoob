#ifndef INC_SDH_CONTROL_H_
#define INC_SDH_CONTROL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "M480.h"

#include "diskio.h"
#include "ff.h"

#define FILE_READ_BUFFER_SIZE  128

unsigned long get_fattime(void);

int32_t SDH_Open_Disk(SDH_T *sdh, uint32_t u32CardDetSrc);
void SDH_Close_Disk(SDH_T *sdh);

void FS_Show_Features(SDH_INFO_T sd);
void FS_Show_Files(const char *dirname, FILINFO *file_array);

FRESULT FS_Get_Data(const char *filename, uint8_t * buffer, uint16_t size);
FRESULT FS_Into_Dir(const char *dirname);
FRESULT list_dir (const char *path);
FRESULT scan_files (char* path);

#endif /* INC_SDH_CONTROL_H_ */
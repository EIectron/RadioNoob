#include "sdh_control.h"

/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */

unsigned long get_fattime (void) {
    unsigned long tmr;

    tmr=0x00000;

    return tmr;
}

FATFS  _FatfsVolSd0;		 // File system object
FATFS  _FatfsVolSd1;     // File system object

FILINFO fileInfo;        // File information object
DIR dir;                 // Directory object
FIL file;                // File object
FRESULT res;             // FatFs function result
UINT bytesRead;          // Number of bytes read from the file

static TCHAR  _Path[3];

int32_t SDH_Open_Disk(SDH_T *sdh, uint32_t u32CardDetSrc)
{
    SDH_Open(sdh, u32CardDetSrc);
    if (SDH_Probe(sdh))
    {
        return Fail;
    }

    _Path[1] = ':';
    _Path[2] = 0;
    if (sdh == SDH0)
    {
        _Path[0] = '0';
        f_mount(&_FatfsVolSd0, _Path, 1);
    }
    else
    {
        _Path[0] = '1';
        f_mount(&_FatfsVolSd1, _Path, 1);
    }
    return Successful;
}

void SDH_Close_Disk(SDH_T *sdh)
{
    if (sdh == SDH0)
    {
        memset(&SD0, 0, sizeof(SDH_INFO_T));
        f_mount(NULL, _Path, 1);
        memset(&_FatfsVolSd0, 0, sizeof(FATFS));
    }
    else
    {
        memset(&SD1, 0, sizeof(SDH_INFO_T));
        f_mount(NULL, _Path, 1);
        memset(&_FatfsVolSd1, 0, sizeof(FATFS));
    }
}

void FS_Show_Features(SDH_INFO_T sd) {
	
	switch(sd.CardType) {
		case SDH_TYPE_SD_HIGH:
			printf("SDH Type : SD HIGH \r\n");
			break;
		case SDH_TYPE_SD_LOW:
			printf("SDH Type : SD LOW \r\n");
			break;
		case SDH_TYPE_EMMC:
			printf("SDH Type : EMMC \r\n");
			break;
		case SDH_TYPE_MMC:
			printf("SDH Type : MMC \r\n");
			break;
		case SDH_TYPE_UNKNOWN:
			printf("SDH Type : Uknown \r\n");
			break;
	}

	printf("SDH Disk Size : %d bytes\r\n", sd.diskSize);
	printf("SDH Sector Size : %d\r\n", sd.sectorSize);
	printf("SDH Total Sector Count : %d\r\n", sd.totalSectorN);

}

void FS_Show_Files(const char *dirname, FILINFO *file_array) {
	
	uint8_t index = 0;
	
	// Open the directory
	res = f_opendir(&dir, dirname);
	if (res != FR_OK) {
			// Error handling
			printf("Unmount the file system on error \r\n");
			f_mount(NULL, "", 0); // Unmount the file system on error
			return;
	}
	
	// Read directory contents
	for (;;) {
		res = f_readdir(&dir, &fileInfo);
		if (res != FR_OK || fileInfo.fname[0] == 0) {
				// Error or end of directory
				break;
		}

		// Ignore "." and ".." entries
		if (fileInfo.fname[0] == '.')
				continue;

		// Display file name
		file_array[index] = fileInfo;
		index++;
	}

	// Close the directory
	f_closedir(&dir);
	
}

FRESULT FS_Get_Data(const char *filename, uint8_t * buffer, uint16_t size) {
	
	res = f_open(&file, filename, FA_READ);
	if (res != FR_OK) {
		return res;
	}
	
	uint8_t read_buffer[FILE_READ_BUFFER_SIZE];
	
	do {
		res = f_read(&file, read_buffer, size, &bytesRead);
		if (res != FR_OK) {
			return res;
		}
		for (UINT i = 0; i < bytesRead; i++) {
			buffer[i] = read_buffer[i];
		}
	} while (bytesRead == size);
	
	f_close(&file);
	
	return FR_OK;
	
}

FRESULT FS_Into_Dir(const char *dirname) {
	
	res = f_chdir(dirname);
	if (res != FR_OK) {
		return res;
	}
	
	return FR_OK;
}


/* List contents of a directory */

FRESULT list_dir (const char *path)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int nfile, ndir;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        nfile = ndir = 0;
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Error or end of dir */
            if (fno.fattrib & AM_DIR) {            /* Directory */
                printf("   <DIR>   %s\n", fno.fname);
                ndir++;
            } else {                               /* File */
                printf("%10u %s\n", fno.fsize, fno.fname);
                nfile++;
            }
        }
        f_closedir(&dir);
        printf("%d dirs, %d files.\n", ndir, nfile);
    } else {
        printf("Failed to open \"%s\". (%u)\n", path, res);
    }
    return res;
}

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}
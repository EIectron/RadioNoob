#include "eeprom.h"

TableParams myTableParams;
RCValues myRCvalues;


FMC_RESP setDataFlashBase(uint32_t DFBA) {
	
    uint32_t   config[2];          /* User Configuration */

    /* Read User Configuration 0 & 1 */
    if (FMC_ReadConfig(config, 2) < 0)
    {
        printf("\nRead User Config failed!\n");       /* Error message */
        return FMC_ERR;                     /* failed to read User Configuration */
    }

    /* Check if Data Flash is enabled and is expected address. */
    if ((!(config[0] & 0x1)) && (config[1] == DFBA))
        return FMC_OK;                      /* no need to modify User Configuration */

    FMC_ENABLE_CFG_UPDATE();           /* Enable User Configuration update. */

    config[0] &= ~0x1;             /* Clear CONFIG0 bit 0 to enable Data Flash */
    config[1] = DFBA;           /* Give Data Flash base address  */

    /* Update User Configuration settings. */
    if (FMC_WriteConfig(config, 2) < 0)
        return FMC_ERR;                     /* failed to write user configuration */

    printf("\nSet Data Flash base as 0x%x.\n", DATA_FLASH_BASE_ADDR);  /* debug message */

    /* Perform chip reset to make new User Config take effect. */
    SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;
    return FMC_OK;                          /* success */
}

/**
 * Flash bellege veri yazma islemi yapar.
 * 
 * @param addr Yazma isleminin baslayacagi bellek adresi
 * @param params Bellege yazilacak verileri i�eren yapi
 * @return FMC_RESP Islemin basari durumunu d�nd�r�r: FMC_OK veya FMC_ERR
 */
FMC_RESP TableWriteToFlash(uint32_t addr, TableParams params) {
    // Fonksiyon baslatilirken gerekli kayitlarin kilidi a�ilir
    SYS_UnlockReg();

    // Flash mod�l� a�ilir
    FMC_Open();

    // Belirtilen adresteki bellek b�lgesi silinir
    if (FMC_Erase(addr) != 0) {
        // Bellek silme islemi hata d�nd�rd�g�nde FMC_ERR yaniti d�nd�r�l�r
        return FMC_ERR;
    }

    // Yapidaki her bir 4 byte'lik veri bellege yazilir
    uint32_t u32Index = 0;
    for (u32Index = 0; u32Index < sizeof(TableParams); u32Index += 4) {
        // Bellege yazma islemi ger�eklestirilir
        if (FMC_Write(addr + u32Index, *((uint32_t*)(&params) + u32Index/4)) != 0) {
            // Bellek yazma islemi hata d�nd�rd�g�nde FMC_ERR yaniti d�nd�r�l�r
            return FMC_ERR;
        }
    }

    // Flash mod�l� kapatilir
    FMC_Close();

    // Fonksiyon tamamlandiginda kayitlarin kilitlenmesi saglanir
    SYS_LockReg();

    // Islem basariyla tamamlandiginda FMC_OK yaniti d�nd�r�l�r
    return FMC_OK;
}



/**
 * Flash bellekten veri okuma islemi yapar.
 * 
 * @param addr Okuma isleminin baslayacagi bellek adresi
 * @param params Okunan verilerin kopyalanacagi yapiyi g�steren isaret�i
 * @return FMC_RESP Islemin basari durumunu d�nd�r�r: FMC_OK veya FMC_ERR
 */
FMC_RESP TableReadToFlash(uint32_t addr, TableParams *params) {
    // Fonksiyon baslatilirken gerekli kayitlarin kilidi a�ilir
    SYS_UnlockReg();

    // Flash mod�l� a�ilir
    FMC_Open();

    // Yapilarin boyutu kadar d�ng� olusturulur (4 byte'lik par�alar halinde okuma yapilacak)
    uint32_t u32Index = 0;
    for (u32Index = 0; u32Index < sizeof(TableParams); u32Index += 4) {
        // Adresten okunan degerler, parametre olarak gelen yapiya kopyalanir
        *((uint32_t*)(params) + u32Index/4) = FMC_Read(addr + u32Index);

        // FMC_Read fonksiyonu hata d�nd�rd�g�nde kontrol yapilir
        if (g_FMC_i32ErrCode != 0) {
            // Hata durumunda FMC_ERR yaniti d�nd�r�l�r
            return FMC_ERR;
        }
    }

    // Flash mod�l� kapatilir
    FMC_Close();

    // Fonksiyon tamamlandiginda kayitlarin kilitlenmesi saglanir
    SYS_LockReg();

    // Basarili tamamlanma durumunda FMC_OK yaniti d�nd�r�l�r
    return FMC_OK;
}



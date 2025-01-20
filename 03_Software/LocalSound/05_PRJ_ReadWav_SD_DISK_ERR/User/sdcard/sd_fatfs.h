#ifndef __SD_FATFS_H
#define __SD_FATFS_H

#include "fatfs.h"
#include "user_bsp.h"

#define fatfs_printf Uart1_SendData


#define Auto_Formatting_Enable 0

extern char SDPath[4]; /* SD¿¨Âß¼­Éè±¸Â·¾¶ */

uint8_t FATFS_LinkInit(void);
void FAFTS_ReLinkInit(void);
uint8_t FATFS_ReInit(void);
uint8_t FATFS_Init(FATFS *fs);
void Printf_FATFS_Error(FRESULT fresult);

#endif

    /*****************************END OF FILE**************************/

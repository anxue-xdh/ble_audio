/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    user_diskio.c
 * @brief   This file includes a diskio driver skeleton to be completed by the user.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/*
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future.
 * Kept to ensure backward compatibility with previous CubeMx versions when
 * migrating projects.
 * User code previously added there should be copied in the new user sections before
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"

#include "bsp_spi_sdcard.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;
// 固定只支持blocksize大小为512的卡，兼容大于512的卡时，该卡容量会变小
#define SD_BLOCKSIZE 512 // SDCardInfo.CardBlockSize

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize(BYTE pdrv);
DSTATUS USER_status(BYTE pdrv);
DRESULT USER_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
DRESULT USER_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
DRESULT USER_ioctl(BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef USER_Driver =
    {
        USER_initialize,
        USER_status,
        USER_read,
#if _USE_WRITE
        USER_write,
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
        USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Initializes a Drive
 * @param  pdrv: Physical drive number (0..)
 * @retval DSTATUS: Operation status
 */
DSTATUS USER_initialize(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
    /* USER CODE BEGIN INIT */
    Stat = STA_NOINIT;

    taskENTER_CRITICAL();
    /* Configure the uSD device */
    if (SD_Init() == SD_RESPONSE_NO_ERROR)
    {
        Stat &= ~STA_NOINIT;
    }
    taskEXIT_CRITICAL();
    return Stat;
    /* USER CODE END INIT */
}

/**
 * @brief  Gets Disk Status
 * @param  pdrv: Physical drive number (0..)
 * @retval DSTATUS: Operation status
 */
DSTATUS USER_status(
    BYTE pdrv /* Physical drive number to identify the drive */
)
{
    /* USER CODE BEGIN STATUS */
    Stat &= ~STA_NOINIT;

    return Stat;
    /* USER CODE END STATUS */
}

/**
 * @brief  Reads Sector(s)
 * @param  pdrv: Physical drive number (0..)
 * @param  *buff: Data buffer to store read data
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to read (1..128)
 * @retval DRESULT: Operation result
 */
DRESULT USER_read(
    BYTE pdrv,    /* Physical drive nmuber to identify the drive */
    BYTE *buff,   /* Data buffer to store read data */
    DWORD sector, /* Sector address in LBA */
    UINT count    /* Number of sectors to read */
)
{
    /* USER CODE BEGIN READ */
    DRESULT res = RES_ERROR;
    SD_Error SD_state = SD_RESPONSE_NO_ERROR;

    taskENTER_CRITICAL();
    // SD_state = SD_ReadMultiBlocks(buff, (uint64_t)sector * SD_BLOCKSIZE, SD_BLOCKSIZE, count);
    SD_state = SD_ReadBlock(buff, (uint64_t)sector * SD_BLOCKSIZE, SD_BLOCKSIZE);
    taskEXIT_CRITICAL();
    if (SD_state == SD_RESPONSE_NO_ERROR)
    {
        res = RES_OK;
    }

    return res;
    /* USER CODE END READ */
}

/**
 * @brief  Writes Sector(s)
 * @param  pdrv: Physical drive number (0..)
 * @param  *buff: Data to be written
 * @param  sector: Sector address (LBA)
 * @param  count: Number of sectors to write (1..128)
 * @retval DRESULT: Operation result
 */
#if _USE_WRITE == 1
DRESULT USER_write(
    BYTE pdrv,        /* Physical drive nmuber to identify the drive */
    const BYTE *buff, /* Data to be written */
    DWORD sector,     /* Sector address in LBA */
    UINT count        /* Number of sectors to write */
)
{
    /* USER CODE BEGIN WRITE */
    DRESULT res = RES_ERROR;
    SD_Error SD_state = SD_RESPONSE_NO_ERROR;

    taskENTER_CRITICAL();

    // SD_state = SD_WriteMultiBlocks((uint8_t *)buff, (uint64_t)sector * SD_BLOCKSIZE, SD_BLOCKSIZE, count);
    SD_state = SD_WriteBlock((uint8_t *)buff, (uint64_t)sector * SD_BLOCKSIZE, SD_BLOCKSIZE);
    taskEXIT_CRITICAL();
    if (SD_state == SD_RESPONSE_NO_ERROR)
    {
        res = RES_OK;
    }

    return res;
    /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
 * @brief  I/O control operation
 * @param  pdrv: Physical drive number (0..)
 * @param  cmd: Control code
 * @param  *buff: Buffer to send/receive control data
 * @retval DRESULT: Operation result
 */
#if _USE_IOCTL == 1
DRESULT USER_ioctl(
    BYTE pdrv, /* Physical drive nmuber (0..) */
    BYTE cmd,  /* Control code */
    void *buff /* Buffer to send/receive control data */
)
{
    /* USER CODE BEGIN IOCTL */
    DRESULT status = RES_PARERR;
    taskENTER_CRITICAL();
    switch (cmd)
    {
    // Get R/W sector size (WORD)
    case GET_SECTOR_SIZE:
        *(WORD *)buff = SD_BLOCKSIZE;
        break;
    // Get erase block size in unit of sector (DWORD)
    case GET_BLOCK_SIZE:
        *(DWORD *)buff = 1;
        break;

    case GET_SECTOR_COUNT:
        *(DWORD *)buff = SDCardInfo.CardCapacity / SDCardInfo.CardBlockSize;
        break;
    case CTRL_SYNC:
        break;
    }
    status = RES_OK;
    taskEXIT_CRITICAL();
    return status;
    /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

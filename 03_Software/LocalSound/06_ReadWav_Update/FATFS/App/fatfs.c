/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file   fatfs.c
 * @brief  Code for fatfs applications
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
#include "fatfs.h"


uint8_t retUSER;  /* Return value for USER */
char USERPath[4]; /* USER logical drive path */
FATFS USERFatFS;  /* File system object for USER logical drive */
FIL USERFile;     /* File object for USER */

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
    /*## FatFS: Link the USER driver ###########################*/
    /* USER CODE BEGIN Init */
    FRESULT f_res;
    retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);

    if (retUSER != 0)
    {
        /* 注销一个FatFS设备：SD卡 */
        FATFS_UnLinkDriver(USERPath);

        return;
    }
    // 在SD卡挂载文件系统，文件系统挂载时会对SD卡初始化
    f_res = f_mount(&USERFatFS, (TCHAR const *)USERPath, 1);
    Printf_FATFS_Error(f_res);
    fatfs_printf("\r\r\n****** 挂载文件系统 ******\r\r\n");
    if (f_res != FR_OK)
    {
        fatfs_printf("！！SD卡挂载文件系统失败。(%d)\r\n", f_res);
        Printf_FATFS_Error(f_res);

        return;
    }
    else
    {
        fatfs_printf("》文件系统挂载成功，可以进行读写测试\r\n");
    }

    /* additional user code for init */
    /* USER CODE END Init */
}

/**
 * @brief  Gets Time from RTC
 * @param  None
 * @retval Time in DWORD
 */
DWORD get_fattime(void)
{
    /* USER CODE BEGIN get_fattime */
    return 0;
    /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

/**
 * @brief  打印输出信息
 * @param  无
 * @retval 无
 */
void Printf_FATFS_Error(FRESULT fresult)
{
    switch (fresult)
    {
    case FR_OK:
        fatfs_printf("》操作成功。\r\n");
        break;
    case FR_DISK_ERR:
        fatfs_printf("！！硬件输入输出驱动出错。\r\n");
        break;
    case FR_INT_ERR:
        fatfs_printf("！！断言错误。\r\n");
        break;
    case FR_NOT_READY:
        fatfs_printf("！！物理设备无法工作。\r\n");
        break;
    case FR_NO_FILE:
        fatfs_printf("！！无法找到文件。\r\n");
        break;
    case FR_NO_PATH:
        fatfs_printf("！！无法找到路径。\r\n");
        break;
    case FR_INVALID_NAME:
        fatfs_printf("！！无效的路径名。\r\n");
        break;
    case FR_DENIED:
    case FR_EXIST:
        fatfs_printf("！！拒绝访问。\r\n");
        break;
    case FR_INVALID_OBJECT:
        fatfs_printf("！！无效的文件或路径。\r\n");
        break;
    case FR_WRITE_PROTECTED:
        fatfs_printf("！！逻辑设备写保护。\r\n");
        break;
    case FR_INVALID_DRIVE:
        fatfs_printf("！！无效的逻辑设备。\r\n");
        break;
    case FR_NOT_ENABLED:
        fatfs_printf("！！无效的工作区。\r\n");
        break;
    case FR_NO_FILESYSTEM:
        fatfs_printf("！！无效的文件系统。\r\n");
        break;
    case FR_MKFS_ABORTED:
        fatfs_printf("！！因函数参数问题导致f_mkfs函数操作失败。\r\n");
        break;
    case FR_TIMEOUT:
        fatfs_printf("！！操作超时。\r\n");
        break;
    case FR_LOCKED:
        fatfs_printf("！！文件被保护。\r\n");
        break;
    case FR_NOT_ENOUGH_CORE:
        fatfs_printf("！！长文件名支持获取堆空间失败。\r\n");
        break;
    case FR_TOO_MANY_OPEN_FILES:
        fatfs_printf("！！打开太多文件。\r\n");
        break;
    case FR_INVALID_PARAMETER:
        fatfs_printf("！！参数无效。\r\n");
        break;
    }
}
/* USER CODE END Application */

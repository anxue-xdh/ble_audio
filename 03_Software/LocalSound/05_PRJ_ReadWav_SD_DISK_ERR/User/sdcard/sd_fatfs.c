/**
 ******************************************************************************
 * @file    bsp_led.c
 * @author  fire
 * @version V1.0
 * @date    2018-xx-xx
 * @brief   SPI sd卡测试驱动（不含文件系统）
 ******************************************************************************
 * @attention
 *
 * 实验平台:野火  STM32 F103-MINI 开发板
 * 论坛    :http://www.firebbs.cn
 * 淘宝    :https://fire-stm32.taobao.com
 *
 ******************************************************************************
 */
#include "sd_fatfs.h"
#include "bsp_spi_sdcard.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
extern Diskio_drvTypeDef USER_Driver;

char SDPath[4]; /* SD卡逻辑设备路径 */

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/* FatFs文件系统对象 */
uint8_t FATFS_LinkInit(void)
{
    uint8_t f_res = FATFS_LinkDriver(&USER_Driver, SDPath);
    if (f_res != 0)
    {
        /* 注销一个FatFS设备：SD卡 */
        FATFS_UnLinkDriver(SDPath);
    }
    return f_res;
}

void FAFTS_ReLinkInit(void)
{
    /* 注销一个FatFS设备：SD卡 */
    FATFS_UnLinkDriver(SDPath);
}



uint8_t FATFS_ReInit(void)
{
    if (SDPath == NULL)
        return 1;

    FRESULT f_res; /* 文件操作结果 */
    f_res = f_mount(NULL, (TCHAR const *)SDPath, 1);
    return f_res;
}

uint8_t FATFS_Init(FATFS *fs)
{
    if (SDPath == NULL)
        return 1;

    FRESULT f_res; /* 文件操作结果 */
    // 在SD卡挂载文件系统，文件系统挂载时会对SD卡初始化
    f_res = f_mount(fs, (TCHAR const *)SDPath, 1);
    Printf_FATFS_Error(f_res);
    /* 如果没有文件系统就格式化创建创建文件系统 */
    if (f_res == FR_NO_FILESYSTEM)
    {
        fatfs_printf("》SD卡还没有文件系统.\r\n");
#if Auto_Formatting_Enable
        fatfs_printf("》SD卡还没有文件系统，即将进行格式化...\r\n");
        /* 格式化 */
        f_res = f_mkfs((TCHAR const *)SDPath, 0, 0);

        if (f_res == FR_OK)
        {
            fatfs_printf("》SD卡已成功格式化文件系统。\r\n");
            /* 格式化后，先取消挂载 */
            f_res = f_mount(NULL, (TCHAR const *)SDPath, 1);
            /* 重新挂载	*/
            f_res = f_mount(&fs, (TCHAR const *)SDPath, 1);
        }
        else
        {
            fatfs_printf("《《格式化失败。》》\r\n");
            while (1)
                ;
        }
#endif
    }
    else if (f_res != FR_OK)
    {
        fatfs_printf("SD卡挂载文件系统失败。(%d)\r\n", f_res);
        Printf_FATFS_Error(f_res);
        return 2;
    }
    else
    {
        fatfs_printf("》文件系统挂载成功，可以进行读写测试\r\n");
    }
    return 0;
}

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

/*********************************************END OF FILE**********************/

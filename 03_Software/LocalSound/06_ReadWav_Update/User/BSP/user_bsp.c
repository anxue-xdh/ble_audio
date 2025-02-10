#include "user_bsp.h"

#include "gui.h"
#include "fatfs.h"

char Uart_TxBuf[UART1_MAXLEN];
char Uart1_Buf[UART1_MAXLEN];
int Uart1_Len = 0;
char Uart1_ReveFlag = False;

extern FRESULT SD_Read_FileInfo(const char *path, char (*list)[64]);
extern char USERPath[4]; /* USER logical drive path */

void System_Init(void)
{
    Uart1_Init();

    Tim_Init();

    GUI_Init();

    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

    // hdma_dac_ch1.XferCpltCallback = DAC_DMA_CpCallback;
    // hdma_dac_ch1.XferErrorCallback = DAC_DMA_ErrorCallback;
    // hdma_dac_ch1.XferHalfCpltCallback = DAC_DMA_HaltcpCallback;
    __HAL_DMA_ENABLE_IT(&hdma_dac_ch1, DMA_IT_TC);
    __HAL_DMA_ENABLE_IT(&hdma_dac_ch1, DMA_IT_TE);
    __HAL_DMA_ENABLE_IT(&hdma_dac_ch1, DMA_IT_HT);

    // HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)Uart1_Buf, 2048, DAC_ALIGN_12B_R);

    // SD_Read_FileInfo(USERPath, MusicList);

    Uart1_SendData("程序初始化完毕\r\n"); // 实验程序
}

void Tim_Init()
{
    // TIM6时间定时器初始化
    // HAL_TIM_Base_Start_IT(&htim6);

    HAL_TIM_Base_Start_IT(&htim2);
}

void Uart1_Init()
{
    // UART初始化
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&huart1, (uint8_t *)Uart1_Buf, sizeof(Uart1_Buf));
}

void USART_SendData(UART_HandleTypeDef *huart, char *data, ...)
{
    int len;

    va_list ap;
    va_start(ap, data);
    vsprintf(Uart_TxBuf, data, ap);
    va_end(ap);

    len = strlen(Uart_TxBuf);

    taskENTER_CRITICAL();
    HAL_UART_Transmit(&huart1, (uint8_t *)Uart_TxBuf, len, 20);
    taskEXIT_CRITICAL();
}

// void Uart1_SendData(char *data)
// {
// 	int len = strlen(data);
//     taskENTER_CRITICAL();
//     HAL_UART_Transmit(&huart1, (uint8_t *)data, len, 10);
//     taskEXIT_CRITICAL();
// }

char Uart_Strcmp(char *buf, char *desc)
{
    if (strcmp(buf, desc) == 0)
        return True;
    return False;
}

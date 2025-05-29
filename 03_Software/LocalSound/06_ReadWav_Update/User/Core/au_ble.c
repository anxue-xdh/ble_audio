#include "au_ble.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "user_bsp.h"
#include "i2s.h"
#include "au_decode.h"

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#if (Define_DEBUG == 1)
#define ble_Debug_Print(...) Uart1_SendData(__VA_ARGS__)
#else
#define ble_Debug_Print(...)
#endif

// #define WavBuff_Size 256
#define Ble_Rx_Start(buf, len) HAL_I2S_Receive_DMA(&hi2s2, buf, len)
#define Ble_Rx_Stop() HAL_I2S_DMAStop(&hi2s2)
#define Wav_Start(out, len) HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)out, len, DAC_ALIGN_12B_R)
#define Wav_Stop() HAL_DAC_Stop_DMA(&hdac, DAC1_CHANNEL_1)
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
TaskHandle_t Ble_Task_Handle = NULL;
// int16_t WAV_CH[WavBuff_Size * 2] = {0};
// int16_t WAV_TmpBuf[WavBuff_Size] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* extern variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
extern short ble_rx_buffer[BleBuff_Size];
short ble_dac_buffer[BleBuff_Size] = {0}; // 用于DAC输出的缓冲区
/* USER CODE END EV */

/* extern function prototypes -----------------------------------------------*/
/* USER CODE BEGIN EFP */
void printf_BleInfo(short *data, int len);
/* USER CODE END EFP */

void Start_Ble(void)
{
    if (Ble_Task_Handle != NULL)
    {
        // 获取任务状态
        eTaskState eReturn = eTaskGetState(Ble_Task_Handle);
        ble_Debug_Print("Ble_Task_Handle Task is %d\r\n", eReturn);

        if (eReturn != eDeleted)
        {
            // 任务正常运行，则忽略本次按键命令
            ble_Debug_Print("Ble_Task_Handle is running\r\n");
            return;
        }
    }

    if (HAL_I2S_Receive_DMA(&hi2s2, (uint16_t *)ble_rx_buffer, BleBuff_Size) != HAL_OK)
    {
        // 错误处理
        Error_Handler();
    }


    ble_Debug_Print("create Ble_Task_Handle\r\n");
    // 创建音乐播放任务
    xTaskCreate((TaskFunction_t)Ble_Task,
                (const char *)"Ble_Task",
                (configSTACK_DEPTH_TYPE)2048,
                (void *)NULL,
                (UBaseType_t)5,
                &Ble_Task_Handle);
}

extern void printf_WavInfo(short *data, int len);

void Ble_Task(void const *argument)
{
    u32 xReturn = 0;
    u32 tmp_ret = 0;

    ble_Debug_Print("Ble Task started\r\n");
    while (1)
    {
        // if (dma_rx_complete == 1)
        // {
        //     dma_rx_complete = 0;
        //     printf_WavInfo(ble_rx_buffer, BleBuff_Size);
        // }

        // ble_Debug_Print("Ble Task run\r\n");
        xTaskNotifyWait(0, 0xFFFF, &xReturn, portMAX_DELAY);
        if (xReturn != Ble_Bit_Rx_Compelete)
        {
            if(xReturn == Ble_Bit_Rx_Half)
                // ble_Debug_Print("Ble Half\r\n");
            continue; // 只处理接收完成的通知
        }

        printf_BleInfo(ble_rx_buffer, BleBuff_Size);
        // Wav_Stop();
        // tmp_ret = Data_16to12_Mult(ble_dac_buffer, (short *)ble_rx_buffer, BleBuff_Size);
        // // Wav_Start(ble_dac_buffer, BleBuff_Size*2); // 启动DAC输出

        // printf_BleInfo(ble_dac_buffer, BleBuff_Size);
        // vTaskDelay(100);
    }
    // vTaskDelete(NULL);
}

void printf_BleInfo(short *data, int len)
{
    int j = 0;
    for (int i = 0; i < len; i++)
    {
        ble_Debug_Print("%04d\t", data[i]);
        j++;
        if (j == 8)
        {
            j = 0;
            ble_Debug_Print("\r\n");
        }
    }
}
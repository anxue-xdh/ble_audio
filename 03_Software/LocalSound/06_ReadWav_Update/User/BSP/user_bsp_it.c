#include "user_bsp.h"

#include "bsp_sysTimer.h"
#include "au_os.h"
#include "au_ble.h"

// DMA接收过半完成回调
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    BaseType_t ret = pdFALSE;
    
    xTaskNotifyFromISR(Ble_Task_Handle, Ble_Bit_Rx_Half, eSetBits, &ret);
    if (ret == pdTRUE) // 需要进行任务切换
    {
        portYIELD_FROM_ISR(ret); // 执行任务切换
    }
}

// DMA接收完成回调
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    dma_rx_complete = 1;

     BaseType_t ret = pdFALSE;

    xTaskNotifyFromISR(Ble_Task_Handle, Ble_Bit_Rx_Compelete, eSetBits, &ret);
    if (ret == pdTRUE) // 需要进行任务切换
    {
        portYIELD_FROM_ISR(ret); // 执行任务切换
    }
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    BaseType_t ret = pdFALSE;
    // xSemaphoreGiveFromISR(Sem_Uart1, &xHigherPriorityTaskWoken); // 给出信号量
    // vTaskNotifyGiveFromISR(Wav_Task_Handle, &ret);
    xTaskNotifyFromISR(Wav_Task_Handle, Wav_PlayBit_DacNotify, eSetBits, &ret);
    if (ret == pdTRUE) // 需要进行任务切换
    {
        portYIELD_FROM_ISR(ret); // 执行任务切换
    }
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    // HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

    BaseType_t ret = pdFALSE;
    // xSemaphoreGiveFromISR(Sem_Uart1, &xHigherPriorityTaskWoken); // 给出信号量
    // vTaskNotifyGiveFromISR(Wav_Task_Handle, &ret);
    xTaskNotifyFromISR(Wav_Task_Handle, Wav_PlayBit_DacNotify, eSetBits, &ret);
    if (ret == pdTRUE) // 需要进行任务切换
    {
        portYIELD_FROM_ISR(ret); // 执行任务切换
    }
}

void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
    HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
    // Uart1_SendData("[ERROR] DMA Fransfer Error");
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
}

// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
// {
// 	static uint16_t Timer6_Cnt = 0;
// 	if (htim->Instance == TIM6)
// 	{
// 		Timer6_Cnt++;
// 		if (Timer6_Cnt >= 8000)
// 		{
// 			Timer6_Cnt = 0;
// 			HAL_GPIO_TogglePin(GPIOC, LED1_Pin);
// 		}

// 		SysTimer_Rtc_IRQ();
// 	}
// }

extern int16_t sin_array[];

int16_t dac = 0;
extern uint32_t FreeRTOS_RunTimeTicks;
void USER_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
    }
    else if (htim->Instance == TIM7)
    {
        FreeRTOS_RunTimeTicks++;
    }
    else if (htim->Instance == TIM2)
    {
        // SysTimer_Rtc_IRQ();
        // tmp ++;
        // if (tmp >= 32)
        //     tmp = 0;
        // dac = sin_array[tmp]+2048;
        // HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac);
    }
}

extern DMA_HandleTypeDef hdma_usart1_rx;
extern TaskHandle_t Uart1_Scan_Task_Handle;
// extern SemaphoreHandle_t Sem_Uart1;
//  该函数应当放在USART1_IRQHandler()中
void Uart1_Reve_Callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) != RESET)
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            __HAL_UART_CLEAR_IDLEFLAG(huart);

            HAL_UART_DMAStop(huart);
            Uart1_Len = UART1_MAXLEN - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
            HAL_UART_Receive_DMA(&huart1, (uint8_t *)Uart1_Buf, sizeof(Uart1_Buf));

            Uart1_Buf[Uart1_Len] = '\0';
            Uart1_Len = 0;
            Uart1_ReveFlag = True;

            // xSemaphoreGiveFromISR(Sem_Uart1, &xHigherPriorityTaskWoken); // 给出信号量
            vTaskNotifyGiveFromISR(Uart1_Scan_Task_Handle, &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken == pdTRUE) // 需要进行任务切换
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // 执行任务切换
            }
        }
    }
}

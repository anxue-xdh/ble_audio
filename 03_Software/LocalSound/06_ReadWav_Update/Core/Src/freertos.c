/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "user_bsp.h"

#include "bsp_spi_sdcard.h"
#include "fatfs.h"

// #include "GUI.h"

#include "core.h"
#include "audio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
TaskHandle_t Led_Ctr_Task_Handle;
TaskHandle_t Led_Test_Task_Handle;
TaskHandle_t Key_Scan_Task_Handle;
TaskHandle_t Key_Run_Task_Handle;
TaskHandle_t Uart1_Scan_Task_Handle;
TaskHandle_t Temp_Task_Handle;
TaskHandle_t ReadWav_Task_Handle;

TaskHandle_t GUI_Task_Handle;
TaskHandle_t GUI_Touch_Handle;
// SemaphoreHandle_t Sem_Uart1 = NULL;

/* USER CODE END Variables */
/* Definitions for SysInitTask */
osThreadId_t SysInitTaskHandle;
const osThreadAttr_t SysInitTask_attributes = {
    .name = "SysInitTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Led_Ctr_Task(void const *argument);
void Led_Test_Task(void const *argument);
void Temp_Task(void const *argument);

extern void GUI_Task(void const *argument);
extern void Touch_Task(void *parameter);
/* USER CODE END FunctionPrototypes */

void Sys_Init_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
uint32_t FreeRTOS_RunTimeTicks = 0;
__weak void configureTimerForRunTimeStats(void)
{
    HAL_TIM_Base_Start_IT(&htim7);
    FreeRTOS_RunTimeTicks = 0;
}

__weak unsigned long getRunTimeCounterValue(void)
{
    return FreeRTOS_RunTimeTicks;
}
/* USER CODE END 1 */

/* USER CODE BEGIN PREPOSTSLEEP */
__weak void PreSleepProcessing(uint32_t *ulExpectedIdleTime)
{
    /* place for user code */
    // Uart1_SendData("Entry TickLess Mode\r\n");
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
    __HAL_RCC_GPIOD_CLK_DISABLE();
    __HAL_RCC_GPIOE_CLK_DISABLE();
    __HAL_RCC_GPIOF_CLK_DISABLE();
    __HAL_RCC_GPIOG_CLK_DISABLE();
    //__HAL_RCC_USART1_CLK_DISABLE();
}

__weak void PostSleepProcessing(uint32_t *ulExpectedIdleTime)
{
    /* place for user code */
    // Uart1_SendData("Exit TickLess Mode\r\n");
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    //__HAL_RCC_USART1_CLK_ENABLE();
}
/* USER CODE END PREPOSTSLEEP */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
    /* USER CODE BEGIN Init */
    System_Init();
    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    // sd_test_func();
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    // Sem_Uart1 = xSemaphoreCreateBinary();
    // assert_param(Sem_Uart1 != NULL);   //断言 判断Sem_Uart1是否创建成功
    /* USER CODE END RTOS_SEMAPHORES */

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* creation of SysInitTask */
    SysInitTaskHandle = osThreadNew(Sys_Init_Task, NULL, &SysInitTask_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    EvenGroup_Key_Handle = xEventGroupCreate();
    assert_param(EvenGroup_Key_Handle != NULL); // 断言
                                                /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_Sys_Init_Task */
/**
 * @brief  Function implementing the SysInitTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_Sys_Init_Task */
void Sys_Init_Task(void *argument)
{
    /* USER CODE BEGIN Sys_Init_Task */
    BaseType_t xReturn = pdPASS;
    taskENTER_CRITICAL();

    // xTaskCreate((TaskFunction_t)Led_Ctr_Task,
    //             (const char *)"Led_Ctr_Task",
    //             (configSTACK_DEPTH_TYPE)64,
    //             (void *)NULL,
    //             (UBaseType_t)2,
    //             (TaskHandle_t *)&Led_Ctr_Task_Handle);

    // xTaskCreate((TaskFunction_t)Led_Test_Task,
    //             (const char *)"Led_Test_Task",
    //             (configSTACK_DEPTH_TYPE)64,
    //             (void *)NULL,
    //             (UBaseType_t)2,
    //             (TaskHandle_t *)&Led_Test_Task_Handle);

    xTaskCreate((TaskFunction_t)Key_Scan_Task,
                (const char *)"Key_Scan_Task",
                (configSTACK_DEPTH_TYPE)64,
                (void *)NULL,
                (UBaseType_t)2,
                (TaskHandle_t *)&Key_Scan_Task_Handle);

    xTaskCreate((TaskFunction_t)Key_Run_Task,
                (const char *)"Key_Run_Task",
                (configSTACK_DEPTH_TYPE)128,
                (void *)NULL,
                (UBaseType_t)2,
                (TaskHandle_t *)&Key_Run_Task_Handle);

    xTaskCreate((TaskFunction_t)Uart1_Scan_Task,
                (const char *)"Uart1_Scan_Task",
                (configSTACK_DEPTH_TYPE)128,
                (void *)NULL,
                (UBaseType_t)5,
                (TaskHandle_t *)&Uart1_Scan_Task_Handle);

    // xTaskCreate((TaskFunction_t)Temp_Task,
    //             (const char *)"Temp_Task",
    //             (configSTACK_DEPTH_TYPE)1024,
    //             (void *)NULL,
    //             (UBaseType_t)5,
    //             (TaskHandle_t *)&Temp_Task_Handle);

    xTaskCreate((TaskFunction_t)ReadWav,
                (const char *)"ReadWav",
                (configSTACK_DEPTH_TYPE)2048,
                (void *)NULL,
                (UBaseType_t)1,
                &ReadWav_Task_Handle);

    xReturn = xTaskCreate((TaskFunction_t)GUI_Task,
                          (const char *)"GUI_Task",
                          (configSTACK_DEPTH_TYPE)256,
                          (void *)NULL,
                          (UBaseType_t)2,
                          (TaskHandle_t *)&GUI_Task_Handle);
    if (pdPASS == xReturn)
        Uart1_SendData("Sys_Init_Task successful\r\n");

    //    xReturn = xTaskCreate((TaskFunction_t)Touch_Task,
    //                          (const char *)"Touch_Task",
    //                          (configSTACK_DEPTH_TYPE)512,
    //                          (void *)NULL,
    //                          (UBaseType_t)2,
    //                          (TaskHandle_t *)&GUI_Touch_Handle);
    //    if (pdPASS == xReturn)
    //        Uart1_SendData("Touch_Task successful\r\n");

    taskEXIT_CRITICAL();
    vTaskDelete(NULL);
    /* USER CODE END Sys_Init_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
BYTE ReadBuffer[512] = {0}; /* ???? */
void Temp_Task(void const *argument)
{
    FRESULT f_res; /* ?????? */
    FIL file;      /* ???? */
    UINT fnum;     /* ???????? */

    while (1)
    {

        Uart1_SendData("\r\n****** 这是一个SD卡 文件系统实验 ******\r\n");

        Uart1_SendData("****** 即将进行文件写入测试... ******\r\n");

        char tempfilepath[60];
        Uart1_SendData("SDPath:%s\r\n", USERPath);
        // sprintf(tempfilepath, "%s%s", USERPath, "INeverForget.wav"); // 拼接出带逻辑驱动器名的完整路径名
        sprintf(tempfilepath, "%s%s", USERPath, "NiHao.wav"); // 拼接出带逻辑驱动器名的完整路径名
        // sprintf(tempfilepath, "%s%s", USERPath, "FatFs.txt");       // 拼接出带逻辑驱动器名的完整路径名
        Uart1_SendData("%s\r\n", tempfilepath);

        /*------------------- 文件系统测试：读测试 ------------------------------------*/
        Uart1_SendData("****** 即将进行文件读取测试... ******\r\n");
        f_res = f_open(&file, tempfilepath, FA_OPEN_EXISTING | FA_READ);
        Uart1_SendData("f_res:%d\r\n", f_res);
        if (f_res == FR_OK)
        {
            Uart1_SendData("》打开文件成功。\r\n");
            while (1)
            {
                f_res = f_read(&file, ReadBuffer, sizeof(ReadBuffer), &fnum);
                if (f_res == FR_OK)
                {
                    Uart1_SendData("》文件读取成功,读到字节数据：%d\r\n", fnum);
                    if (0 == fnum)
                    {
                        vTaskDelay(HAL_MAX_DELAY);
                        break;
                    }

                    int j = 0;
                    for (int i = 0; i < fnum; i++)
                    {
                        Uart1_SendData("%02x ", ReadBuffer[i]);
                        j++;
                        if (j == 16)
                        {
                            j = 0;
                            Uart1_SendData("\r\n");
                        }
                    }
                    vTaskDelay(200);
                }
                else
                {
                    Uart1_SendData("！！文件读取失败：(%d)\r\n", f_res);
                    break;
                }
            }
        }
        else
        {
            Uart1_SendData("！！打开文件失败。\r\n");
            break;
        }
        vTaskDelay(HAL_MAX_DELAY);
    }
    /* 不再读写，关闭文件 */
    f_close(&file);
    vTaskDelete(NULL);
}

void Led_Ctr_Task(void const *argument)
{
    while (1)
    {
        // HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
        vTaskDelay(25);
    }
    // TaskDelete(NULL);
}

void Led_Test_Task(void const *argument)
{
    while (1)
    {
        // HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
        vTaskDelay(1000);
    }
    // vTaskDelete(NULL);
}

/* USER CODE END Application */

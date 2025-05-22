/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "dac.h"
#include "dma.h"
#include "i2s.h"
#include "fatfs.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "user_bsp.h"
#include "bsp_spi_sdcard.h"
#include "sd_fatfs.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char temp = 0;
extern int16_t sin_array[];

// extern Diskio_drvTypeDef USER_Driver;

// FATFS fs;                    /* FatFs文件系统对象 */
// FIL file;                    /* 文件对象 */
// FRESULT f_res;               /* 文件操作结果 */
// UINT fnum;                   /* 文件成功读写数量 */
// BYTE ReadBuffer[1024] = {0}; /* 读缓冲区 */
// BYTE WriteBuffer[] = "欢迎使用野火STM32开发板 今天是个好日子，新建文件系统测试文件\r\n";

// void sd_test_func(void)
// {
//     Uart1_SendData("\r\r\n****** 这是一个SD卡 文件系统实验 ******\r\r\n");

//     if (FATFS_LinkDriver(&USER_Driver, SDPath) != 0)
//     {
//         /* 注销一个FatFS设备：SD卡 */
//         FATFS_UnLinkDriver(SDPath);

//         return;
//     }

//     // 在SD卡挂载文件系统，文件系统挂载时会对SD卡初始化
//     f_res = f_mount(&fs, (TCHAR const *)SDPath, 1);
//     Printf_FATFS_Error(f_res);

//     /*----------------------- 格式化测试 ---------------------------*/
//     /* 如果没有文件系统就格式化创建创建文件系统 */
//     if (f_res == FR_NO_FILESYSTEM)
//     {
//         Uart1_SendData("》SD卡还没有文件系统，即将进行格式化...\r\n");
//         /* 格式化 */
//         f_res = f_mkfs((TCHAR const *)SDPath, 0, 0);

//         if (f_res == FR_OK)
//         {
//             Uart1_SendData("》SD卡已成功格式化文件系统。\r\n");
//             /* 格式化后，先取消挂载 */
//             f_res = f_mount(NULL, (TCHAR const *)SDPath, 1);
//             /* 重新挂载	*/
//             f_res = f_mount(&fs, (TCHAR const *)SDPath, 1);
//         }
//         else
//         {
//             Uart1_SendData("《《格式化失败。》》\r\n");
//             while (1)
//                 ;
//         }
//     }
//     else if (f_res != FR_OK)
//     {
//         Uart1_SendData("！！SD卡挂载文件系统失败。(%d)\r\n", f_res);
//         Printf_FATFS_Error(f_res);
//         while (1)
//             ;
//     }
//     else
//     {
//         Uart1_SendData("》文件系统挂载成功，可以进行读写测试\r\n");
//     }

//     /*----------------------- 文件系统测试：写测试 -----------------------------*/
//     /* 打开文件，如果文件不存在则创建它 */
//     Uart1_SendData("****** 即将进行文件写入测试... ******\r\n");

//     char tempfilepath[60];
//     Uart1_SendData("SDPath:%s\r\n", SDPath);
//     sprintf(tempfilepath, "%s%s", SDPath, "FatFs.txt"); // 拼接出带逻辑驱动器名的完整路径名
//     Uart1_SendData("%s\r\n", tempfilepath);

//     f_res = f_open(&file, tempfilepath, FA_CREATE_ALWAYS | FA_WRITE);
//     if (f_res == FR_OK)
//     {
//         Uart1_SendData("》打开/创建FatFs读写测试文件.txt文件成功，向文件写入数据。\r\n");
//         /* 将指定存储区内容写入到文件内 */
//         f_res = f_write(&file, WriteBuffer, sizeof(WriteBuffer), &fnum);
//         if (f_res == FR_OK)
//         {
//             Uart1_SendData("》文件写入成功，写入字节数据：%d\r\n", fnum);
//             Uart1_SendData("》向文件写入的数据为：\r\n%s\r\n", WriteBuffer);
//         }
//         else
//         {
//             Uart1_SendData("！！文件写入失败：(%d)\r\n", f_res);
//         }
//         /* 不再读写，关闭文件 */
//         f_close(&file);
//     }
//     else
//     {
//         Printf_FATFS_Error(f_res);
//         Uart1_SendData("！！打开/创建文件失败。\r\n");
//     }

//     /*------------------- 文件系统测试：读测试 ------------------------------------*/
//     Uart1_SendData("****** 即将进行文件读取测试... ******\r\n");
//     f_res = f_open(&file, tempfilepath, FA_OPEN_EXISTING | FA_READ);
//     if (f_res == FR_OK)
//     {
//         Uart1_SendData("》打开文件成功。\r\n");
//         f_res = f_read(&file, ReadBuffer, sizeof(ReadBuffer), &fnum);
//         if (f_res == FR_OK)
//         {
//             Uart1_SendData("》文件读取成功,读到字节数据：%d\r\n", fnum);
//             Uart1_SendData("》读取得的文件数据为：\r\n%s \r\n", ReadBuffer);
//         }
//         else
//         {
//             Uart1_SendData("！！文件读取失败：(%d)\r\n", f_res);
//         }
//     }
//     else
//     {
//         Uart1_SendData("！！打开文件失败。\r\n");
//     }
//     /* 不再读写，关闭文件 */
//     f_close(&file);

//     /* 不再使用，取消挂载 */
//     f_res = f_mount(NULL, (TCHAR const *)SDPath, 1);

//     /* 注销一个FatFS设备：SD卡 */
//     FATFS_UnLinkDriver(SDPath);
// }

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2S2_Init();
  MX_USART1_UART_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_DAC_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */

  // Data_16to12_Mult(output_12_array, ucDataBlock, 2048);

  // HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)sin_array, 100, DAC_ALIGN_12B_R);
  // HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)output_12_array, 2048, DAC_ALIGN_12B_R);

  // WritetoSD(WriteBuffer, sizeof(WriteBuffer));
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  USER_TIM_PeriodElapsedCallback(htim);
  /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    Uart1_SendData("Error Handler");
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  Uart1_SendData("Wrong parameters value: file %s on line %d\r\n", file, line);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

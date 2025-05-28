#ifndef __BSP_H
#define __BSP_H

#include "user_bsp_config.h"

#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "dac.h"
#include "i2s.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"

/*******    Init    **********/
void System_Init(void);
void LCD_Init(void);
void Tim_Init(void);
void Uart1_Init(void);
/*******    Init    **********/

/*******    Uart    **********/
extern char Uart1_Buf[UART1_MAXLEN];
extern int Uart1_Len;
extern char Uart1_ReveFlag;

//__VA_ARGS__不可被省略，必须增加一个参数，##__VA_ARGS__可以被省略
#define Log(data, ...) USART_SendData(&huart1, data, ##__VA_ARGS__)
#define Uart1_SendData(data, ...) USART_SendData(&huart1, data, ##__VA_ARGS__)
#define Uart1_strcmp(desc) Uart_Strcmp(Uart1_Buf, desc)

void Uart1_Scan_Task(void);
void Uart1_Reve_Callback(UART_HandleTypeDef *huart);
// void Uart1_SendData(char *data);
void USART_SendData(UART_HandleTypeDef *huart, char *data, ...);
char Uart_Strcmp(char *buf, char *desc);
/*******    Uart    **********/

/*******    Key    **********/
extern EventGroupHandle_t EvenGroup_Key_Handle;

#define Key1Read() HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin)
#define Key2Read() HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin)
#define Key1_Bit 0x01
#define Key2_Bit 0x02

void Key_Run_Task(void);
void Key_Scan_Task(void);
/*******    Key    **********/

void USER_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

/*******    i2s dma    **********/
extern volatile uint8_t dma_rx_complete;

#endif

#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sntp.h"

#include "bsp_uart.h"

#include "string.h"
#include "stdio.h"

// 非必要的头文件
// #include "ble_gatt_server.h" //用于提示上位机，下位机未回复

#define TXD_PIN (GPIO_NUM_19)
#define RXD_PIN (GPIO_NUM_18)

#define UART_RECV_BIT BIT0
#define RX_BUF_SIZE 1024

#define ACK_WAITING_TIME 1000 // 等待响应的最大允许时间
#define RETRY_MAX_NUM 3
#define TX_NONE_CMD 0x00
// 使用时，通过malloc申请
// 在处理完毕后一定要记得free
typedef struct
{
    char *param;
    int len;
    char cmd;
} Uart_Tx_Msg;

typedef struct
{
    char *param; // 保留
    int len;     // 保留
    char cmd;    // 目前仅使用cmd位
} Uart_Rx_Ack_Msg;

extern Uart_Rx_Ack_Msg uart_rx_ack_msg;
extern SemaphoreHandle_t uart_rx_ack_record_semaphore;
extern QueueHandle_t uart_tx_queue;
#define uart_tx_sendData_queue(data, len) uart_tx_sendData_Hex_queue(data, len, 0x00)

void uart_init(int baud_rate);
// void uart_tx_sendData_queue(char *data, int len);
void uart_tx_sendData_Hex_queue(char *data, int len, char cmd);
void uart_tx_SendData(const char *data);
void uart_tx_sendData_Hex(const char *data, int len);

void uart_tx_wait_response_task(void *pt);       // (未被使用,保留)外部实现等待响应功能
void uart_rx_info_analysis(char *data, int len); // 分析函数接受到的数据
void uart_rx_info_analysis_task(char *str);      // 需要外部实现该函数
void uart_tx_msg_free(Uart_Tx_Msg *msg);
int uart_strcmp(char *str);

#endif

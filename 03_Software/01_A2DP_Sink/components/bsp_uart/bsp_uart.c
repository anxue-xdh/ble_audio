#include "bsp_uart.h"

char rx_buffer[RX_BUF_SIZE + 1];
QueueHandle_t uart_tx_queue;

Uart_Rx_Ack_Msg uart_rx_ack_msg;                // 回复响应中的各项参数
SemaphoreHandle_t uart_rx_ack_record_semaphore; // 互斥锁，保证uart_rx_ack_msg不会被意外改变

static void uart_rx_task(void *pt); // 内部已实现
static void uart_tx_task(void *pt); // 内部已实现（无重发功能），如需等待响应，需要外部重写该函数
static uint32_t gettime_ms();

static void uart_rx_task(void *pt)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);

    while (1)
    {
        // uart read bytes 中的第4个参数ticks to wait代表最长阻塞时间
        const int rxBytes = uart_read_bytes(UART_NUM_1, rx_buffer, RX_BUF_SIZE, 10 / portTICK_PERIOD_MS);
        if (rxBytes > 0)
        {
            rx_buffer[rxBytes] = 0;

            // ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, rx_buffer);
            // ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, rx_buffer, rxBytes, ESP_LOG_INFO);

            uart_rx_info_analysis(rx_buffer, rxBytes);
            // xTaskCreate(uart_rx_info_analysis_task, "rx info", 2048, rx_buffer, 2, NULL);
        }
    }
    vTaskDelete(NULL);
}

static void uart_tx_task(void *pt)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);

    Uart_Tx_Msg *msg = NULL;
    TickType_t time;
    int8_t recvFlag = 0;
    int8_t retryNum = 0;
    uint8_t cmdValue = 0;
    while (1)
    {
        // 获取需要发送的数据
        if (xQueueReceive(uart_tx_queue, (void *)&msg, (portTickType)portMAX_DELAY))
        {
            //  发送数据
            uart_write_bytes(UART_NUM_1, (const char *)msg->param, msg->len);
            ESP_LOGI(TX_TASK_TAG, "[uart] uart write_len %d", msg->len);

            // 释放空间
            uart_tx_msg_free(msg);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);
}

void uart_init(int baud_rate)
{
    // baud_rate需要进行检验，是否是正确的波特率，之后在写
    ESP_LOGI("UART_TAG", "uart init start");

    const uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.

    // install uart driver
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);

    // init uart config param
    uart_param_config(UART_NUM_1, &uart_config);

    vTaskDelay(100 / portTICK_PERIOD_MS);
    // set Tx Rx Rts Cts pin
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    uart_tx_queue = xQueueCreate(10, sizeof(Uart_Tx_Msg *));
    uart_rx_ack_record_semaphore = xSemaphoreCreateMutex();

    ESP_LOGI("UART_TAG", "uart init complete");

    // xTaskCreate(uart_rx_task, "uart rx task", 4096, NULL, 3, NULL);
    // xTaskCreate(uart_tx_task, "uart tx task", 2048, NULL, 3, NULL);

    ESP_LOGI("UART_TAG", "uart init task create");
}

void uart_tx_sendData_Hex_queue(char *data, int len, char cmd)
{
    // 申请存储空间
    Uart_Tx_Msg *msg = (Uart_Tx_Msg *)malloc(sizeof(Uart_Tx_Msg));
    if (msg == NULL)
        return;
    msg->param = (char *)malloc(len * sizeof(char));
    if (msg->param == NULL) // 申请失败
    {
        free(msg);
        return;
    }

    memcpy(msg->param, data, len);
    msg->len = len;
    msg->cmd = cmd;

    // 发送msg本身的地址
    xQueueSend(uart_tx_queue, &msg, portMAX_DELAY);
}

void uart_tx_SendData(const char *data)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);

    const int len = strlen(data);
    uart_write_bytes(UART_NUM_1, data, len);

    ESP_LOGI(TX_TASK_TAG, "%s", data);
}

void uart_tx_sendData_Hex(const char *data, int len)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);

    uart_write_bytes(UART_NUM_1, data, len);

    esp_log_buffer_hex(TX_TASK_TAG, data, len);
    // ESP_LOGI(TX_TASK_TAG, "Wrote %d bytes", txBytes);
}

void uart_tx_msg_free(Uart_Tx_Msg *msg)
{
    if (msg)
    {
        if (msg->param)
        {
            free(msg->param);
            msg->param = NULL;
        }
        free(msg);
        msg = NULL;
    }
}

int uart_strcmp(char *str)
{
    if (strcmp(rx_buffer, str) == 0)
        return 1;
    return 0;
}

static uint32_t gettime_ms()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000 + now.tv_usec / 1000;
}
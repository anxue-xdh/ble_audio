#include "user_bsp.h"
#include "ff.h"

#include "gui.h"
#include "core.h"

// extern SemaphoreHandle_t Sem_Uart1;
void SoftReset(void)
{
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}

/******     临时         *****/
extern FRESULT SD_Read_FileInfo(const char *path, char (*list)[64]);
extern char USERPath[4]; /* USER logical drive path */
extern TaskHandle_t ReadWav_Task_Handle;
extern TaskHandle_t GUI_Task_Handle;
/******     临时         *****/

char strTmp[500];
void Uart1_Scan_Task(void)
{
    while (1)
    {
        // if (xSemaphoreTake(Sem_Uart1, HAL_MAX_DELAY) == pdTRUE)
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 1)
            Uart1_SendData("[ERROR] Uart1 obliterated Data!!");

        Uart1_SendData("[DEBUG] %s\r\n", Uart1_Buf);
        if (Uart1_strcmp("Test"))
        {
            ;
        }
        else if (Uart1_strcmp("Query task time"))
        {
            // 任务时间占用
            vTaskGetRunTimeStats(strTmp);
            Uart1_SendData(strTmp);
        }
        else if (Uart1_strcmp("Query task resource"))
        {
            // 任务资源占用
            vTaskList(strTmp);
            Uart1_SendData(strTmp);
        }
        else if (Uart1_strcmp("soft reset"))
        {
            Uart1_SendData("soft reset");
            SoftReset();
        }
        else if (Uart1_strcmp("sd read"))
        {
            SD_Read_FileInfo(USERPath, MusicList);
        }
        else if (Uart1_strcmp("music start"))
        {
            if (ReadWav_Task_Handle != NULL)
            {
                eTaskState eReturn = eTaskGetState(ReadWav_Task_Handle);
                Uart1_SendData("ReadWav_Task_Handle Task is %d\r\n", eReturn);

                if (eReturn != eDeleted)
                {
                    Uart1_SendData("ReadWav_Task_Handle is running\r\n");
                    continue;
                }
            }
            // xTaskNotifyGive(ReadWav_Task_Handle);
            Uart1_SendData("create ReadWav_Task_Handle\r\n");
            xTaskCreate((TaskFunction_t)ReadWav,
                        (const char *)"ReadWav",
                        (configSTACK_DEPTH_TYPE)2048,
                        (void *)MusicList[MusicList_Pointer],
                        (UBaseType_t)5,
                        &ReadWav_Task_Handle);
        }
        else if (Uart1_strcmp("music task query"))
        {
            if (ReadWav_Task_Handle != NULL)
            {
                eTaskState eReturn = eTaskGetState(ReadWav_Task_Handle);
                Uart1_SendData("ReadWav_Task_Handle Task is %d\r\n", eReturn);
            }
            else
            {
                Uart1_SendData("Handle is NULL!!\r\n");
            }
        }
    }
    // vTaskDelete(NULL);
}

EventGroupHandle_t EvenGroup_Key_Handle = NULL;

void Key_Run_Task(void)
{
    EventBits_t keyBit;
    while (1)
    {
        keyBit = xEventGroupWaitBits(EvenGroup_Key_Handle,
                                     Key1_Bit | Key2_Bit,
                                     pdTRUE,
                                     pdFALSE,
                                     portMAX_DELAY);
        switch (keyBit)
        {
        case Key1_Bit:
            xTaskNotify(GUI_Task_Handle, GUI_TaskBit_Key_Up, eSetBits);
            HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
            break;
        case Key2_Bit:
            HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

            if (ReadWav_Task_Handle != NULL)
            {
                eTaskState eReturn = eTaskGetState(ReadWav_Task_Handle);
                Uart1_SendData("ReadWav_Task_Handle Task is %d\r\n", eReturn);

                if (eReturn != eDeleted)
                {
                    Uart1_SendData("ReadWav_Task_Handle is running\r\n");
                    continue;
                }
            }
            // xTaskNotifyGive(ReadWav_Task_Handle);
            Uart1_SendData("create ReadWav_Task_Handle\r\n");
            xTaskCreate((TaskFunction_t)ReadWav,
                        (const char *)"ReadWav",
                        (configSTACK_DEPTH_TYPE)2048,
                        (void *)MusicList[MusicList_Pointer],
                        (UBaseType_t)5,
                        &ReadWav_Task_Handle);
            break;
        }
    }
    // vTaskDelete(NULL);
}

char Key_Reg = 0x00;
void Key_Scan_Task(void)
{
    while (1)
    {
        if ((Key1Read() == RESET) && (!(Key_Reg & Key1_Bit)))
        {
            xEventGroupSetBits(EvenGroup_Key_Handle, Key1_Bit);
            Key_Reg |= Key1_Bit;
        }
        if ((Key2Read() == RESET) && (!(Key_Reg & Key2_Bit)))
        {
            xEventGroupSetBits(EvenGroup_Key_Handle, Key2_Bit);
            Key_Reg |= Key2_Bit;
        }
        if ((Key1Read() == SET) && (Key_Reg & Key1_Bit))
            Key_Reg &= ~Key1_Bit;
        if ((Key2Read() == SET) && (Key_Reg & Key2_Bit))
            Key_Reg &= ~Key2_Bit;
        vTaskDelay(20);
    }
    // vTaskDelete(NULL);
}

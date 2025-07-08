#include "user_bsp.h"
#include "ff.h"

#include "gui.h"
#include "au_os.h"
#include "beta_test_framework.h"
#include "sd_card_recovery.h"
#include "audio_buffer_manager.h"
#include "beta_test_init.h"

// extern SemaphoreHandle_t Sem_Uart1;
void SoftReset(void)
{
    __set_FAULTMASK(1);
    NVIC_SystemReset();
}

/******     ��ʱ         *****/
extern char USERPath[4]; /* USER logical drive path */
/******     ��ʱ         *****/

char strTmp[500];
void Uart1_Scan_Task(void)
{
    while (1)
    {
        // Enhanced notification handling with overflow detection
        uint32_t notification_count = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (notification_count > 1) {
            BetaTest_IncrementCoreObliteratedErrors();
            Uart1_SendData("[ERROR] Uart1 obliterated Data!! Count: %lu (Monitored by Beta Test)\r\n", notification_count);
        }

        Uart1_SendData("[DEBUG] %s\r\n", Uart1_Buf);
        if (Uart1_strcmp("Test"))
        {
            ;
        }
        else if (Uart1_strcmp("Query task time"))
        {
            // ����ʱ��ռ��
            vTaskGetRunTimeStats(strTmp);
            Uart1_SendData(strTmp);
        }
        else if (Uart1_strcmp("Query task resource"))
        {
            // ������Դռ��
            vTaskList(strTmp);
            Uart1_SendData(strTmp);
        }
        else if (Uart1_strcmp("soft reset"))
        {
            Uart1_SendData("soft reset");
            SoftReset();
        }
        // Beta testing framework commands
        else if (Uart1_strcmp("beta test init"))
        {
            Uart1_SendData("Initializing beta test framework...\r\n");
            TestResult_t result = BetaTest_Init();
            if (result == TEST_OK) {
                Uart1_SendData("Beta test framework initialized successfully\r\n");
            } else {
                Uart1_SendData("Failed to initialize beta test framework\r\n");
            }
        }
        else if (Uart1_strcmp("beta test run"))
        {
            Uart1_SendData("Running comprehensive test suite...\r\n");
            TestResult_t result = BetaTest_RunFullSuite();
            if (result == TEST_OK) {
                Uart1_SendData("Test suite completed successfully\r\n");
            } else {
                Uart1_SendData("Test suite completed with errors\r\n");
            }
        }
        else if (Uart1_strcmp("beta test report"))
        {
            BetaTest_GenerateReport();
        }
        else if (Uart1_strcmp("beta test reset"))
        {
            BetaTest_ResetStats();
            Uart1_SendData("Beta test statistics reset\r\n");
        }
        // SD card recovery commands
        else if (Uart1_strcmp("sd health"))
        {
            SDCard_GenerateHealthReport();
        }
        else if (Uart1_strcmp("sd stats reset"))
        {
            SDCard_ResetStats();
        }
        // Audio buffer manager commands
        else if (Uart1_strcmp("audio health"))
        {
            AudioBuffer_GenerateHealthReport();
        }
        else if (Uart1_strcmp("sd read"))
        {
            MicList_Update(USERPath);
        }
        else if (Uart1_strcmp("micList reinit"))
        {
            MicList_reinit();
        }
        else if (Uart1_strcmp("music start"))
        {
            Start_Wav();
        }
        else if (Uart1_strcmp("music stop"))
        {
            xTaskNotify(Wav_Task_Handle, Wav_PlayBit_Stop, eSetBits);
        }
        else if (Uart1_strcmp("music resume"))
        {
            xTaskNotify(Wav_Task_Handle, Wav_PlayBit_Resume, eSetBits);
        }
        else if (Uart1_strcmp("music task query"))
        {
            if (Wav_Task_Handle != NULL)
            {
                eTaskState eReturn = eTaskGetState(Wav_Task_Handle);
                Uart1_SendData("Wav_Task_Handle Task is %d\r\n", eReturn);
            }
            else
            {
                Uart1_SendData("Handle is NULL!!\r\n");
            }
        }
        else if (Uart1_strcmp("help"))
        {
            BetaTest_PrintHelp();
        }
        else
        {
            Uart1_SendData("Unknown command. Type 'help' for available commands.\r\n");
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

            Start_Wav();
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

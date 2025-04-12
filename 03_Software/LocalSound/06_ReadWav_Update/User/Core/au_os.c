/*
 * @Author: YourName
 * @Date: 2025-01-22 10:30:01
 * @LastEditTime: 2025-02-14 16:51:55
 * @LastEditors: YourName
 * @Description:
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\04_MyPrj\02_BleAudio\03_Software\LocalSound\06_ReadWav_Update\User\Core\au_os.c
 * 版权声明
 */
#include "au_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "au_decode.h"
#include "au_config.h"

#include "gui.h"

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#if (Define_DEBUG == 1)
#define Wav_Debug_Print(...) Uart1_SendData(__VA_ARGS__)
#else
#define Wav_Debug_Print(...)
#endif

// #define WavBuff_Size 256
#define WavBuff_Size 1024
#define Wav_Start(out, len) HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)out, len, DAC_ALIGN_12B_R)
#define Wav_Stop() HAL_DAC_Stop_DMA(&hdac, DAC1_CHANNEL_1)
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
TaskHandle_t Wav_Task_Handle;
// int16_t WAV_CH[WavBuff_Size * 2] = {0};
// int16_t WAV_TmpBuf[WavBuff_Size] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
int Wav_OpenFile(FIL *file, char *path, Audio_WAV_Info *wav);
int Wav_Player_Init(FIL *file, char *path, Audio_WAV_Info *wav, Wav_CH_Data *wav_ch);
int Wav_Player(FIL *file, char *path, Audio_WAV_Info *wav, Wav_CH_Data *wav_ch);
/* USER CODE END PFP */

/* extern variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
/* USER CODE END EV */

/* extern function prototypes -----------------------------------------------*/
/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

void Start_Wav(void)
{
    if (Wav_Task_Handle != NULL)
    {
        //获取任务状态
        eTaskState eReturn = eTaskGetState(Wav_Task_Handle);
        Uart1_SendData("Wav_Task_Handle Task is %d\r\n", eReturn);

        if (eReturn != eDeleted)
        {
            //任务正常运行，则忽略本次按键命令
            Uart1_SendData("Wav_Task_Handle is running\r\n");
            return;
        }
    }

        //获取选定的音乐文件名称
    char *misName = (char *)list_get_element(&MicList, MicList_Idx_Gui);

    Uart1_SendData("create Wav_Task_Handle\r\n");
    //创建音乐播放任务
    xTaskCreate((TaskFunction_t)Wav_Task,
                (const char *)"Wav_Task",
                (configSTACK_DEPTH_TYPE)2048,
                (void *)misName,
                (UBaseType_t)5,
                &Wav_Task_Handle);
}

void Wav_Task(void const *argument)
{
    FIL file;               // 文件对象
    Audio_WAV_Info WavData; // wav信息结构体
    int wav_len;

    // 等待任务开始，后面会改成创建一个新任务 等待GUI任务发送信号开始播放音乐
    // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    char filePath[60];
    sprintf(filePath, "%s%s", USERPath, (char *)argument); // 拼接出带逻辑驱动器名的完整路径名
    Wav_Debug_Print("%s\r\n", filePath);

    // 打开文件，读取wav信息
    wav_len = Wav_OpenFile(&file, filePath, &WavData);
    if (wav_len < 0)
    {
        // f_close(&file);
        vTaskDelete(NULL);
    }

    Wav_CH_Data wav_ch = {0};
    // int16_t Wav_DacOutout_Buf[WavBuff_Size * 2] = {0};

    // wav_ch.Ch_r = WAV_CH;
    Wav_Debug_Print("wav_ch.Ch_r:%d\r\n", sizeof(int16_t) * WavBuff_Size * 2);
    wav_ch.Ch_r = malloc(sizeof(int16_t) * WavBuff_Size * 2);
    wav_ch.Ch_l = NULL; // 暂时只用单声道播放
    wav_ch.Len = WavBuff_Size * 2;

    /*------------------- 初始化DAC输出信号，并开始DMA传输 ------------------------------------*/
    if (Wav_Player_Init(&file, filePath, &WavData, &wav_ch) < 0)
    {
        // f_close(&file);
        vTaskDelete(NULL);
    }

    MicList_Idx_au = MicList_Idx_Gui;
    Wav_Debug_Print("dac start\r\n");
    Wav_Start(wav_ch.Ch_r, WavBuff_Size * 2);
    // HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)wav_ch.Ch_r, WavBuff_Size * 2, DAC_ALIGN_12B_R);

    Wav_Debug_Print("循环开始\r\n");
    // vTaskDelay(portMAX_DELAY);

    // if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 1)
    //     Wav_Debug_Print("[ERROR] Core obliterated Data!!");

    Wav_Player(&file, filePath, &WavData, &wav_ch);

    /* 不再读写，关闭文件 */
    Wav_Debug_Print("循环结束\r\n");
    Wav_Debug_Print("关闭文件\r\n");

    free(wav_ch.Ch_r);
    // free(wav_ch.Ch_l);
    f_close(&file);
    Wav_Stop();
    MicList_Idx_au = 0;

    vTaskDelete(NULL);
}

int Wav_Player(FIL *file, char *path, Audio_WAV_Info *wav, Wav_CH_Data *wav_ch)
{
    FRESULT f_res;
    UINT fnum;
    int wReturn = 1;
    char WriteCnt = 0; // 计数器，用于实现半写入、全写入区分
    u64 oldPtr = 0;
    u32 xReturn = 0;

    int wavTmplen = 0;
    if (wav->fmt_ck.nChannels == 1)
        wavTmplen = sizeof(int16_t) * (wav_ch->Len / 2);
    else
        wavTmplen = sizeof(int16_t) * (wav_ch->Len);

    Wav_Debug_Print("player\r\n");
    Wav_Debug_Print("tmpBuf len:%d\r\n", wavTmplen);
    int16_t *tmpBuf = (int16_t *)malloc(wavTmplen);
    // int16_t *tmpBuf = WAV_TmpBuf;

    while (1)
    {
        xTaskNotifyWait(0, 0xFFFF, &xReturn, portMAX_DELAY);
        while (xReturn != Wav_PlayBit_DacNotify)
        {
            // Wav_Stop();
            HAL_TIM_Base_Stop(&htim4);
            Wav_Debug_Print("暂停播放\r\n");

            xTaskNotifyWait(0, 0xFFFF, &xReturn, portMAX_DELAY);
            if (xReturn == Wav_PlayBit_Resume)
            {
                // Wav_Start(wav_ch->Ch_r, WavBuff_Size * 2);
                HAL_TIM_Base_Start(&htim4);
                Wav_Debug_Print("继续播放\r\n");
                break;
            }
        }
        // if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 1)
        //     Wav_Debug_Print("[ERROR] Core obliterated Data!!");

        oldPtr = f_tell(file);

        f_res = f_read(file, tmpBuf, wavTmplen, &fnum);
        if (f_res == FR_OK)
        {
            // Wav_Debug_Print("》fnum：%d\r\n", fnum);
            if (0 == fnum)
            {
                Wav_Debug_Print("文件读取完毕\r\n");
                Wav_Stop();
                wReturn = -1;
                break;
            }
            // Wav_Process_SingTrack((wav_ch->Ch_r + (WriteCnt % 2) * WavBuff_Size), tmpBuf, fnum / 2);

            if (wav->fmt_ck.nChannels == 1)
                Wav_Process_SingTrack((wav_ch->Ch_r + (WriteCnt % 2) * (wav_ch->Len / 2)), tmpBuf, wav_ch->Len / 2);
            else
            {
                if (wav_ch->Ch_l != NULL)
                    Wav_Process_DualTrack((wav_ch->Ch_r + (WriteCnt % 2) * wav_ch->Len), (wav_ch->Ch_l + (WriteCnt % 2) * wav_ch->Len), tmpBuf, wav_ch->Len / 2);
                else // 可能可以删除，后续需要测试是否可以删除
                    Wav_Process_DualTrack((wav_ch->Ch_r + (WriteCnt % 2) * wav_ch->Len), NULL, tmpBuf, wav_ch->Len / 2);
            }

            // printf_WavInfo(wav_ch->Ch_r, WavBuff_Size * 2);
            WriteCnt++;
        }
        else
        {
            Wav_Debug_Print("！！文件读取失败：(%d)\r\n", f_res);
            // HAL_DAC_Stop_DMA(&hdac, DAC1_CHANNEL_1);

            f_close(file);
            f_res = f_open(file, path, FA_OPEN_EXISTING | FA_READ);
            if (f_res != FR_OK)
            {
                Wav_Debug_Print("重新打开失败\r\n");
                wReturn = -2;
                break;
            }

            Wav_Debug_Print("old pointer:%d\r\n", oldPtr);
            f_res = f_lseek(file, oldPtr);
            if (f_res != FR_OK)
            {
                Wav_Debug_Print("指针移动失败,f_res:(%d)\r\n", f_res);
                wReturn = -3;
                break;
            }
            Wav_Debug_Print("重新传输开始\r\n");
            // HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)wav_ch.Ch_r, WavBuff_Size * 2, DAC_ALIGN_12B_R);

            // ptr = f_tell(&file);
            // Wav_Debug_Print("now pointer:%d\r\n", ptr);
            // Wav_Debug_Print("pointer diff:%d\r\n", (ptr - oldPtr));
            continue;
        }
    }

    free(tmpBuf);
    return wReturn;
}

/**
 * @brief 打开并读取WAV音频文件
 *
 * 该函数用于打开指定路径的WAV音频文件，并读取文件内容到指定的结构体中。
 *
 * @param file 文件指针，指向要打开的文件
 * @param path 文件路径
 * @param WavData 用于存储WAV音频信息的结构体指针
 *
 * @return 成功返回WAV音频文件的长度，失败返回错误码
 *         -1: 文件类型不支持
 *         -2: 文件读取失败
 *         -3: 读取到的字节数为0
 */
int Wav_OpenFile(FIL *file, char *path, Audio_WAV_Info *wav)
{
    FRESULT f_res;
    UINT fnum;
    int wav_len;
    /*------------------- 验证文件后缀名 ------------------------------------*/
    if (my_strnstr_kmp(path, ".wav", strlen(path)) == NULL)
    {
        Wav_Debug_Print("不支持的文件类型", f_res);
        return -1;
    }

    char wavTmp[WavBuff_Size] = {0}; /* ???? */
    /*------------------- 打开文件 ------------------------------------*/
    /*------------------- 文件读取 ------------------------------------*/
    f_res = f_open(file, path, FA_OPEN_EXISTING | FA_READ);
    if (f_res != FR_OK)
    {
        Wav_Debug_Print("f_res:%d\r\n", f_res);
        Wav_Debug_Print("！！打开文件失败。\r\n");
        return -1;
    }

    Wav_Debug_Print("》打开文件成功。\r\n");
    f_res = f_read(file, wavTmp, sizeof(wavTmp), &fnum);
    if (f_res != FR_OK)
    {
        Wav_Debug_Print("！！文件读取失败：(%d)\r\n", f_res);
        return -2;
    }

    Wav_Debug_Print("》文件读取成功,读到字节数据：%d\r\n", fnum);
    if (0 == fnum)
        return -3;
    else
    {
        wav_len = WAV_Format_parsing(wav, wavTmp);
        Wav_Debug_Print("wav_len:%d\n", wav->wavLen);
        return wav_len;
    }
}

/**
 * @brief 初始化并处理WAV音频输出
 *
 * 该函数用于初始化WAV音频输出，并处理音频数据。
 *
 * @param file 文件指针，指向需要读取的WAV文件
 * @param path 文件路径，指向WAV文件的路径
 * @param WavData 指向Audio_WAV_Info结构体的指针，存储WAV文件的元数据信息
 * @param out 指向输出音频数据的缓冲区
 * @param len 输出音频数据的长度
 *
 * @return 返回值表示函数执行结果：
 *         -1：输入参数为空
 *         -2：文件指针移动失败
 *         -3：文件读取失败
 *          1：成功
 */

volatile int16_t * pointer=NULL;
int Wav_Player_Init(FIL *file, char *path, Audio_WAV_Info *wav, Wav_CH_Data *wav_ch)
{
    FRESULT f_res;
    UINT fnum;
    Wav_Debug_Print("player_init\r\n");

    if ((file == NULL) || (path == NULL) || (wav == NULL) || (wav_ch == NULL))
        return -1;

    f_res = f_rewind(file);
    if (f_res != FR_OK)
    {
        Wav_Debug_Print("指针移动失败，f_res:(%d)\r\n", f_res);
        f_close(file);
        return -2;
    }

    int wavTmplen = 0;
    if (wav->fmt_ck.nChannels == 1)
        wavTmplen = sizeof(int16_t) * (wav_ch->Len / 2);
    else
        wavTmplen = sizeof(int16_t) * (wav_ch->Len);

    Wav_Debug_Print("tmpBuf len:%d\r\n", wavTmplen);
    int16_t *tmpBuf = (int16_t *)malloc(wavTmplen);
		pointer = tmpBuf;
    // int16_t *tmpBuf = WAV_TmpBuf;

    if (tmpBuf == NULL)
        return -1;

    for (int i = 0; i < 2; i++)
    {
        f_res = f_read(file, tmpBuf, wavTmplen, &fnum);
        if (f_res != FR_OK)
        {
            Wav_Debug_Print("！！文件读取失败：(%d)\r\n", f_res);
            f_close(file);
            free(tmpBuf);
            return -3;
        }
        // printf_WavInfo(tmpBuf, WavBuff_Size);

        Wav_Debug_Print("》文件读取成功,读到字节数据：%d\r\n", fnum);

        if (wav->fmt_ck.nChannels == 1)
            Wav_Process_SingTrack((wav_ch->Ch_r + i * (wav_ch->Len / 2)), tmpBuf, wav_ch->Len / 2);
        else
        {
            if (wav_ch->Ch_l != NULL)
                Wav_Process_DualTrack((wav_ch->Ch_r + i * wav_ch->Len), (wav_ch->Ch_l + i * wav_ch->Len), tmpBuf, wav_ch->Len / 2);
            else // 可能可以删除，后续需要测试是否可以删除
                Wav_Process_DualTrack((wav_ch->Ch_r + i * wav_ch->Len), NULL, tmpBuf, wav_ch->Len / 2);
        }

        // printf_WavInfo((wav_ch->Ch_r + i * (wav_ch->Len / 2)), WavBuff_Size);

        if (wav->fmt_ck.nChannels == 1)
        {
            if (i == 0) // 清零wav的文件信息，防止输出奇怪的声音
                memset(wav_ch->Ch_r, 0, wav->wavLen * 2);
        }
        else
        {
            if (i == 0) // 清零wav的文件信息，防止输出奇怪的声音
                memset(wav_ch->Ch_r, 0, wav->wavLen);
        }
        // Wav_Debug_Print("\r\n");
        // Wav_Debug_Print("\r\n");
        // Wav_Debug_Print("\r\n");
        // Wav_Debug_Print("\r\n");
    }
    Wav_Debug_Print("wav memset\r\n");

    // printf_WavInfo(wav_ch->Ch_r, WavBuff_Size * 2);
    free(tmpBuf);
    return 1;
}

void printf_WavInfo(short *data, int len)
{
    int j = 0;
    for (int i = 0; i < len; i++)
    {
        Wav_Debug_Print("%04d\t", data[i]);
        j++;
        if (j == 12)
        {
            j = 0;
            Wav_Debug_Print("\r\n");
        }
    }
}

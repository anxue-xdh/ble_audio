/*
 * @Author: YourName
 * @Date: 2025-01-22 10:30:01
 * @LastEditTime: 2025-02-07 16:07:24
 * @LastEditors: YourName
 * @Description:
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\04_MyPrj\02_BleAudio\03_Software\LocalSound\06_ReadWav_Update\User\Core\core.c
 * 版权声明
 */
#include "core.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "audio.h"
#include "audioConfig.h"

#include "gui.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#if (Define_DEBUG == 1)
#define Wav_Debug_Print(...) Uart1_SendData(__VA_ARGS__)
#else
#define Wav_Debug_Print(...)
#endif

#define WavBuff_Size 256
#define Wav_Start(out, len) HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)out, len, DAC_ALIGN_12B_R)
#define Wav_Stop() HAL_DAC_Stop_DMA(&hdac, DAC1_CHANNEL_1)
/* USER CODE END PD */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
int Wav_OpenFile(FIL *file, char *path, Audio_WAV_Info *WavData);
int Wav_Output_Init_Sign(FIL *file, char *path, Audio_WAV_Info *WavData, int16_t *out, uint16_t len);
/* USER CODE END PFP */

/* extern variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
extern TaskHandle_t GUI_Task_Handle;
/* USER CODE END EV */

/* extern function prototypes -----------------------------------------------*/
/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

void ReadWav(void const *argument)
{
    FRESULT f_res; /* ?????? */
    FIL file;      /* ???? */
    UINT fnum;     /* ???????? */
    Audio_WAV_Info WavData;
    int wav_len;

    // 等待任务开始，后面会改成创建一个新任务 等待GUI任务发送信号开始播放音乐
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    char filePath[60];
    sprintf(filePath, "%s%s", USERPath, MusicList[MusicList_Pointer]); // 拼接出带逻辑驱动器名的完整路径名
    Wav_Debug_Print("%s\r\n", filePath);

    // 打开文件，读取wav信息
    wav_len = Wav_OpenFile(&file, filePath, &WavData);
    if (wav_len < 0)
        vTaskDelete(NULL);

    int16_t wavBuff[WavBuff_Size] = {0};
    int16_t Wav_DacOutout_Buf[WavBuff_Size * 2] = {0};
    char WriteCnt = 0; // 计数器，用于实现半写入、全写入区分

    // 初始化DAC输出信号，并开始DMA传输
    Wav_Output_Init_Sign(&file, filePath, &WavData, Wav_DacOutout_Buf, WavBuff_Size * 2);

    Wav_Debug_Print("dac start\r\n");
    Wav_Start(Wav_DacOutout_Buf, WavBuff_Size * 2);
    // HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)Wav_DacOutout_Buf, WavBuff_Size * 2, DAC_ALIGN_12B_R);

    Wav_Debug_Print("循环开始\r\n");
    // vTaskDelay(portMAX_DELAY);
    u64 oldPtr = 0;
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 1)
        Wav_Debug_Print("[ERROR] Core obliterated Data!!");

    while (1)
    {

        oldPtr = f_tell(&file);

        f_res = f_read(&file, wavBuff, sizeof(wavBuff), &fnum);
        if (f_res == FR_OK)
        {
            // Wav_Debug_Print("》文件读取成功,读到字节数据：%d\r\n", fnum);
            if (0 == fnum)
            {
                Wav_Debug_Print("文件读取完毕\r\n");
                Wav_Stop();
                break;
            }
            Wav_Process_SingTrack((Wav_DacOutout_Buf + (WriteCnt % 2) * WavBuff_Size), wavBuff, fnum / 2);
            WriteCnt++;
        }
        else
        {
            Wav_Debug_Print("！！文件读取失败：(%d)\r\n", f_res);
            // HAL_DAC_Stop_DMA(&hdac, DAC1_CHANNEL_1);

            f_close(&file);
            // Wav_Debug_Print("关闭文件\r\n");

            f_res = f_open(&file, filePath, FA_OPEN_EXISTING | FA_READ);

            if (f_res != FR_OK)
            {
                Wav_Debug_Print("重新打开失败\r\n");
                break;
            }

            f_res = f_lseek(&file, oldPtr);
            if (f_res != FR_OK)
            {
                Wav_Debug_Print("指针移动失败,f_res:(%d)\r\n", f_res);
                break;
            }
            Wav_Debug_Print("重新传输开始\r\n");
            // HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)Wav_DacOutout_Buf, WavBuff_Size * 2, DAC_ALIGN_12B_R);

            // ptr = f_tell(&file);
            Wav_Debug_Print("old pointer:%d\r\n", oldPtr);
            // Wav_Debug_Print("now pointer:%d\r\n", ptr);
            // Wav_Debug_Print("pointer diff:%d\r\n", (ptr - oldPtr));

            // f_res = f_lseek(&file, oldPtr);
            // if (f_res != FR_OK)
            // {
            //     Wav_Debug_Print("指针移动失败,f_res:(%d)\r\n", f_res);
            //     vTaskDelay(HAL_MAX_DELAY);
            // }
            // ptr = f_tell(&file);
            // Wav_Debug_Print("change pointer:%d\r\n", ptr);
            continue;
        }

        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 1)
            Wav_Debug_Print("[ERROR] Core obliterated Data!!");

        // if (0 == (WriteCnt % 2))
        // {
        //     HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)Wav_DacOutout_Buf, WavBuff_Size * 2, DAC_ALIGN_12B_R);
        // }
    }
    /* 不再读写，关闭文件 */
    Wav_Debug_Print("关闭文件\r\n");

    f_close(&file);
    Wav_Stop();

    vTaskDelete(NULL);
}

/**
 * @brief 初始化WAV输出信号
 *
 * 该函数用于初始化WAV音频文件的输出信号。
 *
 * @param file 文件指针，指向已打开的WAV音频文件
 * @param path WAV音频文件的路径
 * @param WavData 指向包含WAV音频信息的结构体指针
 * @param out 输出缓冲区的指针，用于存储处理后的音频数据
 * @param len 输出缓冲区的长度
 *
 * @return 返回值表示函数执行状态：
 *         -1: 文件指针移动失败
 *         -2: 文件读取失败
 *          1: 操作成功
 */
int Wav_Output_Init_Sign(FIL *file, char *path, Audio_WAV_Info *WavData, int16_t *out, uint16_t len)
{
    FRESULT f_res;
    UINT fnum;

    f_res = f_rewind(file);
    if (f_res != FR_OK)
    {
        Wav_Debug_Print("指针移动失败，f_res:(%d)\r\n", f_res);
        f_close(file);
        return -1;
    }

    int wavTmplen = sizeof(int16_t) * (len / 2);
    Wav_Debug_Print("tmpBuf len:%d\r\n", wavTmplen);
    int16_t *tmpBuf = (int16_t *)malloc(wavTmplen);

    for (int i = 0; i < 2; i++)
    {
        f_res = f_read(file, tmpBuf, wavTmplen, &fnum);
        if (f_res != FR_OK)
        {
            Wav_Debug_Print("！！文件读取失败：(%d)\r\n", f_res);
            f_close(file);
            free(tmpBuf);
            return -2;
        }

        Wav_Debug_Print("》文件读取成功,读到字节数据：%d\r\n", fnum);
        Wav_Process_SingTrack((out + i * (len / 2)), tmpBuf, len / 2);

        // printf_WavInfo((out + i * (len / 2)), WavBuff_Size);

        // Wav_Debug_Print("\r\n");
        // Wav_Debug_Print("\r\n");
        // Wav_Debug_Print("\r\n");
        // Wav_Debug_Print("\r\n");
    }
    Wav_Debug_Print("wav memset\r\n");

    // 清零wav的文件信息，防止输出奇怪的声音
    memset(out, 0, WavData->wavLen * 2);

    // printf_WavInfo(out, WavBuff_Size * 2);
    free(tmpBuf);
    return 1;
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
int Wav_OpenFile(FIL *file, char *path, Audio_WAV_Info *WavData)
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

    // Wav_Debug_Print("\r\n****** 文件读取开始 ******\r\n");
    // char tempfilepath[60];
    // sprintf(tempfilepath, "%s%s", USERPath, path); // 拼接出带逻辑驱动器名的完整路径名
    // Wav_Debug_Print("%s\r\n", tempfilepath);

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
        wav_len = WAV_Format_parsing(WavData, wavTmp);
        Wav_Debug_Print("wav_len:%d\n", WavData->wavLen);
        return wav_len;
    }
}

void printf_WavInfo(short *data, int len)
{
    int j = 0;
    for (int i = 0; i < len; i++)
    {
        Wav_Debug_Print("%d ", data[i]);
        j++;
        if (j == 16)
        {
            j = 0;
            Wav_Debug_Print("\r\n");
        }
    }
}
FRESULT SD_Read_FileInfo(const char *path, char (*list)[64])
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int nfile, ndir;
    TCHAR name[64];

    fno.lfname = name;
    fno.lfsize = 64;

    res = f_opendir(&dir, path); /* Open the directory */
    if (res == FR_OK)
    {
        nfile = ndir = 0;
        for (;;)
        {
            res = f_readdir(&dir, &fno); /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0)
                break; /* Error or end of dir */
            if (fno.fattrib & AM_DIR)
            { /* Directory */
                Wav_Debug_Print("   <DIR>   %s\n", fno.lfname);
                ndir++;
            }
            else
            { /* File */
                Wav_Debug_Print("%10u %s\n", fno.fsize, fno.lfname);

                if (fno.lfname[0] == 0)
                    continue;
                memcpy(list[nfile], fno.lfname, strlen(fno.lfname));

                nfile++;
            }
        }
        f_closedir(&dir);
        Wav_Debug_Print("%d dirs, %d files.\n", ndir, nfile);

        // xTaskNotifyGive(GUI_Task_Handle);
        xTaskNotify(GUI_Task_Handle, GUI_TaskBit_MusicList_Update, eSetBits);
    }
    else
    {
        Wav_Debug_Print("Failed to open \"%s\". (%u)\n", path, res);
    }
    return res;
}

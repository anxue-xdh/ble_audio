#include "core.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "fatfs.h"

#include "audio.h"
#include "audioConfig.h"

#define WavBuff_Size 256

char wavInfo[256] = {0}; /* ???? */
// int16_t wavBuff[512] = {0}; /* ???? */
int16_t wavBuff[WavBuff_Size] = {0}; /* ???? */
int16_t Wav_DacOutout_Buf[WavBuff_Size * 2] = {0};
char WriteCnt = 0; // 计数器，用于实现半写入、全写入区分

void ReadWav(void const *argument)
{
    FRESULT f_res; /* ?????? */
    FIL file;      /* ???? */
    UINT fnum;     /* ???????? */
    Audio_WAV_Info WavData;
    int wav_len;

    Uart1_SendData("\r\n****** Wav文件读取 ******\r\n");
    // vTaskDelay(portMAX_DELAY);

    char tempfilepath[60];
    // Uart1_SendData("SDPath:%s\r\n", USERPath);

    // sprintf(tempfilepath, "%s%s", USERPath, "INeverForget_R.wav"); // 拼接出带逻辑驱动器名的完整路径名
    sprintf(tempfilepath, "%s%s", USERPath, "BaJiaoNightRain_R.wav"); // 拼接出带逻辑驱动器名的完整路径名
    // sprintf(tempfilepath, "%s%s", USERPath, "NiHao.wav"); // 拼接出带逻辑驱动器名的完整路径名
    // Uart1_SendData("%s\r\n", tempfilepath);

    /*------------------- 文件系统测试：读测试 ------------------------------------*/
    // Uart1_SendData("****** 即将进行文件读取测试... ******\r\n");
    f_res = f_open(&file, tempfilepath, FA_OPEN_EXISTING | FA_READ);
    if (f_res == FR_OK)
    {
        Uart1_SendData("》打开文件成功。\r\n");
        f_res = f_read(&file, wavInfo, sizeof(wavInfo), &fnum);
        if (f_res == FR_OK)
        {
            Uart1_SendData("》文件读取成功,读到字节数据：%d\r\n", fnum);
            if (0 == fnum)
            {
                vTaskDelete(NULL);
            }
            else
            {
                wav_len = WAV_Format_parsing(&WavData, wavInfo);
                Uart1_SendData("wav_len:%d\n", wav_len);
            }
        }
        else
        {
            Uart1_SendData("！！文件读取失败：(%d)\r\n", f_res);
        }
    }
    else
    {
        Uart1_SendData("f_res:%d\r\n", f_res);
        Uart1_SendData("！！打开文件失败。\r\n");
        vTaskDelete(NULL);
    }

    f_res = f_rewind(&file);
    // f_res = f_lseek(&file, wav_len);
    if (f_res != FR_OK)
    {
        Uart1_SendData("指针移动失败，f_res:(%d)\r\n", f_res);
        vTaskDelay(HAL_MAX_DELAY);
    }

    Uart1_SendData("指针移动成功\r\n");

    for (int i = 0; i < 2; i++)
    {
        f_res = f_read(&file, wavBuff, sizeof(wavBuff), &fnum);
        if (f_res == FR_OK)
        {
            Uart1_SendData("》文件读取成功,读到字节数据：%d\r\n", fnum);
            // Data_16to12_Mult((Wav_DacOutout_Buf), wavBuff, fnum / 2);
            Data_16to12_Mult((Wav_DacOutout_Buf + i * WavBuff_Size), wavBuff, fnum / 2);

            // printf_WavInfo(Wav_DacOutout_Buf, WavBuff_Size);

            // Uart1_SendData("\r\n");
            // Uart1_SendData("\r\n");
            // Uart1_SendData("\r\n");
            // Uart1_SendData("\r\n");
        }
        else
        {
            Uart1_SendData("！！文件读取失败：(%d)\r\n", f_res);
            vTaskDelay(HAL_MAX_DELAY);
        }
    }

    memset(Wav_DacOutout_Buf, 0, wav_len * 2);
    // printf_WavInfo(Wav_DacOutout_Buf, WavBuff_Size * 2);

    HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)Wav_DacOutout_Buf, WavBuff_Size * 2, DAC_ALIGN_12B_R);

    Uart1_SendData("循环开始\r\n");
    // vTaskDelay(portMAX_DELAY);
    u64 oldPtr = 0;
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 1)
        Uart1_SendData("[ERROR] Core obliterated Data!!");

    while (1)
    {

        oldPtr = f_tell(&file);

        f_res = f_read(&file, wavBuff, sizeof(wavBuff), &fnum);
        if (f_res == FR_OK)
        {
            // Uart1_SendData("》文件读取成功,读到字节数据：%d\r\n", fnum);
            if (0 == fnum)
            {
                Uart1_SendData("文件读取完毕\r\n");
                break;
            }

            Data_16to12_Mult((Wav_DacOutout_Buf + (WriteCnt % 2) * WavBuff_Size), wavBuff, fnum / 2);
            WriteCnt++;
        }
        else
        {
            Uart1_SendData("！！文件读取失败：(%d)\r\n", f_res);

            f_close(&file);
            // Uart1_SendData("关闭文件\r\n");

            f_res = f_open(&file, tempfilepath, FA_OPEN_EXISTING | FA_READ);

            if (f_res != FR_OK)
            {
                Uart1_SendData("重新打开失败\r\n");
                break;
            }

            f_res = f_lseek(&file, oldPtr);
            if (f_res != FR_OK)
            {
                Uart1_SendData("指针移动失败,f_res:(%d)\r\n", f_res);
                break;
            }
            Uart1_SendData("重新传输开始\r\n");

            // ptr = f_tell(&file);
            Uart1_SendData("old pointer:%d\r\n", oldPtr);
            // Uart1_SendData("now pointer:%d\r\n", ptr);
            // Uart1_SendData("pointer diff:%d\r\n", (ptr - oldPtr));

            // f_res = f_lseek(&file, oldPtr);
            // if (f_res != FR_OK)
            // {
            //     Uart1_SendData("指针移动失败,f_res:(%d)\r\n", f_res);
            //     vTaskDelay(HAL_MAX_DELAY);
            // }
            // ptr = f_tell(&file);
            // Uart1_SendData("change pointer:%d\r\n", ptr);
            continue;
        }

        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 1)
            Uart1_SendData("[ERROR] Core obliterated Data!!");

        // if (0 == (WriteCnt % 2))
        // {
        //     HAL_DAC_Start_DMA(&hdac, DAC1_CHANNEL_1, (uint32_t *)Wav_DacOutout_Buf, WavBuff_Size * 2, DAC_ALIGN_12B_R);
        // }
    }
    /* 不再读写，关闭文件 */
    f_close(&file);
    Uart1_SendData("关闭文件\r\n");
    HAL_DAC_Stop_DMA(&hdac, DAC1_CHANNEL_1);
    vTaskDelete(NULL);
}

void printf_WavInfo(short *data, int len)
{
    int j = 0;
    for (int i = 0; i < len; i++)
    {
        Uart1_SendData("%d ", data[i]);
        j++;
        if (j == 16)
        {
            j = 0;
            Uart1_SendData("\r\n");
        }
    }
}
#define Def_Data_16to12_single(data) ((short)(data + (1 << 15)) >> 4)

short Data_16to12_single(short data)
{
    unsigned short tmp = 0;
    tmp = ((unsigned short)(data + (1 << 15))) >> 4;
    //    tmp = ((unsigned short)(data + 16384)) >> 4;
    return tmp;
}

int Data_16to12_Mult(short *ret, short *data, int len)
{
    if ((ret == NULL) || (data == NULL))
        return -1;

    for (int i = 0; i < len; i++)
    {
        // ret[i] = Data_16to12_single(data[i]);
        ret[i] = Def_Data_16to12_single(data[i]);
    }
    return len;
}

int Data_16to12_Mult_one(short *data, int len)
{
    if (data == NULL)
        return -1;

    for (int i = 0; i < len; i++)
    {
        data[i] = Data_16to12_single(data[i]);
    }
    return len;
}

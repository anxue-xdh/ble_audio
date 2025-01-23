/*
 * @Date: 2023-06-12 09:56:56
 * @LastEditors: YourName
 * @LastEditTime: 2025-01-22 16:53:37
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\04_MyPrj\02_BleAudio\03_Software\LocalSound\06_ReadWav_Update\User\Core\audio.c
 */
#include "audio.h"

static int audio_wave_info_verify(const Audio_WAV_Info *wav_info);

/**
 * @description: 将wave格式的音频文件中的格式参数提取到Audio_WAV_Info中，同时返回音频data的指针头
 * @param {Audio_WAV_Info} *wav_info	wave文件参数		[out]
 * @param {char} *audio_wav_ori			wave文件原始数据	[in]
 * @return {*} 错误代码 见audio.h
 * @use:
 */
int WAV_Format_parsing(Audio_WAV_Info *wav_info, char *audio_wav_ori)
{
    char *temp_data = audio_wav_ori;

    // check for RIFF
    memcpy(wav_info->ckID, temp_data, 4);
    temp_data += 4;
    if (memcmp(wav_info->ckID, WAVE_CKID, 4))
        return ERR_ID_RIFF;

    // Get Size
    memcpy(&wav_info->cksize, temp_data, 4);
    temp_data += 4;

    Uart1_SendData("wave_size:%d\r\n", wav_info->cksize);

    // .wav file flag
    memcpy(wav_info->WAVEID, temp_data, 4);
    temp_data += 4;
    if (memcmp(wav_info->WAVEID, WAVE_ID, 4))
        return ERR_ID_WAVE;

    Uart1_SendData("wave_pass\r\n");
    // fmt
    memcpy(wav_info->fmt_ck.ckID, temp_data, 4);
    temp_data += 4;
    // Uart1_SendData("%s\r\n", wav_info->fmt_ck.ckID);
    if (memcmp(wav_info->fmt_ck.ckID, "fmt ", 4))
    {
        Uart1_SendData("fmt_error\r\n");
        return ERR_ID_FMT;
    }

    // fmt length
    memcpy(&wav_info->fmt_ck.cksize, temp_data, 4);
    temp_data += 4;

    Uart1_SendData("fmt_size:%d\r\n", wav_info->fmt_ck.cksize);

    // wFormatTag : PCM or other
    memcpy(&wav_info->fmt_ck.wFormatTag, temp_data, 2);
    temp_data += 2;
    if (wav_info->fmt_ck.wFormatTag != WAVE_FORMAT_PCM &&
        wav_info->fmt_ck.wFormatTag != WAVE_FORMAT_IEEE_FLOAT &&
        wav_info->fmt_ck.wFormatTag != WAVE_FORMAT_ALAW &&
        wav_info->fmt_ck.wFormatTag != WAVE_FORMAT_MULAW &&
        wav_info->fmt_ck.wFormatTag != WAVE_FORMAT_EXTENSIBLE)
        return ERR_FMT_PCM;
    // nChannels
    memcpy(&wav_info->fmt_ck.nChannels, temp_data, 2);
    temp_data += 2;
    Uart1_SendData("nChannels:%d\r\n", wav_info->fmt_ck.nChannels);

    // Sample Rate in Hz
    memcpy(&wav_info->fmt_ck.nSamplesPerSec, temp_data, 4);
    temp_data += 4;
    memcpy(&wav_info->fmt_ck.nAvgBytesPerSec, temp_data, 4);
    temp_data += 4;
    Uart1_SendData("nSamplesPerSec:%d\r\n", wav_info->fmt_ck.nSamplesPerSec);
    Uart1_SendData("nAvgBytesPerSec:%d\r\n", wav_info->fmt_ck.nAvgBytesPerSec);
    Uart1_SendData("nSamplesPerSec:%x\r\n", wav_info->fmt_ck.nSamplesPerSec);
    Uart1_SendData("nAvgBytesPerSec:%x\r\n", wav_info->fmt_ck.nAvgBytesPerSec);

    // quantize bytes for per samp point
    memcpy(&wav_info->fmt_ck.nBlockAlign, temp_data, 2);
    temp_data += 2;
    memcpy(&wav_info->fmt_ck.wBitsPerSample, temp_data, 2);
    temp_data += 2;
    Uart1_SendData("fmt_pass\r\n");

#if AUDIO_FMT_EXTENSION_ON_OFF
    memcpy(&wav_info->fmt_ck.cbSize, temp_data, 2);
    if (wav_info->fmt_ck.cbSize > 0)
    {
        temp_data += 2;
        // fmt extension info function
    }
#endif

#if AUDIO_FACT_ON_OFF
    // fact
    memcpy(wav_info->fact_ck.ckID, temp_data, 4);
    if (memcmp(wav_info->fact_ck.ckID, FACT_CKID, 4))
    {
        temp_data += 4;
        // fact chunk info function
    }
#endif

    char *location = my_strnstr_kmp(temp_data, "data", 200);
    temp_data = location;
    // data chunk id
    memcpy(wav_info->data_ck.ckID, temp_data, 4);
    temp_data += 4;
    if (memcmp(wav_info->data_ck.ckID, DATA_CKID, 4))
        return ERR_ID_DATA;
    // data length
    memcpy(&wav_info->data_ck.cksize, temp_data, 4);
    temp_data += 4;

    Uart1_SendData("data size:%d\r\n", wav_info->data_ck.cksize);
    Uart1_SendData("data size:%x\r\n", wav_info->data_ck.cksize);

    wav_info->data_ck.sampleData = temp_data;

    // memcpy(wav_info->data_ck.pad_byte, temp_data, 1);

    // 校验过程函数
    // 输入的参数是否符合要求
    // 主要校验size的长度是否一致
    int err_code = audio_wave_info_verify(wav_info);
    if (err_code != ERR_NO)
        return err_code;

    return temp_data - audio_wav_ori;
}
/**
 * @description: 检验wave文件内容是否正确，内部函数
 * @param {Audio_WAV_Info}*wav_info	wave文件参数		[out]
 * @return {*}错误代码 见audio.h
 * @use:
 */
static int audio_wave_info_verify(const Audio_WAV_Info *wav_info)
{
    if (wav_info->fmt_ck.nBlockAlign != WAVE_BLCOK_ALIGN_VERIFY)
        return ERR_BLOCK_ALIGN;

    // byteRate verify
    if (wav_info->fmt_ck.nAvgBytesPerSec != WAVE_AVG_BYTE_PERSEC_VERIFY)
        return ERR_AVG_BYTE;

    return ERR_NO;
}

// 简化的KMP算法实现（未完整实现KMP的所有优化）
char *my_strnstr_kmp(const char *s1, const char *s2, size_t n)
{
    if (s1 == NULL || s2 == NULL)
        return NULL;
    if (n == 0)
        return NULL;
    if (*s2 == '\0')
        return (char *)s1; // 如果s2为空字符串，则s1的任何位置都是匹配点

    const char *p1 = s1;
    const char *p2 = s2;
    int s2len = strlen(s2);

    while (n--)
    {
        // 字符匹配，继续检查下一个字符
        if (*p2 == '\0')
        {
            // s2已结束，找到了匹配
            return (char *)(p1 - s2len);
        }

        if (*p1 == *p2)
        {
            p1++;
            p2++;
        }
        else
        {
            // 当前字符不匹配，重置p2到s2的开头，p1根据s2的某些性质移动（这里简化处理）
            p2 = s2;
            p1++;
        }
    }
    // 搜索完n个字符后仍未找到匹配
    return NULL;
}

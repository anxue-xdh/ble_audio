/*
 * @Date: 2023-06-12 09:56:56
 * @LastEditors: YourName
 * @LastEditTime: 2025-02-14 13:53:41
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\04_MyPrj\02_BleAudio\03_Software\LocalSound\06_ReadWav_Update\User\Core\au_decode.c
 */
#include "au_decode.h"

#if (Define_DEBUG == 1)
#define Audio_Debug(...) Uart1_SendData(__VA_ARGS__)
#else
#define Audio_Debug(...)
#endif

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

    Audio_Debug("wave_size:%d\r\n", wav_info->cksize);

    // .wav file flag
    memcpy(wav_info->WAVEID, temp_data, 4);
    temp_data += 4;
    if (memcmp(wav_info->WAVEID, WAVE_ID, 4))
        return ERR_ID_WAVE;

    Audio_Debug("wave_pass\r\n");
    // fmt
    memcpy(wav_info->fmt_ck.ckID, temp_data, 4);
    temp_data += 4;
    // Audio_Debug("%s\r\n", wav_info->fmt_ck.ckID);
    if (memcmp(wav_info->fmt_ck.ckID, "fmt ", 4))
    {
        Audio_Debug("fmt_error\r\n");
        return ERR_ID_FMT;
    }

    // fmt length
    memcpy(&wav_info->fmt_ck.cksize, temp_data, 4);
    temp_data += 4;

    Audio_Debug("fmt_size:%d\r\n", wav_info->fmt_ck.cksize);

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
    Audio_Debug("nChannels:%d\r\n", wav_info->fmt_ck.nChannels);

    // Sample Rate in Hz
    memcpy(&wav_info->fmt_ck.nSamplesPerSec, temp_data, 4);
    temp_data += 4;
    memcpy(&wav_info->fmt_ck.nAvgBytesPerSec, temp_data, 4);
    temp_data += 4;
    Audio_Debug("nSamplesPerSec:%d\r\n", wav_info->fmt_ck.nSamplesPerSec);
    Audio_Debug("nAvgBytesPerSec:%d\r\n", wav_info->fmt_ck.nAvgBytesPerSec);
    // Audio_Debug("nSamplesPerSec:%x\r\n", wav_info->fmt_ck.nSamplesPerSec);
    // Audio_Debug("nAvgBytesPerSec:%x\r\n", wav_info->fmt_ck.nAvgBytesPerSec);

    // quantize bytes for per samp point
    memcpy(&wav_info->fmt_ck.nBlockAlign, temp_data, 2);
    temp_data += 2;
    memcpy(&wav_info->fmt_ck.wBitsPerSample, temp_data, 2);
    temp_data += 2;
    Audio_Debug("fmt_pass\r\n");

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

    Audio_Debug("data size:%d\r\n", wav_info->data_ck.cksize);
    // Audio_Debug("data size:%x\r\n", wav_info->data_ck.cksize);

    wav_info->data_ck.sampleData = temp_data;

    // memcpy(wav_info->data_ck.pad_byte, temp_data, 1);

    // 校验过程函数
    // 输入的参数是否符合要求
    // 主要校验size的长度是否一致
    int err_code = audio_wave_info_verify(wav_info);
    if (err_code != ERR_NO)
        return err_code;

    wav_info->wavLen = temp_data - audio_wav_ori;
    return wav_info->wavLen;
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
/**
 * @brief 使用KMP算法在字符串s1中查找字符串s2的前n个字符，并返回匹配起始位置的指针
 *
 * 在字符串s1中查找字符串s2的前n个字符，如果找到则返回匹配起始位置的指针，否则返回NULL。
 *
 * @param s1 待搜索的字符串
 * @param s2 要查找的子字符串
 * @param n 要查找的子字符串长度
 *
 * @return 如果找到匹配的子字符串，则返回匹配起始位置的指针；否则返回NULL
 *
 * @note 如果s2为空字符串，则任何位置都是匹配点，函数将返回s1的起始地址。
 */
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
        if (*p1 == *p2)
        {
            p1++;
            p2++;
            if (*p2 == '\0') // s2已结束，找到了匹配
                return (char *)(p1 - s2len);
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

/**
 * @brief 处理双声道音频数据
 *
 * 该函数将输入的16位音频数据转换为12位音频数据，并输出到两个声道（右声道和左声道）。
 *
 * @param out_r 右声道输出数据的指针
 * @param out_l 左声道输出数据的指针（可以为NULL，表示只处理右声道）
 * @param in 输入的16位音频数据指针
 * @param len 输入数据的长度（以样本为单位）
 *
 * @return 无返回值，如果输入参数无效，则返回-1
 */
int Wav_Process_DualTrack(int16_t *out_r, int16_t *out_l, int16_t *in, u32 len)
{
    if ((out_r == NULL) || (in == NULL))
        return -1;

    if (out_l == NULL)
    {
        for (int i = 0; i < len; i++)
        {
            out_r[i] = Def_Data_16to12_single(in[i * 2]);
        }
    }
    else
    {
        for (int i = 0; i < len; i++)
        {
            out_r[i] = Def_Data_16to12_single(in[i * 2]);
            out_l[i] = Def_Data_16to12_single(in[i * 2 + 1]);
        }
    }
    return len;
}

/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-12 09:57:11
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-14 14:14:06
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\VsCodeStm32\AudioPlayer\UserCore\Inc\audio.h
 * @Description: ËøôÊòØÈªòË?§Ë?æÁΩÆ,ËØ∑Ë?æÁΩÆ`customMade`, ÊâìÂºÄkoroFileHeaderÊü•ÁúãÈÖçÁΩÆ ËøõË?åË?æÁΩÆ: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __AUDIO_DECODE_H__
#define __AUDIO_DECODE_H__

#include "basic.h"
#include "au_config.h"
#include "string.h"

#define WAVE_CKID "RIFF"
#define WAVE_ID "WAVE"
#define FMT_CKID "fmt "
#define FACT_CKID "fact"
#define DATA_CKID "data"
#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#define WAVE_FORMAT_ALAW 0x0006
#define WAVE_FORMAT_MULAW 0x0007
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE
#define WAVE_BLCOK_ALIGN_VERIFY (wav_info->fmt_ck.nChannels * wav_info->fmt_ck.wBitsPerSample / 8)                                       // ‰∏çÂèØË¢?Ë∞ÉÁî®
#define WAVE_AVG_BYTE_PERSEC_VERIFY (wav_info->fmt_ck.nSamplesPerSec * wav_info->fmt_ck.nChannels * wav_info->fmt_ck.wBitsPerSample / 8) // ‰∏çÂèØË¢?Ë∞ÉÁî®
#define WAVE_FMT_CHNANELS_MONO 1
#define WAVE_FMT_CHNANELS_SETREO 2
#define WAVE_FMT_SAMPLES_BIT_LOW 0x0008
#define WAVE_FMT_SAMPLES_BIT_HIGH 0x0010
#define WAVE_FMT_SAMPLES_BIT_EXTRE_HIGH 0x0020

// error code
#define ERR_NO 1
#define ERR_ID_RIFF -1
#define ERR_ID_WAVE -2
#define ERR_ID_FMT -3
#define ERR_FMT_PCM -4
#define ERR_ID_DATA -5
#define ERR_BLOCK_ALIGN -6
#define ERR_AVG_BYTE -7

typedef struct fmt
{
    char ckID[4];        // Chunk ID: fmt
    u32 cksize;          //	Chunk size: 16, 18 or 40
    u16 wFormatTag;      //	Format code
    u16 nChannels;       // Number of interleaved channels
    u32 nSamplesPerSec;  //	Sampling rate (blocks per second)	//unitÔº? bit
    u32 nAvgBytesPerSec; // Data rate
    u16 nBlockAlign;     //	Data block size (bytes)
    u16 wBitsPerSample;  //	Bits per sample
#if AUDIO_FMT_EXTENSION_ON_OFF
    u16 cbSize;              // Size of the extension (0 or 22)
    u16 wValidBitsPerSample; //	Number of valid bits
    u32 dwChannelMask;       //	Speaker position mask
    u8 SubFormat[16];        //	Speaker position mask
#endif
} Audio_FMT_Chunk;

#if AUDIO_FACT_ON_OFF
typedef struct fact
{
    char ckID[4];       // Chunk ID: fact
    u32 cksize;         // Chunk size: minimum 4
    u16 dwSampleLength; // Number of samples (per channel)
} Audio_Fact_Chunk;
#endif

typedef struct data
{
    char ckID[4];     // Chunk ID: data
    u32 cksize;       // Chunk size: n
    char *sampleData; //	Samples
    int pad_byte;     // 0 or 1 Padding byte if is oddn
} Audio_Data_Chunk;

typedef struct WAV
{
    char ckID[4];   //	Chunk ID: RIFF
    u32 cksize;     // Chunk size: 4+n
    char WAVEID[4]; //	WAVE ID: WAVE
    Audio_FMT_Chunk fmt_ck;
#if AUDIO_FACT_ON_OFF
    Audio_Fact_Chunk fact_ck;
#endif
    Audio_Data_Chunk data_ck;
    u32 wavLen;
} Audio_WAV_Info;

typedef struct
{
    int16_t *Ch_r; // ”“…˘µ¿
    int16_t *Ch_l; // ◊Û…˘µ¿
    u32 Len;       //  ˝æ›≥§∂»£¨◊Û”“…˘µ¿≥§∂»±ÿ–Î“ª÷¬
} Wav_CH_Data;

int WAV_Format_parsing(Audio_WAV_Info *wav_info, char *audio_wav_ori);
static int audio_wave_info_verify(const Audio_WAV_Info *wav_info);
char *my_strnstr_kmp(const char *s1, const char *s2, size_t n);

/***    ”¶”√≤„    ***/
#include "user_bsp.h"

#define Wav_Printf Uart1_SendData

#define Def_Data_16to12_single(data) ((short)(data + (1 << 15)) >> 4)
#define Wav_Process_SingTrack(in, out, len) Data_16to12_Mult(in, out, len)

int Data_16to12_Mult(short *ret, short *data, int len);
int Wav_Process_DualTrack(int16_t *out_r, int16_t *out_l, int16_t *in, u32 len);

#endif

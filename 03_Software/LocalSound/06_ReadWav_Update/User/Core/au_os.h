/***
 * @Author: YourName
 * @Date: 2025-01-22 10:30:01
 * @LastEditTime: 2025-02-13 13:49:59
 * @LastEditors: YourName
 * @Description:
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\04_MyPrj\02_BleAudio\03_Software\LocalSound\06_ReadWav_Update\User\Core\au_os.h
 * @°æÈ¨ÉùÃ÷
 */
#ifndef __AUDIO_OS_H__
#define __AUDIO_OS_H__

#include "basic.h"

#include "fatfs.h"

#define Wav_PlayBit_DacNotify 0x01
#define Wav_PlayBit_Stop 0x02
#define Wav_PlayBit_Resume 0x04

extern TaskHandle_t Wav_Task_Handle;

void Start_Wav(void);
void Wav_Task(void const *argument);
void printf_WavInfo(short *data, int len);

#endif

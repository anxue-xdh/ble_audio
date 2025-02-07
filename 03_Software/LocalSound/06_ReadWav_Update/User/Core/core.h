/***
 * @Author: YourName
 * @Date: 2025-01-22 10:30:01
 * @LastEditTime: 2025-02-06 15:55:45
 * @LastEditors: YourName
 * @Description:
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\04_MyPrj\02_BleAudio\03_Software\LocalSound\06_ReadWav_Update\User\Core\core.h
 * @°æÈ¨ÉùÃ÷
 */
/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-12 10:11:47
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-14 11:28:47
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\VsCodeStm32\AudioPlayer\UserCore\Inc\basic.h
 * @Description: è¿™æ˜¯é»˜è?¤è?¾ç½®,è¯·è?¾ç½®`customMade`, æ‰“å¼€koroFileHeaderæŸ¥çœ‹é…ç½® è¿›è?Œè?¾ç½®: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __CORE_H__
#define __CORE_H__

#include "basic.h"

#include "fatfs.h"

void ReadWav(void const *argument);

void printf_WavInfo(short *data, int len);

FRESULT SD_Read_FileInfo(const char *path, char (*list)[64]);

#endif

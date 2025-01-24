/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-12 10:11:47
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-14 11:28:47
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\VsCodeStm32\AudioPlayer\UserCore\Inc\basic.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __CORE_H__
#define __CORE_H__

#include "basic.h"

#include "fatfs.h"

void ReadWav(void const *argument);

short Data_16to12_single(short data);
int Data_16to12_Mult(short *ret, short *data, int len);
int Data_16to12_Mult_one(short *data, int len);

void printf_WavInfo(short *data, int len);

FRESULT SD_Read_FileInfo(const char *path);

#endif

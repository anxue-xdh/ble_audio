/***
 * @Author: YourName
 * @Date: 2025-01-22 10:30:01
 * @LastEditTime: 2025-01-22 10:46:30
 * @LastEditors: YourName
 * @Description:
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\04_MyPrj\02_BleAudio\03_Software\LocalSound\06_ReadWav_Update\User\BSP\user_bsp_config.h
 * @°æÈ¨ÉùÃ÷
 */
#ifndef __BSP_CONFIG_H
#define __BSP_CONFIG_H

#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"

#include "stm32f1xx_hal.h"

// #define u8 unsigned char
// #define u16 unsigned short
// #define u32 unsigned int
// #define u64 unsigned long
// #define s8 signed char
// #define s16 signed short
// #define s32 signed int
// #define s64 signed long

#define False 0
#define True 1

#define BASECLOCK 72000000 / (71 + 1)


// #define Audio_Mode_Local
#define Audio_Mode_BLe


#endif

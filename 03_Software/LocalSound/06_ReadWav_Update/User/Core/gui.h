/***
 * @Author: YourName
 * @Date: 2025-02-07 15:16:27
 * @LastEditTime: 2025-02-13 14:35:54
 * @LastEditors: YourName
 * @Description:
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\04_MyPrj\02_BleAudio\03_Software\LocalSound\06_ReadWav_Update\User\Core\gui.h
 * @°æÈ¨ÉùÃ÷
 */
#ifndef __GUI_H__
#define __GUI_H__

#include "basic.h"

#include "fatfs.h"

#define SDFile_Name_Len 64
#define SDFile_Name_Num 16

#define GUI_TaskBit_MusicList_Update 0x01
#define GUI_TaskBit_Key_Up 0x02
#define GUI_TaskBit_Key_Down 0x04

extern TaskHandle_t GUI_Task_Handle;
extern signed char MusicList_Pointer;
extern char MusicList_Num;
extern char MusicList[SDFile_Name_Num][SDFile_Name_Len];

void GUI_Init(void);
void GUI_Task(void *argument);
FRESULT SD_Read_FileInfo(const char *path, char (*list)[64]);

#endif

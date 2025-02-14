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
#include "my_list.h"

#define SDFile_Name_Len 64
#define SDFile_Name_Num 16

#define GUI_TaskBit_MusicList_Update 0x01
#define GUI_TaskBit_Key_Up 0x02
#define GUI_TaskBit_Key_Down 0x04

extern TaskHandle_t GUI_Task_Handle;
extern u8 MicList_Idx_Gui;
extern u8 MicList_Idx_au;
extern yList MicList;
// extern char MusicList[SDFile_Name_Num][SDFile_Name_Len];

void GUI_Init(void);
void GUI_Task(void *argument);
FRESULT MicList_Update(const char *path);
void MicList_reinit(void);

#endif

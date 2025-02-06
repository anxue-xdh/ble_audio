#ifndef __GUI_H__
#define __GUI_H__

#include "basic.h"

#define SDFile_Name_Len 64
#define SDFile_Name_Num 16

#define GUI_MusicList_Update 0x01
#define GUI_Key_Up 0x02
#define GUI_Key_Down 0x04

extern signed char musicList_Pointer;
extern char musicList_Num;
extern char sdFile_Name[SDFile_Name_Num][SDFile_Name_Len];

void GUI_Init(void);
void GUI_Task(void);

#endif

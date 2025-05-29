/***
 * @Author: YourName
 * @Date: 2025-01-22 10:30:01
 * @LastEditTime: 2025-02-13 13:49:59
 * @LastEditors: YourName
 * @Description:
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\04_MyPrj\02_BleAudio\03_Software\LocalSound\06_ReadWav_Update\User\Core\au_os.h
 * @°æÈ¨ÉùÃ÷
 */
#ifndef __AUDIO_BLE_H__
#define __AUDIO_BLE_H__

#include "basic.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

// #include "fatfs.h"
#define BleBuff_Size 1024

#define Ble_Bit_Rx_Compelete 0x01
#define Ble_Bit_Rx_Half 0x02

extern TaskHandle_t Ble_Task_Handle;

void Start_Ble(void);
void Ble_Task(void const *argument);
void printf_WavInfo(short *data, int len);

#endif

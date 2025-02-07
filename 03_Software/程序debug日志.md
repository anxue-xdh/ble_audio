[TOC]



## 2025年1月22日 

问题描述：sd卡频繁出现FR_DISK_ERR错误，导致歌曲播放中断

> [!Debug Info]
>
> wav_len:166
>
> old pointer:16492710
> now pointer:16493056
> pointer diff:346
>
> old pointer:2765990
> now pointer:2766336
> pointer diff:346

wav_len+pointer diff = 512

每次出现错误时的now pointer的数值刚好能被512整除。

问题应该是出在block size的大小在512，进行跨越block size进行数据读取时，  产生了FR_DISK_ERR错误。

问题描述：在修改Blcok size后，重新进行实验，仍然会产生该错误

> [!Debug Info]
>
> [15:20:31.783]收←◆！！文件读取失败：(1)
> old pointer:14365184
> now pointer:14365184
> pointer diff:0
>
> [15:23:34.106]收←◆！！文件读取失败：(1)
> old pointer:14365184
> now pointer:14365184
> pointer diff:0
>
> [15:27:32.037]收←◆！！文件读取失败：(1)
> old pointer:9651712
> now pointer:9651712
> pointer diff:0
> 指针移动失败，f_res:(1)
>
> [15:30:32.742]收←◆！！文件读取失败：(1)
> old pointer:12030464
> now pointer:12030464
> pointer diff:0
> 指针移动失败，f_res:(1)
>
> [15:38:30.661]收←◆文件读取完毕
> 关闭文件

### 更改spi速率，64分频

15:40 从4分频改到64分频 速率从18Mbit/s->1.125Mbit/s

> [!Debug Info]
>
> [15:42:38.599]收←◆！！文件读取失败：(1)
> old pointer:5260288
> now pointer:5260288
> pointer diff:0
> 指针移动失败，f_res:(1)
>
> [15:47:01.852]收←◆！！文件读取失败：(1)
> old pointer:5260288
> now pointer:5260288		
> pointer diff:0
> 指针移动失败，f_res:(1)
>
> [15:48:26.749]收←◆！！文件读取失败：(1)
> old pointer:5260288
> now pointer:5260288		10274
> pointer diff:0
> 指针移动失败，f_res:(1)
>
> 指针移动成功
> 》文件读取成功,读到字节数据：512
> 》文件读取成功,读到字节数据：512
> 循环开始
>
> [15:50:02.345]收←◆[ERROR] Uart1 obliterated Data!!
> [15:50:02.391]收←◆[ERROR] Uart1 obliterated Data!!
> [15:50:03.267]收←◆[ERROR] Uart1 obliterated Data!!
> [15:50:03.424]收←◆[ERROR] Uart1 obliterated Data!!
> [15:50:05.578]收←◆[ERROR] Uart1 obliterated Data!!
> [15:50:05.625]收←◆[ERROR] Uart1 obliterated Data!!
>
> [15:52:19.016]收←◆！！文件读取失败：(1)
> old pointer:9773056
> now pointer:9773056		19088
> pointer diff:0
> 指针移动失败，f_res:(1)
>
> [15:54:00.576]收←◆！！文件读取失败：(1)
> old pointer:6049792
> now pointer:6049792		11816
> pointer diff:0
> 指针移动失败，f_res:(1)

### spi速率，32分频

16:00 64分频速度偏低，会偶尔产生通知数量超过1，处理速度无法正常跟上，目前已调整至32分频，可以正常运行obliterated Data错误。

FATFS官方的解释：

### FR_DISK_ERR

The lower layer, `disk_read`, `disk_write` or `disk_ioctl` function, reported that an unrecoverable hard error occured.
Note that if once this error occured at any operation to an open file, the file object is aborted and any operations to the file except for close will be rejected.（出现FR_DISK_ERR后，文件只能f_Close，其他函数无法正常使用）

可以考虑在问题出现后，先关闭文件，再将文件指针重新指向出现错误的位置，如果在处理速度够快的情况下，应该可以正常运行不影响整体播放。

> [!Debug Info]
>
> [16:54:28.672]收←◆文件读取完毕
> 关闭文件
>
> [16:59:44.258]收←◆文件读取完毕
> 关闭文件
>
> [17:03:59.050]收←◆文件读取完毕
> 关闭文件
>
> [17:07:03.986]收←◆！！文件读取失败：(1)
> old pointer:14510592
> now pointer:14510592
> pointer diff:0
> 指针移动失败,f_res:(1)

16:50~17:05   三次连续实验，均为出现问题。

------

## 2025年1月23日

### 读取失败处理

更改了文件读取失败的处理逻辑。读取失败后，关闭文件，重新定位指针，重新读取数据。

```c
Uart1_SendData("！！文件读取失败：(%d)\r\n", f_res);

f_close(&file);
Uart1_SendData("关闭文件\r\n");

f_res = f_open(&file, tempfilepath, FA_OPEN_EXISTING | FA_READ);

if (f_res != FR_OK)
{
    Uart1_SendData("重新打开失败\r\n");
    break;
}

f_res = f_lseek(&file, oldPtr);
if (f_res != FR_OK)
{
    Uart1_SendData("指针移动失败,f_res:(%d)\r\n", f_res);
    break;
}
Uart1_SendData("重新传输开始\r\n");

continue;
```

> [!Debug info]
>
> [09:10:36.903]收←◆！！文件读取失败：(1)
> 关闭文件
> 重新传输开始
> [ERROR] Core obliterated Data!!
> [09:11:42.012]收←◆！！文件读取失败：(1)
> 关闭文件
> [09:11:42.042]收←◆重新传输开始
> [ERROR] Core obliterated Data!!
> [09:12:21.674]收←◆文件读取完毕
> 关闭文件
>
> [09:15:37.722]收←◆！！文件读取失败：(1)
> 关闭文件
> [09:15:37.752]收←◆重新传输开始
> [ERROR] Core obliterated Data!!
> [09:16:14.714]收←◆文件读取完毕
> 关闭文件
>
> [09:19:23.464]收←◆！！文件读取失败：(1)
> 关闭文件
> 重新传输开始
> [ERROR] Core obliterated Data!!
> [09:20:00.456]收←◆文件读取完毕
> 关闭文件
>
> [09:25:15.201]收←◆！！文件读取失败：(1)
> 关闭文件
> 重新传输开始
> [ERROR] Core obliterated Data!!
> [09:25:23.100]收←◆！！文件读取失败：(1)
> 关闭文件
>
> [09:25:23.126]收←◆重新传输开始
> [ERROR] Core obliterated Data!![ERROR] Core obliterated Data!!
> [09:25:24.869]收←◆！！文件读取失败：(1)
> 关闭文件
> 重新传输开始
> [ERROR] Core obliterated Data!!
> [09:25:49.740]收←◆！！文件读取失败：(1)
> 关闭文件
> [09:25:49.767]收←◆重新传输开始
> [ERROR] Core obliterated Data!!
> [09:26:04.410]收←◆！！文件读取失败：(1)
> 关闭文件
> [09:26:04.437]收←◆重新传输开始
> [ERROR] Core obliterated Data!!
> [09:26:31.105]收←◆！！文件读取失败：(1)
> 关闭文件
> 重新传输开始
> [ERROR] Core obliterated Data!!
> [09:26:32.677]收←◆！！文件读取失败：(1)
> 关闭文件
> [09:26:32.707]收←◆重新传输开始
> [ERROR] Core obliterated Data!!
> [09:26:32.904]收←◆！！文件读取失败：(1)
> 关闭文件
> [09:26:32.933]收←◆重新传输开始
> [ERROR] Core obliterated Data!!
> [09:27:08.178]收←◆文件读取完毕
> 关闭文件

这一段调试内容没有接喇叭，实际效果需要之后重新测试。



## 2025年2月5日

添加了emWin的功能，但是因为芯片容量问题无法正常移植。出现报错

> No space in execution regions

考虑更换芯片，目前暂定位stm32g473rct6。512KB ROM,128KB RAM。

目前的话，ui交互就是临时使用原有的英文显示函数即可。

触摸功能使用了SPI1，与SD卡冲突，因此不使用触摸功能，后续增加相对应的功能。目前准备使用按键进行代替。高亮选择。



## 2.6

新增了列表选歌功能。目前不支持翻页功能。通过按键和串口功能进行上下切换选歌，蓝底高亮显示选中歌曲。



准备每次播放时，重新创建一个音乐播放任务，因此需要获取任务的运行状态。但是使用eTaskGetState()函数无法正确获得状态，在任务使用vTaskDelete();删除后，返回值仍然是eSuspended，挂起态，无法正常获取到eDeleted状态。

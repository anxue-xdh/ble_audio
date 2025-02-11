### #0001（Core obliterated Data）

每次重新开始传输，都会产生一个Core obliterated Data!!，这个问题需要去考虑解决。

### #0002（Core obliterated Data）

当重新开始传输时，产生Core obliterated Data!!错误，会导致喇叭产生一个尖刺噪音。

### #0003（Core obliterated Data）

当在喇叭播放时，使用串口发送命令，如果该命令会被系统响应，同样会触发Core obliterated Data错误，然后会导致喇叭的声音产生尖锐感，直到播放结束都不会恢复正常。

> RX：[DEBUG] Query task resource
> Uart1_Scan_Task	X	5	76	8
> ReadWav        	R	5	1671	9
> IDLE           	R	0	38	2
> Led_Ctr_Task   	B	2	43	4
> Led_Test_Task  	B	2	44	5
> Key_Scan_Task  	B	2	43	6
> Key_Run_Task   	B	2	100	7
> Tmr Svc        	B	2	100	3
> [ERROR] Core obliterated Data!![ERROR] Core obliterated Data!!

### #0004（播放结束尖啸）

当音乐播放完毕时，喇叭会产生一个尖锐噪音。

### ~~#0005（任务删除后仍然是挂起态）~~

~~eTaskGetState()函数入参错误，输入了其他任务的句柄，导致无法获取到正确的数据~~



### #0006（多块写入）

在之后排查读取数据的时候，发现sd卡的底层驱动程序，在多块读取存在错误，没有使用专门的多块读命令，而是采用了多次运行单块读命令。目前多块读取已完成修复，但是多块写入暂时搁置，等到后续需要时进修改。


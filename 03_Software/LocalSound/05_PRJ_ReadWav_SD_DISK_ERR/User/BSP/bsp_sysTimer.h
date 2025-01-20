#ifndef __BSP_SYSTIMER_H
#define __BSP_SYSTIMER_H

typedef struct BSP_SYSTIEMR
{
    char Cnt;
    short Ms;
    char Second;
    char Minute;
} sys_Timer;

typedef struct BSP_SYSTIMER_LOW
{
    char Cnt;
    short Ms;
} sys_Timer_Low;

enum Timer_Type
{
    MS = 0,
    SECOND,
    MINUTE,
};

extern sys_Timer Rtc;


#define RTC_CNT_TIMER_INTER 100
#define RTC_CNT_NUM 10


void SysTimer_Rtc_IRQ(void);

void SysTimer_Timing_Restart(sys_Timer *tim);
void SysTimer_Timing_Restart_Low(sys_Timer_Low *tim);
void SysTimer_Timing_Sync(sys_Timer *rtc, sys_Timer *tim);
void SysTimer_Timing_Sync_Low(sys_Timer *rtc, sys_Timer_Low *tim);
char SysTimer_TimingFunc(sys_Timer *rtc, sys_Timer *tim, enum Timer_Type type, int length);
char SysTimer_TimingFunc_Low(sys_Timer *rtc, sys_Timer_Low *tim, int length);


#define SysTimer_Timing_Sync_Rtc(tim) SysTimer_Timing_Sync(&Rtc,tim)
#define SysTimer_Timing_Sync_Rtc_Low(tim) SysTimer_Timing_Sync(&Rtc,tim)
#define SysTimer_Timing_Ms(tim, len) SysTimer_TimingFunc(&Rtc, tim, MS, len)
#define SysTimer_Timing_Second(tim, len) SysTimer_TimingFunc(&Rtc, tim, SECOND, len)
#define SysTimer_Timing_Minute(tim, len) SysTimer_TimingFunc(&Rtc, tim, MINUTE, len)
#define SysTimer_Timing_Ms_Low(tim, len) SysTimer_TimingFunc_Low(&Rtc, tim, len)

#endif



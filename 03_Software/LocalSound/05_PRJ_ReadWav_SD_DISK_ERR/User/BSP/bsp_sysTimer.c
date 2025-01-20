#include "bsp_sysTimer.h"

sys_Timer Rtc = {0};

void SysTimer_Rtc_IRQ(void)
{
	Rtc.Cnt++;
	if(Rtc.Cnt >= RTC_CNT_NUM)
	{
		Rtc.Cnt=0;
		Rtc.Ms++;
		if(Rtc.Ms>=1000)
		{
			Rtc.Ms=0;
			Rtc.Second++;
			if(Rtc.Second>=60)
			{
				Rtc.Second=0;
				Rtc.Minute++;
				if(Rtc.Minute >= 60)
					Rtc.Minute=0;
			}
		}
	}
}


void SysTimer_Timing_Restart(sys_Timer *tim)
{
	tim->Cnt = 0;
	tim->Ms = 0;
	tim->Second= 0;
	tim->Minute = 0;
}

void SysTimer_Timing_Restart_Low(sys_Timer_Low *tim)
{
	tim->Cnt = 0;
	tim->Ms = 0;
}



void SysTimer_Timing_Sync(sys_Timer *rtc, sys_Timer *tim)
{
	tim->Cnt = rtc->Cnt;
	tim->Ms = rtc->Ms;
	tim->Second= rtc->Second;
	tim->Minute = rtc->Minute;
}

void SysTimer_Timing_Sync_Low(sys_Timer *rtc, sys_Timer_Low *tim)
{
	tim->Cnt = rtc->Cnt;
	tim->Ms = rtc->Ms;
}


char SysTimer_TimingFunc(sys_Timer *rtc, sys_Timer *tim, enum Timer_Type type, int length)
{
	int temp =0;
	switch (type)
	{
	case MINUTE:
		if (rtc->Minute > tim->Minute)
		{
			temp = rtc->Minute - tim->Minute;
			if(temp >= length)
				if(rtc->Second >= tim->Second)
					return 1;
		}
		else if(rtc->Minute < tim->Minute)
		{
			temp = (60 + rtc->Minute) - tim->Minute;
			if(temp >= length)
				if(rtc->Second >= tim->Second)
					return 2;
		}
		break;
	case SECOND:
		if(rtc->Second >tim->Second)
		{
			temp = rtc->Second - tim->Second;
			if(temp >= length)
				if(rtc->Ms >= tim->Ms)
					return 1;
		}
		else if(rtc->Second < tim->Second)
		{
			temp = (60 + rtc->Second) - tim->Second;
			if(temp >= length)
				if(rtc->Ms >= tim->Ms)
					return 1;
		}
		break;
	case MS:
		if(rtc->Ms >tim->Ms)
		{
			temp = rtc->Ms - tim->Ms;
			if(temp >= length)
				if(rtc->Cnt >= tim->Cnt)
					return 1;
		}
		else if(rtc->Ms < tim->Ms)
		{
			temp = (1000 + rtc->Ms) - tim->Ms;
			if(temp >= length)
				if(rtc->Cnt >= tim->Cnt)
					return 1;
		}
		break;

	default:
		break;
	}
	return 0;
}

char SysTimer_TimingFunc_Low(sys_Timer *rtc,sys_Timer_Low *tim,int length)
{
	int temp =0;
	if(rtc->Ms >tim->Ms)
	{
		temp = rtc->Ms - tim->Ms;
		if(temp >= length)
			if(rtc->Cnt >= tim->Cnt)
				return 1;
	}
	else if(rtc->Ms < tim->Ms)
	{
		temp = (1000 + rtc->Ms) - tim->Ms;
		if(temp >= length)
			if(rtc->Cnt >= tim->Cnt)
				return 1;
	}
	return 0;
}

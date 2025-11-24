#include <REGX52.H>
#include "event.h"

static unsigned long ms_counter = 0; // 毫秒计数器

void Timer2_Init(void)        //10毫秒@12.000MHz
{
    T2MOD = 0;                //初始化模式寄存器
    T2CON = 0;                //初始化控制寄存器
    TL2 = 0xF0;               //设置定时初始值
    TH2 = 0xD8;               //设置定时初始值
    RCAP2L = 0xF0;            //设置定时重载值
    RCAP2H = 0xD8;            //设置定时重载值
    ET2 = 1;                  //使能Timer2中断
    EA = 1;                   //开启总中断
    TR2 = 1;                  //定时器2开始计时
}

void Timer2_Isr() interrupt 5
{
    static unsigned char T2_Count = 0;
	static unsigned char Stopwatch_Count = 0; 
    TF2 = 0;                  //清除中断标志
    
    // 每次中断增加10毫秒（因为定时器配置为10ms中断）
    ms_counter += 10;
	Stopwatch_Count++; 
	if(Stopwatch_Count >= 10) // 10 * 10ms = 100ms
    {
        Stopwatch_Count = 0;
        // 只有当前模式是秒表时才发，节省CPU资源
        if(Event_GetCurrentMode() == SYS_MODE_STOPWATCH) {
            Event_Publish(EVENT_STOPWATCH_UPDATE, 0, SYS_MODE_STOPWATCH);
        }
    }
    
    T2_Count++;
    if(T2_Count >= 100)        // 10ms * 100 = 1000ms (1秒)
    {
        T2_Count = 0;
        Event_Publish(EVENT_TIME_TIMER_1S, 0, Event_GetCurrentMode());
    }
}

// 获取当前毫秒数
unsigned long Timer_GetMilliseconds(void)
{
    unsigned long temp;
    EA = 0;             // 1. 先关中断（只要我在读，定时器就不许改数据）
    temp = ms_counter;  // 2. 安全地读取
    EA = 1;             // 3. 恢复中断
    return temp;        // 4. 返回读取到的值
}
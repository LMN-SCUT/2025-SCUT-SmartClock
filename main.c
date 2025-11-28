#include <REGX52.H>
#include "LCD1602.h"
#include "event.h"
#include "timer_manage.h"
#include "timer.h"        // 包含Timer2_Init原型
#include "display_manage.h"
#include "keypad.h"
#include "delay.h"
#include "alarm_manage.h"
#include"buzzer.h"
#include "DS3231.H"
#include "i2c_hal.h"
#include "AT24C32.H"
#include  "number_input.h"
#include  "stopwatch_manage.h"
//#include "watch_dog.h"

sbit ALARM_LED = P2^4;//定义LED灯地址

unsigned char blink_interval = 500;

void Delay500ms(void);



void main() {
    // 傻逼C89标准
	static unsigned long last_blink_time = 0;  //闪烁计时
	ALARM_LED = 1;
    
    // 初始化所有模块
    Display_Init();	 //初始化显示屏
    Timer2_Init();   //初始化计时器
	Delay500ms();
	Delay500ms();
	Buzzer_Init();	 //初始化蜂鸣器
	I2C_Init();      // 初始化I2C总线
    DS3231_Init();   // 初始化DS3231
	Alarm_Read_From_EEPROM(); //从存储芯片中读取数值
	DS3231_ReadTime();		//把数值传到时间管理
	Number_Input_Init();	//数字输入系统重置
    Stopwatch_Init();		//秒表重置
//	WDT_Init(); 			//看门狗开始
	

        Event_Publish(EVENT_TIME_HOUR_UPDATED, ds_hour, SYS_MODE_CLOCK);
        Event_Publish(EVENT_TIME_MINUTE_UPDATED, ds_min, SYS_MODE_CLOCK);
        Event_Publish(EVENT_TIME_SECOND_UPDATED, ds_sec, SYS_MODE_CLOCK);
    
    
    while(1) {
        // 处理键盘事件
        Key_ProcessEvents();
        
        // 处理系统事件
        Event_Process();
		Buzzer_Update();
//		WDT_Feed();          //喂狗 
		if (Timer_GetMilliseconds() % 1000 == 0) { // 每秒强制刷
    Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_CLOCK);
}
        
        // 闪烁更新
        if (Timer_GetMilliseconds() - last_blink_time >= blink_interval) { // 精确的500ms
            last_blink_time = Timer_GetMilliseconds();
            
            if(Event_GetCurrentMode() == SYS_MODE_TIME_SET) {
                Time_Blink_Update();
            }
            if(Event_GetCurrentMode() == SYS_MODE_ALARM_SET) {
                Alarm_Blink_Update();
            }
			 if (Alarm_IsRinging()) {
            
             ALARM_LED = !ALARM_LED; 
             
             // 全屏闪烁事件
             Event_Publish(EVENT_DISPLAY_FLSAH, ALARM_LED, SYS_MODE_CLOCK);
        }
        }
        
    }
}

void Delay500ms()		//@12.000MHz
{
	unsigned char i, j, k;
	i = 4;
	j = 205;
	k = 187;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}   

/*目前的BUG
1.时间增减模式INC与ENC键无效
2.秒表的计时逻辑貌似有问题，计时到60秒时会突变为256，然后走到261，然后归零
3.秒表的显示逻辑不太正常
4.闹钟模式的初值错误，INC与ENC无效
5.显示数字底部有误，可能需要强制占位




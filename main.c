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

sbit ALARM_LED = P2^4;//定义LED灯地址

void main() {
    // 傻逼C89标准
	static unsigned long last_blink_time = 0;  //闪烁计时
    
    // 初始化所有模块
    Display_Init();	 //初始化显示屏
    Timer2_Init();   //初始化计时器
	Buzzer_Init();	 //初始化蜂鸣器
	I2C_Init();      // 初始化I2C总线
    DS3231_Init();   // 初始化DS3231
	Alarm_Read_From_EEPROM(); //从存储芯片中读取数值
	DS3231_ReadTime();		//把数值传到时间管理
	Number_Input_Init();	//数字输入系统重置
    Stopwatch_Init();		//秒表重置
	

    // 简单的判断：如果读取到的时间全是0（可能是新电池），则设个默认值
    if(ds_hour == 0 && ds_min == 0 && ds_sec == 0) {
        // 第一次运行，设置一个初始值，比如 12:00:00
        DS3231_SetTime(12, 0, 0);
        // 更新到系统事件，让屏幕显示
        Event_Publish(EVENT_TIME_HOUR_UPDATED, 12, SYS_MODE_CLOCK);
        Event_Publish(EVENT_TIME_MINUTE_UPDATED, 0, SYS_MODE_CLOCK);
        Event_Publish(EVENT_TIME_SECOND_UPDATED, 0, SYS_MODE_CLOCK);
    } else {
        // 不是第一次，将读到的硬件时间同步给系统
        Event_Publish(EVENT_TIME_HOUR_UPDATED, ds_hour, SYS_MODE_CLOCK);
        Event_Publish(EVENT_TIME_MINUTE_UPDATED, ds_min, SYS_MODE_CLOCK);
        Event_Publish(EVENT_TIME_SECOND_UPDATED, ds_sec, SYS_MODE_CLOCK);
    }
    
    while(1) {
        // 处理键盘事件
        Key_ProcessEvents();
        
        // 处理系统事件
        Event_Process();
		Buzzer_Update();
        
        // 闪烁更新
        if (Timer_GetMilliseconds() - last_blink_time >= 500) { // 精确的500ms
            last_blink_time = Timer_GetMilliseconds();
            
            if(Event_GetCurrentMode() == SYS_MODE_TIME_SET) {
                Time_Blink_Update();
            }
            if(Event_GetCurrentMode() == SYS_MODE_ALARM_SET) {
                Alarm_Blink_Update();
            }
			 if (Alarm_IsRinging()) {
            
             ALARM_LED = !ALARM_LED; 
             
             // 顺便让屏幕也跟着闪（全屏闪烁事件）
             Event_Publish(EVENT_DISPLAY_FLSAH, ALARM_LED, SYS_MODE_CLOCK);
        }
        }
        
    }
}

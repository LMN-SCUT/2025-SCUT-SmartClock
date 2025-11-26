#include <REGX52.H>
#include "timer_manage.h"
#include "event.h"
#include "display_manage.h"
#include "ds3231.h"

// 静态全局变量
static idata unsigned char hours = 12;
static idata unsigned char minutes = 0;
static idata unsigned char seconds = 0;

static idata unsigned char last_hour = 12;
static idata unsigned char last_min = 0;
static idata unsigned char last_seconds = 0;

static idata TimeSubMode current_time_submode = TIME_SUBMODE_NORMAL;
static idata TimeUnit selected_unit = UNIT_HOUR;
static idata unsigned char blink_counter = 0;
static idata unsigned char global_blink_state = 0;

// 获取函数
unsigned char Time_GetHour(void) { return hours; }
unsigned char Time_GetMin(void) { return minutes; }
unsigned char Time_GetSec(void) { return seconds; }
TimeSubMode Time_GetCurrentSubMode(void) { return current_time_submode; }
TimeUnit Time_GetSelectedUnit(void) { return selected_unit; }

// 调整选中单位的数值
static void Adjust_Selected_Unit(int delta) {
    switch(selected_unit) {
        case UNIT_HOUR:
            hours = (hours + delta + 24) % 24;
            Event_Publish(EVENT_TIME_HOUR_UPDATED, hours, SYS_MODE_TIME_SET);
            break;
        case UNIT_MINUTE:
            minutes = (minutes + delta + 60) % 60;
            Event_Publish(EVENT_TIME_MINUTE_UPDATED, minutes, SYS_MODE_TIME_SET);
            break;
        case UNIT_SECOND:
            seconds = (seconds + delta + 60) % 60;
            Event_Publish(EVENT_TIME_SECOND_UPDATED, seconds, SYS_MODE_TIME_SET);
            break;
    }
	 DS3231_SetTime(hours, minutes, seconds);
}

// 时间走时逻辑
static void Time_Tick(void) {
	   static unsigned char last_min = 60;
	   static unsigned char last_hour = 25;		//傻逼C89标准，害我查了半天
	   unsigned char i;
   	   bit read_success = 0;
    // 1. 从硬件读取最新时间	 
	 for(i=0 ; i<3 ; i++) //读取超时重试
	 if(DS3231_ReadTime()==1){
	 read_success = 1;
	 break;
	 }
    
    // 2. 更新内部变量 (hours, minutes, seconds 是 timer_manage.c 里的静态变量) 
    // 检查是否发生变化，减少不必要的事件发布（虽然每秒肯定变）
	if (read_success){
    seconds = ds_sec;
    minutes = ds_min;
    hours = ds_hour;
    
    // 3. 发布更新事件，通知显示模块
    Event_Publish(EVENT_TIME_SECOND_UPDATED, seconds, Event_GetCurrentMode());
    
    // 只有分变了才发分事件（减少LCD闪烁）

    if(minutes != last_min) {
        last_min = minutes;
        Event_Publish(EVENT_TIME_MINUTE_UPDATED, minutes, Event_GetCurrentMode());
        
        // 只有时变了才发时事件

        if(hours != last_hour) {
            last_hour = hours;
            Event_Publish(EVENT_TIME_HOUR_UPDATED, hours, Event_GetCurrentMode());
        }
    }
  }
}


// 时间设置状态完全复位
static void Time_Reset_Setup_State(void) {
    current_time_submode = TIME_SUBMODE_SELECT_UNIT;
    selected_unit = UNIT_HOUR;
}

// 处理不同系统模式下的事件
void Time_Manage(Event* e) {
    SystemMode current_sys_mode = Event_GetCurrentMode();
    
    // 1秒定时事件处理（所有模式都需要）
    if(e->type == EVENT_TIME_TIMER_1S) {
        Time_Tick();
    }
    
    // 根据当前系统模式处理其他事件
    switch(current_sys_mode) {
        case SYS_MODE_CLOCK:
            // 时钟模式下，MOD键进入时间设置
            if(e->type == EVENT_KEY_MODE) {
                Time_Reset_Setup_State(); // 确保状态重置
                Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_TIME_SET, current_sys_mode);
            }
            break;
            
        case SYS_MODE_TIME_SET:
            // 时间设置模式下的事件处理
            switch(current_time_submode) {
                case TIME_SUBMODE_SELECT_UNIT:
                    // 选择单位模式
                    switch(e->type) {
                        case EVENT_KEY_INCREMENT:
                            selected_unit = (selected_unit + 1) % 3;
                            Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_TIME_SET);
                            break;
                        case EVENT_KEY_DECREMENT:
                            selected_unit = (selected_unit == 0) ? 2 : (selected_unit - 1);
                            Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_TIME_SET);
                            break;
                        case EVENT_KEY_ENTER:
                            current_time_submode = TIME_SUBMODE_ADJUST_VALUE;
                            Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_TIME_SET);
                            break;
                        case EVENT_KEY_SET:
                            // 退出时间设置模式，返回时钟模式
                            current_time_submode = TIME_SUBMODE_NORMAL;
                            Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, SYS_MODE_TIME_SET);
                            break;
                    }
                    break;
                    
                case TIME_SUBMODE_ADJUST_VALUE:
                    // 调整数值模式
                    switch(e->type) {
                        case EVENT_KEY_INCREMENT:
                            Adjust_Selected_Unit(1);
                            break;
                        case EVENT_KEY_DECREMENT:
                            Adjust_Selected_Unit(-1);
                            break;
                        case EVENT_KEY_ENTER:
                            current_time_submode = TIME_SUBMODE_SELECT_UNIT;
                            Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_TIME_SET);
                            break;
                        case EVENT_KEY_SET:
                            // 确认并退出时间设置
                            current_time_submode = TIME_SUBMODE_NORMAL;
                            Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, SYS_MODE_TIME_SET);
                            break;
                    }
                    break;
                    
                default:
                    break;
            }
            break;
            
        default:
            // 其他系统模式（秒表、闹钟设置）的处理可以在这里添加
            break;
    }
}

// 闪烁更新函数（需要在主循环中定期调用）
void Time_Blink_Update(void) {  
            global_blink_state = !global_blink_state;
            Event_Publish(EVENT_DISPLAY_BLINK, global_blink_state, SYS_MODE_TIME_SET);
        }
    
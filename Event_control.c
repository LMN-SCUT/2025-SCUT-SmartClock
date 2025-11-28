#include <REGX52.H>
#include "event.h"
#include "display_manage.h"
#include "timer_manage.h"
#include "timer.h"
#include "alarm_manage.h"  
#include "number_input.h"
#include "menu_manage.h"  
#include "stopwatch_manage.h"

#define MAX_EVENTS 30
static xdata Event event_queue[MAX_EVENTS];
static xdata unsigned char queue_head = 0, queue_tail = 0;

// 全局系统模式
static SystemMode current_mode = SYS_MODE_CLOCK;

// 发布事件
void Event_Publish(EventType type, int dat, SystemMode mode) reentrant{
    unsigned char ea_save = EA; 
	EA =0; //中断保护数据
	if ((queue_tail + 1) % MAX_EVENTS != queue_head){
    event_queue[queue_tail].type = type;
    event_queue[queue_tail].dat = dat;
    event_queue[queue_tail].mode = mode;
    queue_tail = (queue_tail + 1) % MAX_EVENTS;	
	}
	else
	{}
 	EA = ea_save; 
}

// 处理事件
void Event_Process(void) {
    Event e;   
    while(queue_head != queue_tail) {
        e = event_queue[queue_head];
        queue_head = (queue_head + 1) % MAX_EVENTS;
        
        // 处理 MOD 键：单击进入菜单模式
        if (e.type == EVENT_KEY_MODE) {
            SystemMode current_mode = Event_GetCurrentMode();
            if (current_mode == SYS_MODE_CLOCK) {
                // 时钟模式下按 MOD 键：进入菜单模式
                Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_MENU, current_mode);
            } else if (current_mode == SYS_MODE_MENU) {
                // 菜单模式下按 MOD 键：退出菜单，返回时钟模式
                Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, current_mode);
            } else {
                // 其他模式下按 MOD 键：默认返回时钟模式（保持原有行为）
                Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, current_mode);
            }
        }
        
        // 分发事件到各模块
		Number_Input_Manage(&e);   //数字输入管理
        Display_manage(&e);	   //显示模块
        Time_Manage(&e);        //走时管理
        Alarm_Manage(&e);		//闹钟管理
        
        Menu_Manage(&e);        // 菜单管理模块
		Stopwatch_Manage(&e); // 秒表管理
        
        // 模式切换后的初始化处理
        if (e.type == EVENT_SYS_MODE_CHANGE) {
            if (e.dat == SYS_MODE_NUMBER_INPUT) {
			    current_mode = e.dat;
                Number_Input_Reset();  // 进入数字输入模式时重置
            }
            if (e.dat == SYS_MODE_MENU) {
                Menu_ShowOptions();     // 进入菜单模式时显示菜单选项
            }
        }
    }
}

SystemMode Event_GetCurrentMode(void) {			//获取当前模式的辅助函数
    return current_mode;
}


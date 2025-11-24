#ifndef __EVENT_H__
#define __EVENT_H__

// 系统工作模式
typedef enum {
    SYS_MODE_CLOCK,        // 时钟模式
    SYS_MODE_STOPWATCH,    // 秒表模式
    SYS_MODE_ALARM_SET,    // 闹钟设置模式
    SYS_MODE_TIME_SET,      // 时间设置模式
	SYS_MODE_NUMBER_INPUT,  // 数字输入模式
	SYS_MODE_MENU		   //菜单模式
} SystemMode;

// 统一的事件类型定义
typedef enum {
    // 系统控制事件
    EVENT_NONE,                // 无事件
    EVENT_SYS_MODE_CHANGE,     // 系统模式切换
    EVENT_SYS_SETTING_CHANGE,  // 系统设置状态变化
    
    // 时间相关事件
    EVENT_TIME_TIMER_1S,       // 1秒定时事件
    EVENT_TIME_SECOND_UPDATED, // 秒已更新
    EVENT_TIME_MINUTE_UPDATED, // 分已更新
    EVENT_TIME_HOUR_UPDATED,   // 时已更新
    
    // 闹钟相关事件
    EVENT_ALARM_SECOND_UPDATED, // 闹钟秒更新
    EVENT_ALARM_MINUTE_UPDATED, // 闹钟分更新
    EVENT_ALARM_HOUR_UPDATED,   // 闹钟时更新
    EVENT_ALARM_TRIGGERED,      // 闹钟触发
	EVENT_ALARM_START,			//闹钟响铃
	EVENT_ALARM_STOP,			//闹钟关闭
	EVENT_ALARM_SNOOZE,		    //贪睡

    
    // 键盘输入事件
    EVENT_KEY_NUMBER,          // 数字键0-9
    EVENT_KEY_MODE,            // 模式键
    EVENT_KEY_SET,             // 设置键
    EVENT_KEY_INCREMENT,       // 增加键
    EVENT_KEY_DECREMENT,       // 减少键
    EVENT_KEY_ENTER,           // 确认键
    EVENT_KEY_CANCEL,          // 取消键
    
    // 显示相关事件
    EVENT_DISPLAY_UPDATE,      // 显示内容更新
    EVENT_DISPLAY_BLINK,        // 显示闪烁控制
	EVENT_DISPLAY_FLSAH,			//全闪烁

	//菜单相关事件
	EVENT_MENU_SELECT,     // 菜单选择事件
    EVENT_MENU_EXIT,        // 菜单退出事件
	//秒表相关事件
	EVENT_STOPWATCH_START,     // 秒表开始
    EVENT_STOPWATCH_PAUSE,     // 秒表暂停  
    EVENT_STOPWATCH_RESET,     // 秒表重置
    EVENT_STOPWATCH_UPDATE,     // 秒表时间更新（由定时器中断产生）
	EVENT_STOPWATCH_TOGGLE,  // 秒表开始/暂停切换
} EventType;

// 事件结构
typedef struct {
    EventType type;
    int dat;  // 事件数据，因为data在C51中是关键字
    SystemMode mode;
} Event;

// 函数声明
void Event_Publish(EventType type, int dat, SystemMode mode)reentrant;	 //reetrant关键字保证Event_Publish这个关键函数具有重入性
void Event_Process(void);
SystemMode Event_GetCurrentMode(void);

#endif
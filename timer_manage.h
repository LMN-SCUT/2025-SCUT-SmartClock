#ifndef __TIMER_MANAGE_H__
#define __TIMER_MANAGE_H__

#include "event.h"

// 时间设置子状态（在SYS_MODE_TIME_SET系统模式下使用）
typedef enum {
    TIME_SUBMODE_NORMAL,      // 正常显示（非设置模式）
    TIME_SUBMODE_SELECT_UNIT, // 选择调整单位
    TIME_SUBMODE_ADJUST_VALUE // 调整数值
} TimeSubMode;

// 时间单位
typedef enum {
    UNIT_HOUR,
    UNIT_MINUTE, 
    UNIT_SECOND
} TimeUnit;

// 函数声明
void Time_Manage(Event* e);
void Time_Blink_Update(void);
TimeSubMode Time_GetCurrentSubMode(void);
TimeUnit Time_GetSelectedUnit(void);
unsigned char Time_GetHour(void);
unsigned char Time_GetMin(void);
unsigned char Time_GetSec(void);

#endif
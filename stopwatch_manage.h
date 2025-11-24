#ifndef __STOPWATCH_MANAGE_H__
#define __STOPWATCH_MANAGE_H__

#include "event.h"

// 秒表状态机
typedef enum {
    STOPWATCH_IDLE,      // 空闲/重置状态
    STOPWATCH_RUNNING,   // 运行状态
    STOPWATCH_PAUSED     // 暂停状态
} StopwatchState;

// 函数声明
void Stopwatch_Manage(Event* e);
void Stopwatch_Init(void);
unsigned long Stopwatch_GetTimeMs(void); // 获取当前毫秒数

#endif
#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <REGX52.H>

// 按键功能定义 - 与事件系统匹配
#define KEY_NONE      0xFF

// 功能键定义
#define KEY_MODE      16    // 模式切换
#define KEY_SET       17    // 设置确认
#define KEY_INC       18    // 增加
#define KEY_DEC       19    // 减少
#define KEY_ENTER     20    // 确认
#define KEY_CANCEL    21    // 取消

// 函数声明
unsigned char Key_Scan(void);
unsigned char Key_GetFunction(void);
void Key_ProcessEvents(void);

#endif
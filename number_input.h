#ifndef __NUMBER_INPUT_H__
#define __NUMBER_INPUT_H__

#include "event.h"

// 数字输入状态
typedef enum {
    INPUT_IDLE,      // 空闲状态
    INPUT_ACTIVE,    // 输入中
    INPUT_COMPLETE   // 输入完成
} InputState;

// 函数声明
void Number_Input_Init(void);
void Number_Input_Manage(Event* e);
void Number_Input_Reset(void);
unsigned char* Number_Input_GetBuffer(void);
unsigned char Number_Input_GetCount(void);

#endif
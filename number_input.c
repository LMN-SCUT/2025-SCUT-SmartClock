#include <REGX52.H>
#include "number_input.h"
#include "event.h"
#include "DS3231.h"

// 数字输入缓冲区（存储6位数字：HHMMSS）
static xdata unsigned char input_buffer[6] = {0};
static xdata unsigned char input_count = 0;
static xdata InputState input_state = INPUT_IDLE;

void Number_Input_Init(void) {
    Number_Input_Reset();
}

void Number_Input_Reset(void) {
    unsigned char i;  // 将变量声明提到函数开头
    
    input_count = 0;
    input_state = INPUT_IDLE;
    
    for(i = 0; i < 6; i++) {  // 在循环外声明i，这里直接使用
        input_buffer[i] = 0;
    }
}

unsigned char* Number_Input_GetBuffer(void) {
    return input_buffer;
}

unsigned char Number_Input_GetCount(void) {
    return input_count;
}

void Number_Input_Manage(Event* e) {
    if (Event_GetCurrentMode() != SYS_MODE_NUMBER_INPUT) {
        return;
    }
    
    switch(e->type) {
        case EVENT_KEY_NUMBER:
            if (input_state == INPUT_ACTIVE && input_count < 6) {
                input_buffer[input_count] = e->dat;
                input_count++;
                Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_NUMBER_INPUT);
                
                if (input_count >= 6) {
                    input_state = INPUT_COMPLETE;
                }
            }
            break;
            
        case EVENT_KEY_CANCEL:
            Number_Input_Reset();
            Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_NUMBER_INPUT);
            break;
            
        case EVENT_KEY_ENTER:
            if (input_state == INPUT_COMPLETE) {
                // 解析输入的数字：前两位小时，中间两位分钟，后两位秒
                unsigned char hours = input_buffer[0] * 10 + input_buffer[1];
                unsigned char minutes = input_buffer[2] * 10 + input_buffer[3];
                unsigned char seconds = input_buffer[4] * 10 + input_buffer[5];
                
                // 验证时间有效性
                if (hours < 24 && minutes < 60 && seconds < 60) {
					 DS3231_SetTime(hours, minutes, seconds);//把时间写入DS3231
                    // 发布时间更新事件
                    Event_Publish(EVENT_TIME_HOUR_UPDATED, hours, SYS_MODE_CLOCK);
                    Event_Publish(EVENT_TIME_MINUTE_UPDATED, minutes, SYS_MODE_CLOCK);
                    Event_Publish(EVENT_TIME_SECOND_UPDATED, seconds, SYS_MODE_CLOCK);
                    
                    // 返回时钟模式
                    Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, SYS_MODE_NUMBER_INPUT);
                } else {
                    // 时间无效，显示错误
                    Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_NUMBER_INPUT);
                }
            }
            Number_Input_Reset();
            break;
    }
}
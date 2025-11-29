#ifndef __ALARM_MANAGE_H__
#define __ALARM_MANAGE_H__

#include "event.h"
#include "timer_manage.h"



// EEPROM 存储地址分配
#define ADDR_ALARM_HOUR   0x01
#define ADDR_ALARM_MIN    0x02
#define ADDR_ALARM_SEC    0x03
#define ADDR_ALARM_STATE  0x04 // 存闹钟开关状态 (0:关, 1:开)
#define MEM_MAGIC_ADDR    0x00 // 魔术字地址，用于判断是不是第一次使用
#define MEM_MAGIC_VAL     0x55 // 魔术字


// 闹钟状态
typedef enum {
    ALARM_OFF,      // 闹钟关闭
    ALARM_SET,       // 闹钟已设置
    ALARM_RINGING    // 闹钟响铃中
} AlarmState;

// 函数声明
void Alarm_Manage(Event* e);
void Alarm_Blink_Update(void); 
AlarmState Alarm_GetState(void);
unsigned char Alarm_GetHour(void);
unsigned char Alarm_GetMin(void);
unsigned char Alarm_GetSec(void);
void Alarm_Read_From_EEPROM(void);
bit Alarm_IsRinging(void);
TimeUnit Alarm_GetSelectedUnit(void);

#endif
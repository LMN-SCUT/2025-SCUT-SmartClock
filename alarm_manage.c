#include <REGX52.H>
#include "alarm_manage.h"
#include "event.h"
#include "display_manage.h"
#include "buzzer.h"  
#include "AT24C32.h" 
#include "timer_manage.h"

sbit ALARM_LED = P2^4; //引脚定义

// 闹钟时间存储（使用XDATA节省内存）
static xdata unsigned char alarm_hours = 7;    // 默认闹钟时间7:00
static unsigned char last_check_sec = 61;
static xdata unsigned char alarm_minutes = 0;
static xdata unsigned char alarm_seconds = 0;
static xdata AlarmState alarm_state = ALARM_OFF;
static xdata unsigned char alarm_blink_counter = 0;
static idata bit alarm_blink_state = 0;
static bit triggered_flag = 0;

// 闹钟设置状态（与时间设置类似）
static xdata TimeSubMode alarm_set_mode = TIME_SUBMODE_NORMAL;
static xdata TimeUnit alarm_selected_unit = UNIT_HOUR;

// 响铃控制变量
static unsigned int flash_counter = 0;
static unsigned char flash_state = 0;

// 获取函数
unsigned char Alarm_GetHour(void) { return alarm_hours; }
unsigned char Alarm_GetMin(void) { return alarm_minutes; }
unsigned char Alarm_GetSec(void) { return alarm_seconds; }

// 调整闹钟时间单位
static void Adjust_Alarm_Unit(int delta) {
    switch(alarm_selected_unit) {
        case UNIT_HOUR:
            alarm_hours = (alarm_hours + delta + 24) % 24;
            Event_Publish(EVENT_ALARM_HOUR_UPDATED, alarm_hours, SYS_MODE_ALARM_SET);
            break;
        case UNIT_MINUTE:
            alarm_minutes = (alarm_minutes + delta + 60) % 60;
            Event_Publish(EVENT_ALARM_MINUTE_UPDATED, alarm_minutes, SYS_MODE_ALARM_SET);
            break;
        case UNIT_SECOND:
            alarm_seconds = (alarm_seconds + delta + 60) % 60;
            Event_Publish(EVENT_ALARM_SECOND_UPDATED, alarm_seconds, SYS_MODE_ALARM_SET);
            break;
    }
}

// 检查闹钟是否触发
static void Check_Alarm_Trigger(unsigned char current_h, unsigned char current_m, unsigned char current_s) {
    
    // 如果秒变了，重置标志位
    if (current_s != last_check_sec) {
        last_check_sec = current_s;
        triggered_flag = 0;
    }

    if (alarm_state == ALARM_SET && 
        alarm_hours == current_h && 
        alarm_minutes == current_m && 
        alarm_seconds == current_s) {
        
        // 【关键】如果这秒已经触过发了，就别再发了
        if (triggered_flag == 0) {
            triggered_flag = 1; // 标记已触发
            
            // 发送触发事件
            Event_Publish(EVENT_ALARM_TRIGGERED, 0, SYS_MODE_CLOCK);
        }
    }
}

// 响铃管理
static void Start_Alarm_Ring(void) {
	alarm_state = ALARM_RINGING; //开闹钟
    Buzzer_SetMode(BUZZER_ALARM);  // 设置急促报警声
     ALARM_LED = 0;
    Event_Publish(EVENT_ALARM_START, 0, SYS_MODE_CLOCK);

}

static void Stop_Alarm_Ring(void) {

    alarm_state = ALARM_OFF;  // 关闭闹钟
    Buzzer_SetMode(BUZZER_OFF);    // 关闭蜂鸣器
	 ALARM_LED = 1;//关灯
    Event_Publish(EVENT_ALARM_STOP, 0, SYS_MODE_CLOCK);

}

// 闹钟管理主函数
// alarm_manage.c -> Alarm_Manage

void Alarm_Manage(Event* e) {
    SystemMode current_sys_mode = Event_GetCurrentMode();
    
    // ============================================================
    // 第一部分：全局任务（无论什么模式都要处理！）
    // ============================================================

    // 1. 每秒检查一次：时间到了没？
    if (e->type == EVENT_TIME_SECOND_UPDATED) {
        Check_Alarm_Trigger(Time_GetHour(), Time_GetMin(), Time_GetSec());
    }
    
    // 2. 收到触发通知：立刻开闹钟！
    // (这行代码之前被那个 return 挡住了，现在放出来)
    if (e->type == EVENT_ALARM_TRIGGERED) {
        Start_Alarm_Ring(); 
    }
    
    // 3. 闹钟响的时候：监听停止按键
    // (即使在主界面看时间，按键也能关闹钟)
    if (Alarm_IsRinging()) {
        switch(e->type) {
            case EVENT_KEY_ENTER:  
            case EVENT_KEY_SET:   
            case EVENT_KEY_CANCEL: 
                Stop_Alarm_Ring();
                break;
        }
    }

    // ============================================================
    // 第二部分：模式切换初始化
    // ============================================================
    
    // 如果刚切进“闹钟设置模式”，初始化一下状态
    if (e->type == EVENT_SYS_MODE_CHANGE) {
        if (e->dat == SYS_MODE_ALARM_SET) {
            alarm_set_mode = TIME_SUBMODE_SELECT_UNIT; 
            alarm_selected_unit = UNIT_HOUR;
            Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_ALARM_SET);
        }
        return; // 处理完切换就走
    }

    // ============================================================
    // 第三部分：拦截网
    // ============================================================
    
    // 【关键】如果当前不是闹钟设置模式，后面的设置逻辑就不跑了！
    // (之前的 Bug 就是这行放太前面了，把触发逻辑也拦截了)
    if (Event_GetCurrentMode() != SYS_MODE_ALARM_SET) return;

    // ============================================================
    // 第四部分：设置逻辑（仅在设置模式下运行）
    // ============================================================
    
    switch(alarm_set_mode) {
        case TIME_SUBMODE_SELECT_UNIT:
            // 选择单位模式
            switch(e->type) {
                case EVENT_KEY_INCREMENT:
                    alarm_selected_unit = (alarm_selected_unit + 1) % 3;
                    Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_ALARM_SET);
                    break;
                case EVENT_KEY_DECREMENT:
                    alarm_selected_unit = (alarm_selected_unit == 0) ? 2 : (alarm_selected_unit - 1);
                    Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_ALARM_SET);
                    break;
                case EVENT_KEY_ENTER:
                    alarm_set_mode = TIME_SUBMODE_ADJUST_VALUE;
                    Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_ALARM_SET);
                    break;
                case EVENT_KEY_SET:
                    // 保存并退出
                    alarm_set_mode = TIME_SUBMODE_NORMAL;
                    alarm_state = ALARM_SET;  // 启用闹钟
                    
                    // 保存到 EEPROM
                    AT24C32_WriteByte(ADDR_ALARM_HOUR, alarm_hours);
                    AT24C32_WriteByte(ADDR_ALARM_MIN, alarm_minutes);
                    AT24C32_WriteByte(ADDR_ALARM_SEC, alarm_seconds);
                    AT24C32_WriteByte(ADDR_ALARM_STATE, 1); 
                    AT24C32_WriteByte(MEM_MAGIC_ADDR, MEM_MAGIC_VAL);
                    
                    Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, SYS_MODE_ALARM_SET);
                    break;
            }
            break;
            
        case TIME_SUBMODE_ADJUST_VALUE:
            // 调整数值模式
            switch(e->type) {
                case EVENT_KEY_INCREMENT:
                    Adjust_Alarm_Unit(1);
                    break;
                case EVENT_KEY_DECREMENT:
                    Adjust_Alarm_Unit(-1);
                    break;
                case EVENT_KEY_ENTER:
                    alarm_set_mode = TIME_SUBMODE_SELECT_UNIT;
                    Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_ALARM_SET);
                    break;
                case EVENT_KEY_SET:
                    // 确认并退出
                    alarm_set_mode = TIME_SUBMODE_NORMAL;
                    alarm_state = ALARM_SET;
                    
                    // 这里也要保存 EEPROM (建议加上，防止用户在数值调整界面直接按SET退出)
                    AT24C32_WriteByte(ADDR_ALARM_HOUR, alarm_hours);
                    AT24C32_WriteByte(ADDR_ALARM_MIN, alarm_minutes);
                    AT24C32_WriteByte(ADDR_ALARM_SEC, alarm_seconds);
                    AT24C32_WriteByte(ADDR_ALARM_STATE, 1);
                    
                    Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, SYS_MODE_ALARM_SET);
                    break;
            }
            break;
    }
}


void Alarm_Read_From_EEPROM(void) {
    // 检查是否是第一次运行（检查魔术字）
    unsigned char magic = AT24C32_ReadByte(MEM_MAGIC_ADDR);
    
    if(magic == MEM_MAGIC_VAL) {
        // 如果有魔术字，说明之前保存过，读取出来
        alarm_hours   = AT24C32_ReadByte(ADDR_ALARM_HOUR);
        alarm_minutes = AT24C32_ReadByte(ADDR_ALARM_MIN);
        alarm_seconds = AT24C32_ReadByte(ADDR_ALARM_SEC);
        
        // 恢复闹钟开关状态
        if(AT24C32_ReadByte(ADDR_ALARM_STATE) == 1) {
            alarm_state = ALARM_SET;
        } else {
            alarm_state = ALARM_OFF;
        }
    } else {
        // 第一次运行，保持默认值 (7:00:00)
        alarm_hours = 7;
        alarm_minutes = 0;
        alarm_seconds = 0;
        alarm_state = ALARM_OFF;

		AT24C32_WriteByte(ADDR_ALARM_HOUR, alarm_hours);
        AT24C32_WriteByte(ADDR_ALARM_MIN, alarm_minutes);
        AT24C32_WriteByte(ADDR_ALARM_SEC, alarm_seconds);
        AT24C32_WriteByte(ADDR_ALARM_STATE, 0); // 默认关闭
        AT24C32_WriteByte(MEM_MAGIC_ADDR, MEM_MAGIC_VAL); // 写入魔术字标记
    }
}

bit Alarm_IsRinging(void) {
    // 【关键修改】直接问状态机：现在是不是在响铃状态？
    if (alarm_state == ALARM_RINGING) {
        return 1;
    }
    return 0;
}
// 闹钟设置模式下的数字闪烁逻辑
void Alarm_Blink_Update(void) {
alarm_blink_state = !alarm_blink_state;
// 发布闪烁事件，通知 Display 模块刷新屏幕
Event_Publish(EVENT_DISPLAY_BLINK, alarm_blink_state, SYS_MODE_ALARM_SET);
        }
TimeUnit Alarm_GetSelectedUnit(void) { 
return alarm_selected_unit; }

AlarmState Get_Alarm_State(void) {	  //测试代码
    return alarm_state;
}



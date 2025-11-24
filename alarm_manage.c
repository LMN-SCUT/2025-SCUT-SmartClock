#include <REGX52.H>
#include "alarm_manage.h"
#include "event.h"
#include "display_manage.h"
#include "buzzer.h"  
#include "AT24C32.h" 



// 闹钟时间存储（使用XDATA节省内存）
static xdata unsigned char alarm_hours = 7;    // 默认闹钟时间7:00
static xdata unsigned char alarm_minutes = 0;
static xdata unsigned char alarm_seconds = 0;

static xdata AlarmState alarm_state = ALARM_OFF;
static xdata unsigned char alarm_blink_counter = 0;
static  bit alarm_blink_state = 0;

// 闹钟设置状态（与时间设置类似）
static xdata TimeSubMode alarm_set_mode = TIME_SUBMODE_NORMAL;
static xdata TimeUnit alarm_selected_unit = UNIT_HOUR;

// 响铃控制变量
static bit alarm_ringing = 0;
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
    if (alarm_state == ALARM_SET && 
        alarm_hours == current_h && 
        alarm_minutes == current_m && 
        alarm_seconds == current_s) {
        alarm_state = ALARM_RINGING;
        alarm_ringing = 1;  // 设置响铃标志
        Event_Publish(EVENT_ALARM_TRIGGERED, 0, SYS_MODE_CLOCK);
    }
}

// 响铃管理
static void Start_Alarm_Ring(void) {
    alarm_ringing = 1;
    Buzzer_SetMode(BUZZER_ALARM);  // 设置急促报警声
    Event_Publish(EVENT_ALARM_START, 0, SYS_MODE_CLOCK);
}

static void Stop_Alarm_Ring(void) {
    alarm_ringing = 0;
    alarm_state = ALARM_OFF;  // 关闭闹钟
    Buzzer_SetMode(BUZZER_OFF);    // 关闭蜂鸣器
    Event_Publish(EVENT_ALARM_STOP, 0, SYS_MODE_CLOCK);
}

// 闹钟管理主函数
void Alarm_Manage(Event* e) {
    SystemMode current_sys_mode = Event_GetCurrentMode();
    
    // 检查闹钟触发（在所有模式下都需要）
    if (e->type == EVENT_TIME_SECOND_UPDATED) {
        Check_Alarm_Trigger(Time_GetHour(), Time_GetMin(), Time_GetSec());
    }
    
    // 处理闹钟响铃时的按键事件
    if (alarm_ringing) {
        switch(e->type) {
            case EVENT_KEY_ENTER:  // ENT键停止闹钟
                Stop_Alarm_Ring();
                break;
                
            case EVENT_KEY_SET:   // SET键也可以停止闹钟（可选）
            case EVENT_KEY_CANCEL: // CANCEL键也可以停止闹钟（可选）
                Stop_Alarm_Ring();
                break;
        }
    }
    
    // 闹钟响铃时的显示闪烁控制
    if (alarm_ringing) {
        flash_counter++;
        if (flash_counter >= 5) {  // 每50ms切换一次闪烁状态
            flash_counter = 0;
            flash_state = !flash_state;
            Event_Publish(EVENT_DISPLAY_FLSAH, flash_state, SYS_MODE_CLOCK);
        }
    }
    
    // 只在闹钟设置模式下处理设置相关事件
    if (current_sys_mode == SYS_MODE_ALARM_SET) {
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
                        // 退出闹钟设置，保存设置
                        alarm_set_mode = TIME_SUBMODE_NORMAL;
                        alarm_state = ALARM_SET;  // 启用闹钟
						// 保存到 EEPROM
    					AT24C32_WriteByte(ADDR_ALARM_HOUR, alarm_hours);
    					AT24C32_WriteByte(ADDR_ALARM_MIN, alarm_minutes);
    					AT24C32_WriteByte(ADDR_ALARM_SEC, alarm_seconds);
    					AT24C32_WriteByte(ADDR_ALARM_STATE, 1); // 状态设为开启
    					AT24C32_WriteByte(MEM_MAGIC_ADDR, MEM_MAGIC_VAL); // 标记已初始化
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
                        Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, SYS_MODE_ALARM_SET);
                        break;
                }
                break;
        }
    }
    
    // 处理闹钟触发事件
    if (e->type == EVENT_ALARM_TRIGGERED) {
        Start_Alarm_Ring();  // 开始响铃
    }
}


//放在主循环调用
void Alarm_Loop_Update(void) {
    // 只有响铃时才需要高频闪烁
    if (alarm_ringing) {
        flash_counter++;
        if (flash_counter >= 50) { // 假设主循环10ms一次，这里500ms闪烁
            flash_counter = 0;
            flash_state = !flash_state;
            // 直接发布闪烁事件，让Display去处理显示
            Event_Publish(EVENT_DISPLAY_FLSAH, flash_state, SYS_MODE_CLOCK);
        }
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
    }
}
// 闹钟设置模式下的数字闪烁逻辑
void Alarm_Blink_Update(void) {
    // 只有在“闹钟设置模式”下才运行
    if (Event_GetCurrentMode() == SYS_MODE_ALARM_SET) {
        alarm_blink_counter++;
        // 假设主循环约10ms调用一次，这里计数50次 = 500ms
        if (alarm_blink_counter >= 50) {
            alarm_blink_counter = 0;
            alarm_blink_state = !alarm_blink_state;
            // 发布闪烁事件，通知 Display 模块刷新屏幕
            Event_Publish(EVENT_DISPLAY_BLINK, alarm_blink_state, SYS_MODE_ALARM_SET);
        }
    } else {
        // 退出模式时重置，防止下次进来时状态不对（可选）
        alarm_blink_counter = 0;
        alarm_blink_state = 0;
    }
}

#include <REGX52.H>
#include "LCD1602.h"
#include "event.h"
#include "timer_manage.h"
#include "alarm_manage.h"
#include "buzzer.h"
#include "number_input.h"
#include "menu_manage.h"
#include "stopwatch_manage.h"  
#include <stdio.h>

// 显示缓冲
static xdata char time_display[17] = "Time: 00:00:00";
static xdata char alarm_display[17] = "Alarm:00:00:00";

// 闹钟响铃状态
static bit alarm_ringing = 0;
static unsigned char flash_counter = 0;
static unsigned char flash_state = 0;

void Display_Init(void) {
    LCD_Init();
    LCD_ShowString(1, 1, time_display);
}

// 全屏闪烁函数（闹钟响铃时使用）
static void Display_Full_Flash(unsigned char state) {
    if(state) {
        LCD_ShowString(1, 1, "ALARM ACTIVATED ");
        LCD_ShowString(2, 1, "Press ENT to stop");
    } else {
        LCD_ShowString(1, 1, "                ");
        LCD_ShowString(2, 1, "                ");
    }
}

// 通用闪烁显示函数（用于设置模式）
static void Display_Blink_Control(TimeSubMode sub_mode, 
                                  TimeUnit selected, unsigned char blink_state,
                                  char* display_text) {
	   static xdata char blink_display[17];
    
    // 1. 如果是亮的状态，直接显示原文本
    if(blink_state) {
        LCD_ShowString(1, 1, display_text);
        return;
    } 
    
    // 2. 如果是灭的状态，我们需要把“选中”的部分挖空
 
    sprintf(blink_display, "%s", display_text);
    
    // 注意：这里的时间/闹钟格式必须是 "Time: HH:MM:SS  "
    // HH start at 6, MM start at 9, SS start at 12
    
    switch(selected) {
        case UNIT_HOUR:
            blink_display[6] = ' ';
            blink_display[7] = ' ';
            break;
            
        case UNIT_MINUTE:
            blink_display[9] = ' ';
            blink_display[10] = ' ';
            break;
            
        case UNIT_SECOND:
            blink_display[12] = ' ';
            blink_display[13] = ' ';
            break;
    }
    
    // 显示挖空后的字符串
    LCD_ShowString(1, 1, blink_display);
}

void Display_manage(Event* e) {
    SystemMode sys_mode = e->mode;
    unsigned char i;  // 循环变量声明在函数开头
    
    switch(e->type) {
        case EVENT_TIME_SECOND_UPDATED:
        case EVENT_TIME_MINUTE_UPDATED:
        case EVENT_TIME_HOUR_UPDATED:
            // 更新时间显示
            sprintf(time_display, "Time: %02d:%02d:%02d  ", 
                   (int)Time_GetHour(), (int)Time_GetMin(), (int)Time_GetSec());
            
            if(!alarm_ringing || sys_mode == SYS_MODE_TIME_SET) {
                if(sys_mode == SYS_MODE_CLOCK || sys_mode == SYS_MODE_TIME_SET) {
                    LCD_ShowString(1, 1, time_display);
                }
            }
            break;
            
        case EVENT_ALARM_SECOND_UPDATED:
        case EVENT_ALARM_MINUTE_UPDATED:
        case EVENT_ALARM_HOUR_UPDATED:
            sprintf(alarm_display, "Alarm:%02d:%02d:%02d  ", 
                 (int)Alarm_GetHour(), (int)Alarm_GetMin(), (int)Alarm_GetSec());
            if(sys_mode == SYS_MODE_ALARM_SET) {
                LCD_ShowString(1, 1, alarm_display);
            }
            break;
            
        case EVENT_DISPLAY_UPDATE:
            // 强制刷新显示
            if(sys_mode == SYS_MODE_CLOCK || sys_mode == SYS_MODE_TIME_SET) {
                LCD_ShowString(1, 1, time_display);
            } else if(sys_mode == SYS_MODE_ALARM_SET) {
                LCD_ShowString(1, 1, alarm_display);
            } else if(sys_mode == SYS_MODE_NUMBER_INPUT) {
                unsigned char* buffer = Number_Input_GetBuffer();
                unsigned char count = Number_Input_GetCount();
                
                char input_display[17] = "                ";
                for(i = 0; i < count; i++) {
                    input_display[i] = buffer[i] + '0';
                }
                LCD_ShowString(2, 1, input_display);
            } else if(sys_mode == SYS_MODE_STOPWATCH) {
                // 秒表模式显示处理
                unsigned long total_ms = Stopwatch_GetTimeMs();
                unsigned char minutes = total_ms / 60000;
                unsigned char seconds = (total_ms % 60000) / 1000;
                unsigned int milliseconds = (total_ms % 1000) / 10;
                
                char stopwatch_display[17];
                sprintf(stopwatch_display, "%02d:%02d.%02d       ", (int)minutes, (int)seconds, (int)milliseconds);
                LCD_ShowString(2, 1, stopwatch_display);
            }
            break;
            
        case EVENT_DISPLAY_BLINK:
            if(sys_mode == SYS_MODE_TIME_SET) {
                TimeSubMode sub_mode = Time_GetCurrentSubMode();
                TimeUnit selected = Time_GetSelectedUnit();
                Display_Blink_Control(sub_mode, selected, e->dat, time_display);
            } 
            else if(sys_mode == SYS_MODE_ALARM_SET) {
                TimeSubMode sub_mode = Time_GetCurrentSubMode();
                TimeUnit selected = Time_GetSelectedUnit();
                Display_Blink_Control(sub_mode, selected, e->dat, alarm_display);
            }
            break;
            
        case EVENT_ALARM_TRIGGERED:
            alarm_ringing = 1;
            Buzzer_SetMode(BUZZER_ALARM);
            LCD_ShowString(1, 1, "ALARM ACTIVATED ");
            LCD_ShowString(2, 1, "Press ENT to stop");
            break;
            
        case EVENT_KEY_ENTER:
            if(alarm_ringing) {
                alarm_ringing = 0;
                Buzzer_SetMode(BUZZER_OFF);
                
                sprintf(time_display, "Time: %02d:%02d:%02d", 
                       Time_GetHour(), Time_GetMin(), Time_GetSec());
                LCD_ShowString(1, 1, time_display);
                LCD_ShowString(2, 1, "Alarm Stopped  ");
                
                Event_Publish(EVENT_ALARM_STOP, 0, sys_mode);
            }
            break;
            
        case EVENT_SYS_MODE_CHANGE:
            if(alarm_ringing) {
                alarm_ringing = 0;
                Buzzer_SetMode(BUZZER_OFF);
            }
            
            switch(e->dat) {
                case SYS_MODE_CLOCK:
				 sprintf(time_display, "Time: %02d:%02d:%02d  ", 
                   (int)Time_GetHour(), (int)Time_GetMin(), (int)Time_GetSec());
                    LCD_ShowString(1, 1, time_display);
                    LCD_ShowString(2, 1, "                ");
                    break;
                    
                case SYS_MODE_TIME_SET:
                    LCD_ShowString(1, 1, time_display);
                    LCD_ShowString(2, 1, "Set Time       ");
                    break;
                    
                case SYS_MODE_ALARM_SET:
                    sprintf(alarm_display, "Alarm:%02d:%02d:%02d ", 
                         (int)Alarm_GetHour(), (int)Alarm_GetMin(), (int)Alarm_GetSec());
                    LCD_ShowString(1, 1, alarm_display);
                    LCD_ShowString(2, 1, "Set Alarm      ");
                    break;
                    
                case SYS_MODE_STOPWATCH:
                    LCD_ShowString(1, 1, "Stopwatch      ");
                    LCD_ShowString(2, 1, "00:00.00       ");
                    break;
                    
                case SYS_MODE_NUMBER_INPUT:
                    LCD_ShowString(1, 1, "Enter time:     ");
                    LCD_ShowString(2, 1, "000000          ");
                    break;
                    
                case SYS_MODE_MENU:
                    Menu_ShowOptions();
                    break;
            }
            break;
            
        default:
            break;
    }
    
    // 闹钟响铃时的闪烁处理
    if(alarm_ringing) {
        flash_counter++;
        if(flash_counter >= 3) {
            flash_counter = 0;
            flash_state = !flash_state;
            Display_Full_Flash(flash_state);
        }
    }
}
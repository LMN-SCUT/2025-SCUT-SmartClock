#include "keypad.h"
#include "event.h"
#include "delay.h" // 确保包含 Delay(10)
#include "BUZZER.h"

// 修正后的按键映射表
code const unsigned char key_function_map[16] = {
    '1', '2', '3', KEY_MODE,   
    '4', '5', '6', KEY_SET,    
    '7', '8', '9', KEY_ENTER,  
    '0', KEY_INC, KEY_DEC, KEY_CANCEL 
};

// 简单的阻塞式扫描函数
unsigned char Key_Scan(void)
{
    unsigned char row, col;
    unsigned char key_value = 0xFF;
    
    P1 = 0x0F; // 准备读取
    if ((P1 & 0x0F) != 0x0F) {
        Delay(10); // 阻塞消抖
        if ((P1 & 0x0F) != 0x0F) {
            // 扫描
            for (row = 0; row < 4; row++) {
                P1 = ((~(1 << row)) << 4) | 0x0F;
                if ((P1 & 0x0F) != 0x0F) {
                    for (col = 0; col < 4; col++) {
                        if ((P1 & (1 << col)) == 0) {
                            key_value = row * 4 + col;
                            
                            // 【关键】等待按键释放（防止连发）
                            while((P1 & 0x0F) != 0x0F); 
                            
                            return key_value;
                        }
                    }
                }
            }
        }
    }
    return 0xFF;
}

// 事件处理函数
void Key_ProcessEvents(void)
{
    unsigned char raw_key = Key_Scan(); // 这里会阻塞直到松手
    
    if (raw_key != 0xFF) {
        unsigned char key_func = key_function_map[raw_key];
        SystemMode current_mode = Event_GetCurrentMode();
        
        // 分发事件逻辑 (和之前一样)
        if (key_func >= '0' && key_func <= '9') {
	       
            if (current_mode == SYS_MODE_MENU) {
                Event_Publish(EVENT_MENU_SELECT, key_func - '0', current_mode);
            } else {
                Event_Publish(EVENT_KEY_NUMBER, key_func - '0', current_mode);
            }
        } else {
            // ... (复制你原来的 switch-case 逻辑) ...
            switch (key_func) {
                case KEY_MODE:
                    if (current_mode == SYS_MODE_CLOCK) Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_MENU, current_mode);
                    else if (current_mode == SYS_MODE_MENU) Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, current_mode);
                    else Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, current_mode);
                    break;
                case KEY_SET:
                    if (current_mode == SYS_MODE_CLOCK) Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_NUMBER_INPUT, current_mode);
                    else Event_Publish(EVENT_KEY_SET, 0, current_mode);
                    break;
                // ... 其他按键 ...
                case KEY_INC: Event_Publish(EVENT_KEY_INCREMENT, 0, current_mode); break;
                case KEY_DEC: Event_Publish(EVENT_KEY_DECREMENT, 0, current_mode); break;
                case KEY_ENTER: 
                    if(current_mode == SYS_MODE_STOPWATCH) Event_Publish(EVENT_STOPWATCH_TOGGLE, 0, current_mode);
                    else Event_Publish(EVENT_KEY_ENTER, 0, current_mode); 
                    break;
                case KEY_CANCEL:
                    if(current_mode == SYS_MODE_STOPWATCH) {
                        Event_Publish(EVENT_STOPWATCH_RESET, 0, current_mode);
                        Event_Publish(EVENT_DISPLAY_UPDATE, 0, current_mode);
                    } else {
                        Event_Publish(EVENT_KEY_CANCEL, 0, current_mode);
                    }
                    break;
            }
        }
    }
}
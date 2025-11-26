#include "keypad.h"
#include "event.h"
#include "delay.h"

static bit key_pressed_flag = 0;

// 键值映射表
code const unsigned char key_function_map[16] = {
    '7', '8', '9', KEY_MODE,  // 第一行: 极速模式7, 8, 9, 模式
    '4', '5', '6', KEY_SET,   // 第二行: 4, 5, 6, 设置
    '1', '2', '3', KEY_ENTER, // 第三行: 1, 2, 3, 确认
    '0', KEY_INC, KEY_DEC, KEY_CANCEL // 第四行: 0, 增加, 减少, 取消
};

// 键盘扫描函数（保持不变）
unsigned char Key_Scan(void)
{
    unsigned char row, col;
    unsigned char key_value = 0xFF;
    unsigned char col_state;
    
    // 设置行线输出低，列线输入带上拉
    P1 = 0x0F;
    
    // 检查是否有按键按下
    if ((P1 & 0x0F) != 0x0F) {
        Delay(10); // 消抖
        
        if ((P1 & 0x0F) != 0x0F) {

		if(key_pressed_flag == 1)
		{ return 0xFF; 	}//避免长按卡死
            // 逐行扫描
            for (row = 0; row < 4; row++) {
                P1 = ((~(1 << row)) << 4) | 0x0F;
                
                col_state = P1 & 0x0F;
                if (col_state != 0x0F) {
                    for (col = 0; col < 4; col++) {
                        if (!(col_state & (1 << col))) {
                            key_value = row * 4 + col;
                            
                            // 等待按键释放
                               key_pressed_flag = 1;
                            return key_value;
                        }
                    }
                }
            }
        }
    }
     else {
        //检测到没有按键按下，重置标志位，允许下一次触发
        key_pressed_flag = 0;
    }
    return 0xFF;
}

// 获取按键功能（保持不变）
unsigned char Key_GetFunction(void)
{
    unsigned char raw_key = Key_Scan();
    if (raw_key == 0xFF) {
        return KEY_NONE;
    }
    return key_function_map[raw_key];
}

// 处理键盘事件
void Key_ProcessEvents(void)
{
    unsigned char key_func = Key_GetFunction();
    SystemMode current_mode = Event_GetCurrentMode();
    
    if (key_func != KEY_NONE) {
        if (key_func >= '0' && key_func <= '9') {
            // 数字键事件：根据当前模式决定行为
            if (current_mode == SYS_MODE_MENU) {
                // 菜单模式下：数字极速模式键用于选择功能
                Event_Publish(EVENT_MENU_SELECT, key_func - '0', current_mode);
            } else {
                // 其他模式下：保持原有功能（如数字输入模式）
                Event_Publish(EVENT_KEY_NUMBER, key_func - '0', current_mode);
            }
        }
        else {
            // 功能键事件
            switch (key_func) {
                case KEY_MODE:
                    // MOD键：根据当前模式智能切换
                    if (current_mode == SYS_MODE_CLOCK) {
                        // 时钟模式 → 菜单模式
                        Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_MENU, current_mode);
                    }
                    else if (current_mode == SYS_MODE_MENU) {
                        // 菜单模式 → 时钟模式（退出菜单）
                        Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, current_mode);
                    }
                    else {
                        // 其他模式 → 时钟模式（安全退出）
                        Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_CLOCK, current_mode);
                    }
                    break;
                    
                case KEY_SET:
                    // SET键：在时钟模式下进入数字输入模式
                    if (current_mode == SYS_MODE_CLOCK) {
                        Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_NUMBER_INPUT, current_mode);
                    } else {
                        // 其他模式下保持原有功能
                        Event_Publish(EVENT_KEY_SET, 0, current_mode);
                    }
                    break;
                    
                case KEY_INC:
                    Event_Publish(EVENT_KEY_INCREMENT, 0, current_mode);
                    break;
                    
                case KEY_DEC:
                    Event_Publish(EVENT_KEY_DECREMENT, 0, current_mode);
                    break;
                    
                case KEY_ENTER:
                    if(current_mode == SYS_MODE_STOPWATCH) {
                        // 秒表模式下：发布切换事件（开始/暂停），由秒表模块处理状态转换
                        Event_Publish(EVENT_STOPWATCH_TOGGLE, 0, current_mode);
                    } else {
                        // 其他模式下保持原有功能
                        Event_Publish(EVENT_KEY_ENTER, 0, current_mode);
                    }
                    break;
                    
                case KEY_CANCEL:
                    if(current_mode == SYS_MODE_STOPWATCH) {
                        // 秒表模式下：发布重置事件
                        Event_Publish(EVENT_STOPWATCH_RESET, 0, current_mode);
                        Event_Publish(EVENT_DISPLAY_UPDATE, 0, current_mode); // 触发显示更新
                    } else {
                        // 其他模式下保持原有功能
                        Event_Publish(EVENT_KEY_CANCEL, 0, current_mode);
                    }
                    break;
            }
        }
    }
}
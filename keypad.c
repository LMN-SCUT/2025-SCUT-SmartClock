#include "keypad.h"
#include "event.h"
#include "timer.h" 

// 键值映射表
code const unsigned char key_function_map[16] = {
    '1',          // [0] 对应 SW16 (左上角)
    '2',          // [1] 对应 SW15
    '3',          // [2] 对应 SW14
    KEY_MODE,     // [3] 对应 SW13 (右上角) -> 模式切换
    
    // --- 第二行 ---
    '4',          // [4] 对应 SW12
    '5',          // [5] 对应 SW11
    '6',          // [6] 对应 SW10
    KEY_SET,      // [7] 对应 SW09 -> 设置
    
    // --- 第三行 ---
    '7',          // [8] 对应 SW08
    '8',          // [9] 对应 SW07
    '9',          // [10] 对应 SW06
    KEY_ENTER,    // [11] 对应 SW05 -> 确认/开始
    
    // --- 第四行 ---
    '0',          // [12] 对应 SW04 (左下角)
    KEY_INC,      // [13] 对应 SW03 -> 加
    KEY_DEC,      // [14] 对应 SW02 -> 减
    KEY_CANCEL    // [15] 对应 SW01 (右下角) -> 取消/停止
};

// 状态机变量
static unsigned char key_pressed_flag = 0; // 0=松开, 1=按下
static unsigned long last_key_time = 0;    // 上次检测到有按键的时间
static unsigned char last_key_raw = 0xFF;  // 上次扫描到的原始键值

// 辅助函数：快速扫描一次当前是否有键按下，返回原始键值(0~15)，无键返回0xFF
// (这个函数只负责“看一眼”，不消抖，不延时)
static unsigned char Key_Scan_Raw(void) {
    unsigned char row, col;
    unsigned char col_state;
    
    P1 = 0x0F; // 准备读取
    if ((P1 & 0x0F) == 0x0F) return 0xFF; // 快速判断：如果全是高电平，说明没键按下
    
    // 逐行扫描
    for (row = 0; row < 4; row++) {
        P1 = ((~(1 << row)) << 4) | 0x0F;
        col_state = P1 & 0x0F;
        if (col_state != 0x0F) {
            for (col = 0; col < 4; col++) {
                if (!(col_state & (1 << col))) {
                    return (row * 4 + col); // 找到按键，直接返回
                }
            }
        }
    }
    return 0xFF;
}

// 主处理函数：非阻塞消抖 + 状态机
void Key_ProcessEvents(void) {
    unsigned char current_key_raw = Key_Scan_Raw();
    unsigned long current_time = Timer_GetMilliseconds();
    
    // 状态机逻辑
    if (current_key_raw != 0xFF) {
        // [有键按下]
        
        if (key_pressed_flag == 0) {
            // 之前是松开的，现在刚按下 -> 开始消抖计时
            if (last_key_raw != current_key_raw) {
                // 如果按键变了（或者刚开始按），重置计时起点
                last_key_time = current_time;
                last_key_raw = current_key_raw;
            } 
            else {
                // 按键没变，检查时间差
                if (current_time - last_key_time >= 20) { // 消抖 20ms
                    // 消抖通过！确认为有效按下！
                    key_pressed_flag = 1; 
                    
                    // --- 触发事件 ---
                    {
                        unsigned char key_func = key_function_map[current_key_raw];
                        SystemMode current_mode = Event_GetCurrentMode();
                        
                       
                        if (key_func >= '0' && key_func <= '9') {
                            if (current_mode == SYS_MODE_MENU) {
                                Event_Publish(EVENT_MENU_SELECT, key_func - '0', current_mode);
                            } else {
                                Event_Publish(EVENT_KEY_NUMBER, key_func - '0', current_mode);
                            }
                        } else {
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
                    // ----------------
                }
            }
        }
        else {
            // key_pressed_flag == 1 (已经处于按下状态)
            last_key_time = current_time; // 持续更新时间，防止松开瞬间误判
        }
        
    } else {
        // [无键按下]
        if (key_pressed_flag == 1) {

            key_pressed_flag = 0; // 标记为松开，准备下一次触发
            last_key_raw = 0xFF;
        }
        else {
            last_key_raw = 0xFF; // 保持松开状态
        }
    }
}
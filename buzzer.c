#include <REGX52.H>
#include "buzzer.h"
#include "delay.h"

// 蜂鸣器状态变量
static BuzzerMode current_mode = BUZZER_OFF;
static unsigned int beep_counter = 0;
static unsigned char beep_state = 0;

// 蜂鸣器引脚定义 - 根据原理图，BZ连接到P3.7


// 蜂鸣器初始化
void Buzzer_Init(void) {
    BUZZER = 1;  // 初始状态关闭
    current_mode = BUZZER_OFF;
}

// 设置蜂鸣器模式
void Buzzer_SetMode(BuzzerMode mode) {
    current_mode = mode;
    beep_counter = 0;
    beep_state = 0;
}

// 蜂鸣器状态更新（需要在主循环中定期调用，如每10ms）
void Buzzer_Update(void) {
    switch(current_mode) {
        case BUZZER_OFF:
            BUZZER = 1;
            break;
            
        case BUZZER_ON:
            BUZZER = 0;  // 持续鸣叫
            break;
            
        case BUZZER_BEEP:
            // 产生"滴-滴-滴"声音（500ms周期）
            beep_counter++;
            if(beep_counter >= 50) {  // 50 * 10ms = 500ms
                beep_counter = 0;
                beep_state = !beep_state;
                BUZZER = !beep_state;  // 切换状态
            }
            break;
            
        case BUZZER_ALARM:
            // 急促报警声（200ms周期）
            beep_counter++;
            if(beep_counter >= 20) {  // 20 * 10ms = 200ms
                beep_counter = 0;
                beep_state = !beep_state;
                BUZZER = !beep_state;
            }
            break;
    }
}
void Buzzer_Test(void){
BUZZER=!BUZZER; 
Delay(1000);
BUZZER=!BUZZER;  //测试代码，记得删
			}
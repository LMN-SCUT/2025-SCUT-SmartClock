#ifndef __BUZZER_H__
#define __BUZZER_H__

// 蜂鸣器引脚定义 - 根据原理图，BZ连接到P3.7
sbit BUZZER = P3^7;

// 蜂鸣器工作模式
typedef enum {
    BUZZER_OFF,      // 关闭
    BUZZER_ON,       // 常鸣
    BUZZER_BEEP,     // 滴滴声
    BUZZER_ALARM     // 报警模式（急促鸣叫）
} BuzzerMode;

// 函数声明
void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_SetMode(BuzzerMode mode);
void Buzzer_Update(void);  // 需要在主循环中定期调用

#endif
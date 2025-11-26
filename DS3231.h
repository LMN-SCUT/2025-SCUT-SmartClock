#ifndef __DS3231_H__
#define __DS3231_H__

// 函数声明
void DS3231_Init(void);
bit DS3231_ReadTime(void);
void DS3231_SetTime(unsigned char h, unsigned char m, unsigned char s);

// 这些变量存储从DS3231读回来的最新时间
// 既然是全局调用，我们用 extern 暴露出去
extern unsigned char ds_hour;
extern unsigned char ds_min;
extern unsigned char ds_sec;

#endif
#ifndef __I2C_HAL_H__
#define __I2C_HAL_H__

#include <REGX52.H>

// ==========================================
// [硬件定义] I2C 引脚分配
// SCL 接 P3.4
// SDA 接 P3.5
// 注意：DS3231 和 AT24C02 共用这两根线！
// ==========================================
sbit I2C_SCL = P3^4;
sbit I2C_SDA = P3^5;

// 函数声明
void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_SendByte(unsigned char Byte);
unsigned char I2C_ReceiveByte(void);
void I2C_SendAck(bit AckBit);
bit I2C_ReceiveAck(void);

#endif
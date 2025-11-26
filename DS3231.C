#include <REGX52.H>
#include "ds3231.h"
#include "i2c_hal.h"

#define DS3231_ADDRESS_WRITE 0xD0
#define DS3231_ADDRESS_READ  0xD1

// 全局变量定义
unsigned char ds_hour = 12;
unsigned char ds_min = 0;
unsigned char ds_sec = 0;

// BCD转十进制 (例如: 0x12 -> 12)
unsigned char BCD_To_Decimal(unsigned char bcd) {
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

// 十进制转BCD (例如: 12 -> 0x12)
unsigned char Decimal_To_BCD(unsigned char dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// 初始化 DS3231 (主要是检查一下通信，或者初始化控制寄存器)
void DS3231_Init(void) {
    // 这里暂时留空，DS3231上电默认就能工作
    // 如果需要输出1Hz方波，可以在这里配置寄存器 0x0E
    I2C_Init(); // 确保I2C总线被初始化
}

// 设置 DS3231 时间
void DS3231_SetTime(unsigned char h, unsigned char m, unsigned char s) {
    I2C_Start();
    I2C_SendByte(DS3231_ADDRESS_WRITE);
    if (I2C_ReceiveAck()) { I2C_Stop(); return; } // 错误处理：无应答
    
    I2C_SendByte(0x00); // 设置写指针从 0x00 (秒寄存器) 开始
    I2C_ReceiveAck();
    
    I2C_SendByte(Decimal_To_BCD(s)); // 写入秒
    I2C_ReceiveAck();
    
    I2C_SendByte(Decimal_To_BCD(m)); // 写入分
    I2C_ReceiveAck();
    
    I2C_SendByte(Decimal_To_BCD(h)); // 写入时
    I2C_ReceiveAck();
    
    I2C_Stop();
}

// 读取 DS3231 时间
void DS3231_ReadTime(void) {
    I2C_Start();
    I2C_SendByte(DS3231_ADDRESS_WRITE); // 写地址
    if (I2C_ReceiveAck()) { I2C_Stop(); return; }
    
    I2C_SendByte(0x00); // 设置指针到 0x00 (秒)
    I2C_ReceiveAck();
    
    I2C_Start(); // 重复起始信号 (Restart)
    I2C_SendByte(DS3231_ADDRESS_READ);  // 读地址
    if (I2C_ReceiveAck()) { I2C_Stop(); return; }
    
    // 读秒 (ACK)
    ds_sec = BCD_To_Decimal(I2C_ReceiveByte());
    I2C_SendAck(0); // 发送ACK
    
    // 读分 (ACK)
    ds_min = BCD_To_Decimal(I2C_ReceiveByte());
    I2C_SendAck(0); // 发送ACK
    
    // 读时 (NACK - 最后一个字节发非应答)
    ds_hour = BCD_To_Decimal(I2C_ReceiveByte()& 0x3F); //屏蔽掉 Bit6 的 12/24H 标志位
    I2C_SendAck(1); // 发送NACK，告诉从机读完了
    
    I2C_Stop();
}
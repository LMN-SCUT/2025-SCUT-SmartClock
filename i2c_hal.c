#include "i2c_hal.h"
#include <INTRINS.H> 



// ==========================================
// [硬件定义] I2C 引脚分配
// SCL 接 P3.4
// SDA 接 P3.5
// 注意：DS3231 和 AT24C02 共用这两根线！
// ==========================================
sbit I2C_SCL = P3^4;
sbit I2C_SDA = P3^5;

// I2C 延时函数
// STC89C52 @ 12MHz，_nop_() 约为 1us
// I2C 标准速率需要 5us 左右的保持时间
void I2C_Delay(void)
{
    unsigned char i = 2; // 循环大概消耗 2*4 + 2 = 10us 左右
    while(i--);
}

// I2C 初始化
void I2C_Init(void)
{
    // 1. 先把引脚设为高（释放总线）
    I2C_SCL = 1;
    I2C_SDA = 1;
    
    // 2. 执行总线恢复/解锁检测
    I2C_Bus_Recovery();
    
    // 3. 再次确保释放
    I2C_SCL = 1;
    I2C_SDA = 1;
}

// I2C 起始信号
void I2C_Start(void)
{
    I2C_SDA = 1;
    I2C_SCL = 1;
    I2C_Delay();
    I2C_SDA = 0;
    I2C_Delay();
    I2C_SCL = 0;
}

// I2C 停止信号
void I2C_Stop(void)
{
    I2C_SDA = 0;
    I2C_SCL = 1;
    I2C_Delay();
    I2C_SDA = 1;
    I2C_Delay();
}

// I2C 发送一个字节
void I2C_SendByte(unsigned char Byte)
{
    unsigned char i;
    for(i=0; i<8; i++)
    {
        // 取出最高位
        I2C_SDA = Byte & (0x80 >> i);
        I2C_Delay();
        I2C_SCL = 1; // 拉高时钟，通知从机读取
        I2C_Delay();
        I2C_SCL = 0; // 拉低时钟，准备下一位
        I2C_Delay();
    }
}

// I2C 接收一个字节
unsigned char I2C_ReceiveByte(void)
{
    unsigned char i, Byte=0;
    I2C_SDA = 1; // 释放总线（输入模式）
    for(i=0; i<8; i++)
    {
        I2C_SCL = 1; // 拉高时钟
        I2C_Delay();
        if(I2C_SDA) { 
            Byte |= (0x80 >> i); // 读取数据
        }
        I2C_SCL = 0; // 拉低时钟
        I2C_Delay();
    }
    return Byte;
}

// 发送应答信号
// AckBit: 0=应答(ACK), 1=非应答(NACK)
void I2C_SendAck(bit AckBit)
{
    I2C_SDA = AckBit;
    I2C_Delay();
    I2C_SCL = 1;
    I2C_Delay();
    I2C_SCL = 0;
    I2C_Delay();
}

// 接收应答信号
// 返回值: 0=应答(ACK), 1=非应答(NACK)
bit I2C_ReceiveAck(void)
{
    bit AckBit;
    I2C_SDA = 1; // 释放总线
    I2C_Delay();
    I2C_SCL = 1;
    I2C_Delay();
    AckBit = I2C_SDA; // 读取应答
    I2C_SCL = 0;
    I2C_Delay();
    return AckBit;
}
// I2C 总线解锁/恢复函数
void I2C_Bus_Recovery(void) {
    unsigned char i;
    
    // 如果发现 SDA 是一上电就是低电平（说明被从机拉死了）
    if (I2C_SDA == 0) {
        // 尝试发送 9 个时钟脉冲，把从机里的数据“挤”出来
        for (i = 0; i < 9; i++) {
            I2C_SCL = 0; 
            I2C_Delay();
            I2C_SCL = 1; // 产生一个上升沿，骗从机走一位
            I2C_Delay();
        }
        
        // 发送一个 STOP 信号，彻底复位总线
        I2C_Start(); // 这里虽然SDA可能还是低，但试着造一个停止条件
        I2C_Stop();
    }
}

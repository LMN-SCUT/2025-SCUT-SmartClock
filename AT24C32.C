#include "AT24C32.h"
#include "i2c_hal.h"
#include "delay.h" // 需要用到Delay(5)

// ZS-042 模块上的 AT24C32 默认地址通常是 0xAE (A0/A1/A2 全部上拉)
// 如果读写失败，可以让硬件组把A0/A1/A2短接到地，那样地址就是 0xA0
#define AT24C32_ADDRESS_WRITE  0xAE 
#define AT24C32_ADDRESS_READ   0xAF
#define address_length 8

void AT24C32_WriteByte(unsigned int WordAddress, unsigned char Data)
{
    I2C_Start();
    I2C_SendByte(AT24C32_ADDRESS_WRITE);
    if(I2C_ReceiveAck()) { I2C_Stop(); return; }
    
    I2C_SendByte(WordAddress>>address_length); // 写入存储地址
    if(I2C_ReceiveAck()) { I2C_Stop(); return; }

	I2C_SendByte(WordAddress & 0xFF); // 发送低8位地址
    if(I2C_ReceiveAck()) { I2C_Stop(); return; }
    
    I2C_SendByte(Data);        // 写入数据
    if(I2C_ReceiveAck()) { I2C_Stop(); return; }
    
    I2C_Stop();
    
    // [关键] EEPROM 写入需要时间，必须延时5ms以上
    Delay(10); 
}

unsigned char AT24C32_ReadByte(unsigned int WordAddress)
{
    unsigned char Data;
    
    I2C_Start();
    I2C_SendByte(AT24C32_ADDRESS_WRITE);
    if(I2C_ReceiveAck()) { I2C_Stop(); return 0; }
    
    I2C_SendByte(WordAddress >> address_length );   // 高8位
    if(I2C_ReceiveAck()) { I2C_Stop(); return 0; }
    
    I2C_SendByte(WordAddress & 0xFF); // 低8位
    if(I2C_ReceiveAck()) { I2C_Stop(); return 0; }
    
    I2C_Start(); // Restart
    I2C_SendByte(AT24C32_ADDRESS_READ); // 读地址
    if(I2C_ReceiveAck()) { I2C_Stop(); return 0; }
    
    Data = I2C_ReceiveByte();
    I2C_SendAck(1); // 读取最后一个字节后发非应答(NACK)
    I2C_Stop();
    
    return Data;
}
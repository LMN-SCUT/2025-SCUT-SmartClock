#ifndef __I2C_HAL_H__
#define __I2C_HAL_H__

#include <REGX52.H>



// º¯ÊýÉùÃ÷
void I2C_Init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_SendByte(unsigned char Byte);
unsigned char I2C_ReceiveByte(void);
void I2C_SendAck(bit AckBit);
bit I2C_ReceiveAck(void);

#endif
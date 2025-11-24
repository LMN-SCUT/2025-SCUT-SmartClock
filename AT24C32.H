#ifndef __AT24C32_H__
#define __AT24C32_H__

#include <REGX52.H>

// 写入一个字节到指定地址
void AT24C32_WriteByte(unsigned int WordAddress, unsigned char Data);

// 从指定地址读取一个字节
unsigned char AT24C32_ReadByte(unsigned int WordAddress);

#endif
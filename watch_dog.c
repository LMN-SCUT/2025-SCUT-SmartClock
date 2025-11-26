#include <REGX52.H>
#include "watch_dog.h"

// STC89C52 的看门狗寄存器地址是 0xE1
// 这是一个特殊功能寄存器，标准 regx52.h 里没有，需要自己定义
sfr WDT_CONTR = 0xE1; 

/* 
   WDT_CONTR 寄存器说明：
   Bit 7-6: 保留
   Bit 5: EN_WDT (1=启动看门狗)
   Bit 4: CLR_WDT (1=喂狗/清零计数器)
   Bit 3: IDLE_WDT (1=空闲模式下也计数)
   Bit 2-0: PS (预分频系数，决定溢出时间)
   
   我们将 PS 设置为 101 (即5)，预分频 = 64
   在 12MHz 晶振下，溢出时间大约是 2秒左右。
   这意味着你必须在 2秒内喂一次狗，否则就复位。
*/

void WDT_Init(void) {
    // 启动看门狗，设置溢出时间约为 2秒
    // 0x20(启动) | 0x05(分频系数)
    WDT_CONTR = 0x25; 
}

void WDT_Feed(void) {
    // 喂狗：必须同时置位 EN_WDT 和 CLR_WDT
    // 0x20(启动) | 0x10(清零) | 0x05(保持分频系数不变)
    WDT_CONTR = 0x35; 
}
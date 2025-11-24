#ifndef __DISPLAY_MANAGE_H__
#define __DISPLAY_MANAGE_H__

#include "event.h"  // 需要包含event.h来定义Event类型

void Display_Init(void);
void Display_manage(Event *e);  

#endif
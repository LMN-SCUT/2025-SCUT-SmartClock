
#ifndef __TIMER_H__
#define __TIMER_H__
#include "event.h"
void time_manage(Event*e);
void Timer2_Init(void);
unsigned long Timer_GetMilliseconds(void);
void Timer_IncrementMs(void);
#endif
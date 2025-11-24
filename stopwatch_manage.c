#include <REGX52.H>
#include "stopwatch_manage.h"
#include "event.h"
#include "timer.h"

// 秒表状态变量（保持不变）
static StopwatchState sw_state = STOPWATCH_IDLE;
static unsigned long total_ms = 0;
static unsigned int last_update = 0;

void Stopwatch_Init(void) {
    sw_state = STOPWATCH_IDLE;
    total_ms = 0;
    last_update = 0;
}

unsigned long Stopwatch_GetTimeMs(void) {
    return total_ms;
}

void Stopwatch_Manage(Event* e) {
    unsigned int current_time;
    
    switch(e->type) {
        case EVENT_STOPWATCH_START:
            if(sw_state == STOPWATCH_IDLE || sw_state == STOPWATCH_PAUSED) {
                sw_state = STOPWATCH_RUNNING;
                last_update = Timer_GetMilliseconds();
            }
            break;
            
        case EVENT_STOPWATCH_PAUSE:
            if(sw_state == STOPWATCH_RUNNING) {
                sw_state = STOPWATCH_PAUSED;
                current_time = Timer_GetMilliseconds();
                total_ms += (current_time - last_update);
            }
            break;
            
        // 新增：处理开始/暂停切换事件
        case EVENT_STOPWATCH_TOGGLE:
            if(sw_state == STOPWATCH_IDLE || sw_state == STOPWATCH_PAUSED) {
                // 如果当前是空闲或暂停状态，则开始运行
                sw_state = STOPWATCH_RUNNING;
                last_update = Timer_GetMilliseconds();
            } else if(sw_state == STOPWATCH_RUNNING) {
                // 如果当前是运行状态，则暂停
                sw_state = STOPWATCH_PAUSED;
                current_time = Timer_GetMilliseconds();
                total_ms += (current_time - last_update);
            }
            break;
            
        case EVENT_STOPWATCH_RESET:
            sw_state = STOPWATCH_IDLE;
            total_ms = 0;
            break;
            
        case EVENT_STOPWATCH_UPDATE:
            if(sw_state == STOPWATCH_RUNNING) {
                current_time = Timer_GetMilliseconds();
                total_ms += (current_time - last_update);
                last_update = current_time;
                Event_Publish(EVENT_DISPLAY_UPDATE, 0, SYS_MODE_STOPWATCH);
                
            }
            break;
    }
}
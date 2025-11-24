#include <REGX52.H>
#include "menu_manage.h"
#include "event.h"
#include "LCD1602.h"

// 菜单选项定义（可根据需要扩展）
code const char* menu_options[] = {
    "1.Time Set    ",  // 选项1: 时间设置
    "2.Alarm Set   ",  // 选项2: 闹钟设置
    "3.Stopwatch   ",  // 选项3: 秒表模式
    "4.Num Input   ",  // 选项4: 数字输入
    "5.Future Mode "   // 选项5: 预留未来模式
};

void Menu_ShowOptions(void) {
    // 显示菜单第一页（可根据需要分页显示）
    LCD_ShowString(1, 1, "Menu: Select Num");
    LCD_ShowString(2, 1, menu_options[0]);  // 显示第一个选项
    // 如果需要显示更多选项，可以通过滚动或分页实现
}

void Menu_Manage(Event* e) {
    if (e->type == EVENT_MENU_SELECT) {
        // 处理菜单选择事件：根据数字键切换模式
        switch(e->dat) {
            case 1: // 按1键：进入时间设置模式
                Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_TIME_SET, SYS_MODE_MENU);
                break;
            case 2: // 按2键：进入闹钟设置模式
                Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_ALARM_SET, SYS_MODE_MENU);
                break;
            case 3: // 按3键：进入秒表模式
                Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_STOPWATCH, SYS_MODE_MENU);
                break;
            case 4: // 按4键：进入数字输入模式
                Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_NUMBER_INPUT, SYS_MODE_MENU);
                break;
            case 5: // 按5键：预留未来模式
                // 这里可以添加新模式的入口，例如 SYS_MODE_TEMP_MONITOR
                // Event_Publish(EVENT_SYS_MODE_CHANGE, SYS_MODE_TEMP_MONITOR, SYS_MODE_MENU);
                break;
            default:
                // 无效选择，可显示错误提示
                LCD_ShowString(2, 1, "Invalid choice!");
                break;
        }
    }
}
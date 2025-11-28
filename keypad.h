#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <REGX52.H>

// 键值定义
#define KEY_NONE      0xFF
#define KEY_MODE      16    // 模式切换键
#define KEY_SET       17    // 设置键
#define KEY_INC       18    // 增加键
#define KEY_DEC       19    // 减少键
#define KEY_ENTER     20    // 确认键
#define KEY_CANCEL    21    // 取消键

// 函数声明
/**
 * @brief 键盘扫描函数
 * @return 返回按键的原始键值（0-15对应4x4矩阵位置）
 * @note 该函数会进行按键消抖处理
 */

void Key_ProcessEvents(void);

#endif
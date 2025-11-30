2025 "Wireless Cup" Problem A: Smart Clock based on STC89C52RC
2025年“无线杯”电子设计大赛 A题：基于单片机的智能时钟系统
![alt text](https://img.shields.io/badge/Language-C51-blue.svg)
![alt text](https://img.shields.io/badge/Platform-STC89C52RC-green.svg)
![alt text](https://img.shields.io/badge/license-MIT-lightgrey.svg)
项目简介：
本项目是 2025 年“无线杯”电子设计大赛 A 题的解决方案。系统基于 STC89C52RC 单片机，采用了先进的 事件驱动架构 (Event-Driven Architecture) 和 有限状态机 (FSM) 设计模式，实现了高精度走时、多任务处理（秒表后台运行）、智能闹钟及掉电存储功能。

✨ 核心功能 (Key Features)
⏱️ 高精度 RTC: 采用 DS3231 时钟芯片，年误差极低，支持断电走时。
🧠 事件驱动架构: 摒弃传统的轮询死循环，采用任务队列机制，按键响应无阻塞，系统运行丝般顺滑。
🔢 双模式交互: 支持“传统菜单调整”与“数字键盘极速输入”两种时间设置方式。
🔔 智能闹钟:
支持 EEPROM (AT24C32) 掉电记忆。
声光同步报警（蜂鸣器 + LED 闪烁）。
具备防误触和防重复触发机制。
🏃 多任务秒表: 秒表可在后台独立运行，与主时钟显示互不干扰。
🛡️ 高可靠性设计:
内置硬件看门狗 (WDT)，防死机自动复位。
底层 I2C 通信加入超时重试与数据合理性校验（Anti-shake）。
按键采用非阻塞状态机消抖。
🛠️ 硬件清单 (Hardware)
组件名称	型号/参数	说明
主控芯片	STC89C52RC	12MHz 晶振, Small Model
时钟模块	DS3231 (ZS-042)	高精度 I2C 接口
存储芯片	AT24C32	板载于 DS3231 模块
显示屏	LCD1602	5V 蓝底白字
输入设备	4x4 矩阵键盘	自定义键值映射
报警输出	有源蜂鸣器 + LED	低电平触发
🔌 引脚定义 (Pinout)
模块	单片机引脚	备注
LCD1602	P0 (D0-D7)	数据口
P2.6 (RS), P2.5 (RW), P2.7 (EN)	控制口
I2C 总线	P3.4 (SCL)	模拟 I2C
P3.5 (SDA)	带上拉电阻
蜂鸣器	P3.7	低电平触发
LED	P2.4	低电平点亮
矩阵键盘	P1.0 - P1.7	行列扫描
📖 操作说明 (User Manual)
本系统按键布局经过特殊优化，功能定义如下：
按键	功能	说明
MOD (SW13)	菜单/主页	切换主界面与功能菜单
SET (SW09)	设置/保存	在主界面按此键可直接输入数字修改时间
ENT (SW05)	确认/开始	确认选择 / 秒表开始暂停
ESC (SW01)	取消/停止	停止闹钟响铃 / 秒表归零
0 ~ 9	数字输入	用于快速设置时间
💻 软件架构 (Architecture)
为了在资源有限的 51 单片机上实现多任务和高响应，本项目采用了以下分层架构：
Driver Layer (驱动层): i2c_hal, lcd1602, keypad (负责底层时序与硬件IO)。
Middle Layer (中间层):
Event_Control: 维护一个环形事件队列，实现生产者-消费者模型。
Timer: 提供系统滴答与毫秒级时间戳。
Application Layer (应用层):
main.c: 主循环负责分发事件。
*_manage.c: 各个功能模块（时钟、闹钟、秒表）作为独立的状态机运行。
🚀 如何编译 (Build)
使用 Keil uVision 4/5 打开 alarm_1.uvproj。
Target 设置中，Memory Model 选择 Small: variables in DATA (已通过代码优化解决 RAM 溢出问题)。
点击 Rebuild，生成 .hex 文件。
使用 STC-ISP 烧录，注意勾选 STC89C52RC 型号。
📄 许可证 (License)
本项目开源，供学习交流使用。
Created by [LMN] / 2025 Wireless Cup Team

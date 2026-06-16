# XPWinmine - 还原 Windows XP SP3 扫雷

通过逆向分析 Windows XP SP3 系统自带的扫雷游戏 (`winmine.exe`)，使用 C++ 对其进行源码级还原。

## 截图

![Minesweeper](https://upload.wikimedia.org/wikipedia/en/2/20/Minesweeper_Icon.png)

## 功能特性

- 完整还原 XP SP3 扫雷的游戏逻辑与界面绘制
- 支持三种难度模式：初级 (9×9, 10 雷)、中级 (16×16, 40 雷)、高级 (16×30, 99 雷)
- 支持自定义雷区大小与地雷数量
- 还原原版音效播放、计时器、笑脸按钮交互
- 还原注册表读写（游戏设置与最佳成绩）
- 还原 XYZZY 作弊码
- 还原帮助文档调用

## 项目结构

```
XPWinmine/
├── winmine.cpp          # 主源代码（游戏逻辑、窗口过程、绘图等）
├── Resource.h           # 资源 ID 定义
├── XPWinmine.rc         # 资源脚本（菜单、对话框、位图、图标、音效）
├── XPWinmine.h          # 项目头文件
├── framework.h          # 框架头文件
├── targetver.h          # 目标 Windows 版本
├── BITMAP*.bmp          # 原版位图资源（雷区格子、数字、笑脸）
├── WAVE*.wav            # 原版音效资源（胜利、失败、滴答）
├── ICON100_1.ico        # 原版图标
└── MANIFEST1_1.txt      # 应用程序清单
```

## 构建方式

### 环境要求

- Visual Studio 2019 或更高版本
- Windows SDK

### 编译步骤

1. 使用 Visual Studio 打开 `XPWinmine.sln`
2. 选择 `Release` 或 `Debug` 配置
3. 点击 **生成 → 生成解决方案** (Ctrl+Shift+B)

## 技术细节

本项目通过 IDA Pro 等逆向工具对 `winmine.exe` 进行静态分析，还原了以下核心模块：

| 模块 | 说明 |
|------|------|
| 窗口与消息处理 | `MainWinProc`、菜单命令、鼠标事件 |
| 雷区逻辑 | 布雷、翻格、自动展开空白区域、标旗、计数 |
| 界面绘制 | GDI 绘图、位图资源加载、边框绘制 |
| 计时与音效 | 游戏计时器、Win32 多媒体音效 |
| 注册表/INI | 游戏设置与最佳成绩持久化存储 |
| 对话框 | 自定义难度、最佳成绩、玩家姓名、关于 |

## 许可证

本项目仅供学习与研究用途。原版扫雷游戏的版权归 Microsoft 所有。

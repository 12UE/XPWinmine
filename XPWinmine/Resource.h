//=============================================================================
// Resource.h — 资源标识符定义
// Windows XP SP3 扫雷 (winmine.exe 5.1.2600.0)
//=============================================================================

#ifndef RESOURCE_H
#define RESOURCE_H

//=============================================================================
// 版本信息
//=============================================================================
#define VER_PRODUCT_VERSION         5,1,2600,0
#define VER_FILE_VERSION            5,1,2600,0

//=============================================================================
// 图标 (Icon)
//=============================================================================
#define IDI_GAME                    100     // 主图标 (ICON100_1.ico)

//=============================================================================
// 位图 (Bitmap) — 游戏图形资源
//=============================================================================
#define IDB_FACE                    410     // 笑脸表情 (BITMAP410_1.bmp)
#define IDB_FACE_ALT                411     // 笑脸表情备用 (BITMAP411_1.bmp)
#define IDB_TILES                   420     // 数字/雷图块 (BITMAP420_1.bmp)
#define IDB_TILES_ALT               421     // 数字/雷图块备用 (BITMAP421_1.bmp)
#define IDB_MISC                    430     // 杂项图形 (BITMAP430_1.bmp)
#define IDB_MISC_ALT                431     // 杂项图形备用 (BITMAP431_1.bmp)

//=============================================================================
// 声音 (Wave)
//=============================================================================
#define IDW_CLICK                   432     // 点击音效 (WAVE432_1.wav)
#define IDW_EXPLOSION               433     // 爆炸音效 (WAVE433_1.wav)
#define IDW_WIN                     434     // 胜利音效 (WAVE434_1.wav)

//=============================================================================
// 菜单 (Menu)
//=============================================================================
#define IDM_GAME_MENU               500     // 游戏主菜单

// --- "游戏(G)" 菜单项 ---
#define IDM_NEW                     510     // 开局(&N)\tF2
#define IDM_BEGINNER                521     // 初级(&B)
#define IDM_INTERMEDIATE            522     // 中级(&I)
#define IDM_EXPERT                  523     // 高级(&E)
#define IDM_CUSTOM                  524     // 自定义(&C)...
#define IDM_SAVE_GAME               525     // 保存局面(&V)...
#define IDM_MARK_MODE               527     // 标记(?)(&M)
#define IDM_COLOR                   529     // 颜色(&L)
#define IDM_SOUND                   526     // 声音(&S)
#define IDM_RECORDS                 528     // 扫雷英雄榜(&T)...
#define IDM_LOAD_GAME               530     // 读取局面(&O)...
#define IDM_UNDO_GAME               531     // 撤销(&U)
#define IDM_REDO_GAME               532     // 重做(&R)
#define IDM_EXIT                    512     // 退出(&X)

// --- "帮助(H)" 菜单项 ---
#define IDM_HELP_CONTENTS           590     // 目录(&C)\tF1
#define IDM_HELP_SEARCH             591     // 查找帮助主题(&S)...
#define IDM_HELP_USAGE              592     // 使用帮助(&H)
#define IDM_HELP_ABOUT              593     // 关于扫雷(&A)...

//=============================================================================
// 快捷键 (Accelerator)
//=============================================================================
#define IDA_MAIN                    501     // 快捷键表

//=============================================================================
// 对话框 (Dialog)
//=============================================================================
#define IDD_CUSTOM_DIFFICULTY       80      // 自定义难度对话框
#define IDD_PLAYER_NAME             600     // 玩家姓名对话框
#define IDD_HIGH_SCORES             700     // 扫雷英雄榜对话框

// --- 自定义难度对话框 (#80) 控件 ---
#define IDC_DLG_OK                  1       // 确定按钮
#define IDC_DLG_CANCEL              2       // 取消按钮
#define IDC_MINES_LABEL             111     // "雷数(&M):" 标签
#define IDC_HEIGHT_LABEL            112     // "高度(&H):" 标签
#define IDC_WIDTH_LABEL             113     // "宽度(&W):" 标签
#define IDC_HEIGHT_EDIT             141     // 高度输入框
#define IDC_WIDTH_EDIT              142     // 宽度输入框
#define IDC_MINES_EDIT              143     // 雷数输入框

// --- 玩家姓名对话框 (#600) 控件 ---
#define IDC_PLAYER_OK               100     // 确定按钮
#define IDC_RECORD_LABEL            601     // 记录文本标签
#define IDC_PLAYER_NAME_EDIT        602     // 姓名输入框

// --- 扫雷英雄榜对话框 (#700) 控件 ---
#define IDC_SCORES_OK               1       // 确定按钮
#define IDC_RESET_SCORES            707     // 重新计分(&R) 按钮
#define IDC_LABEL_BEGINNER          708     // "初级:" 标签
#define IDC_LABEL_INTERMEDIATE      709     // "中级:" 标签
#define IDC_LABEL_EXPERT            710     // "高级:" 标签
#define IDC_TIME_BEGINNER           701     // 初级时间/姓名
#define IDC_TIME_BEGINNER_DATE      702     // 初级日期
#define IDC_TIME_INTERMEDIATE       703     // 中级时间/姓名
#define IDC_TIME_INTERMEDIATE_DATE  704     // 中级日期
#define IDC_TIME_EXPERT             705     // 高级时间/姓名
#define IDC_TIME_EXPERT_DATE        706     // 高级日期

//=============================================================================
// 字符串表 (String Table)
//=============================================================================
#define IDS_MINEWEEPER              1       // "扫雷" (窗口标题)
#define IDS_GAME_NAME               3       // "扫雷程序"
#define IDS_TIMER_FAIL               4       // "无法设置计时器，请关闭一些应用程序后再试一次。"
#define IDS_OUT_OF_MEMORY           5       // "内存不足"
#define IDS_MINES_REMAINING         6       // "剩余: %d"
#define IDS_TIME_FORMAT             7       // "%d 秒"
#define IDS_NEW_GAME                8       // "开局"
#define IDS_NEW_BEGINNER_RECORD     9       // "你已打破初级记录！\r请输入大名："
#define IDS_NEW_INTERMEDIATE_RECORD 10      // "你已打破中级记录！\r请输入大名："
#define IDS_NEW_EXPERT_RECORD       11      // "你已打破高级记录！\r请输入大名："
#define IDS_APP_TITLE               12      // "扫雷"
#define IDS_COPYRIGHT               13      // "by Robert Donner and Curt Johnson"

//=============================================================================
// 雷块绘制状态 (Tile Display State — arrMineFieldData & 0x1F)
//=============================================================================
#define TILE_EMPTY                  0
#define TILE_NUMBER_1               1
#define TILE_NUMBER_2               2
#define TILE_NUMBER_3               3
#define TILE_NUMBER_4               4
#define TILE_NUMBER_5               5
#define TILE_NUMBER_6               6
#define TILE_NUMBER_7               7
#define TILE_NUMBER_8               8
#define TILE_QUESTION               9
#define TILE_MINE                   10
#define TILE_FLAG_WRONG             11
#define TILE_MINE_RED               12
#define TILE_PRESSED                13
#define TILE_FLAG                   14
#define TILE_UNOPENED               15

//=============================================================================
// 雷块位标志 (arrMineFieldData bit flags)
//=============================================================================
#define MINE_CELL_MARK              0x80    // 仅用于踩雷时标记已重置
#define MINE_CELL_FLAG              0x40    // 插旗/问号标记位
#define MINE_CELL_EXPLODED          0x20    // 爆炸标记
#define MINE_CELL_OPENED            0x10    // 已打开
#define MINE_CELL_HAS_MINE          0x08    // 有雷
#define TILE_DISPLAY_MASK           0x1F    // 雷块显示状态掩码 (5 bits, 0-15)
#define MINE_CELL_ADJACENT_MASK     0x07    // 周围雷数掩码 (0-8)

//=============================================================================
// 游戏状态位 (g_gameStatusArray[0])
//=============================================================================
#define GAME_STATUS_ACTIVE          0x01    // 游戏进行中
#define GAME_STATUS_PAUSED          0x08    // 游戏暂停

//=============================================================================
// 笑脸按钮状态 (nSmileyBtnState)
//=============================================================================
#define SMILEY_NORMAL               0       // 正常
#define SMILEY_WORRY                1       // 紧张 (按下左键时)
#define SMILEY_WIN                  2       // 胜利
#define SMILEY_DEAD                 3       // 踩雷
#define SMILEY_PRESSED              4       // 按下 (笑脸按钮被点击)

//=============================================================================
// 游戏常量
//=============================================================================
#define MAX_BOARD_WIDTH             30
#define MAX_BOARD_HEIGHT            24
#define CELL_SIZE                   16      // 每格像素
#define FACE_BUTTON_SIZE            24      // 笑脸按钮尺寸
#define FACE_BUTTON_TOP             16      // 笑脸按钮 Y 坐标
#define TIMER_INTERVAL              1000    // 计时器间隔 (毫秒)
#define GAME_CLASS_NAME             L"WinMine_Window"
#define REGISTRY_KEY                L"Software\\Microsoft\\winmine"

//=============================================================================
// 消息常量 (内部专用)
//=============================================================================
#define WM_GAME_ABOUT               (WM_USER + 100)

//=============================================================================
// Studio 自动生成区
//=============================================================================
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NO_MFC                 130
#define _APS_NEXT_RESOURCE_VALUE    129
#define _APS_NEXT_COMMAND_VALUE     32771
#define _APS_NEXT_CONTROL_VALUE     1000
#define _APS_NEXT_SYMED_VALUE       110
#endif
#endif

#ifndef IDC_STATIC
#define IDC_STATIC                  -1
#endif

#endif // RESOURCE_H

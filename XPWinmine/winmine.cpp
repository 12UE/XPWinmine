
#include <windows.h>
#include<CommCtrl.h>
#include <string.h> 
#include <float.h> 
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"comctl32.lib")
BOOL WINAPI HandleSmileyButtonInteraction(LPARAM lParam); // sub_100140C
DWORD UpdateMenuCheckStates(); // sub_1001516
INT_PTR WINAPI DialogFunc(HWND, UINT, WPARAM, LPARAM); // DialogFunc
BOOL WINAPI SetDlgItemTimeAndName(HWND hDlg, int nIDDlgItem, int timeValue, LPCWSTR lpString); // sub_10016BA
INT_PTR WINAPI HighScoresDialogProc(HWND hDlg, UINT uMsg, HWND wParam, LPARAM lParam); // sub_10016FA
INT_PTR WINAPI PlayerNameDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); // sub_100181F
int WINAPI GetAdjustedSystemMetrics(int nIndex); // sub_1001915
void WINAPI AdjustMainWindowPosAndSize(char flags); // sub_1001950
void OpenCustomDifficultyDialog(); // sub_1001B49
INT_PTR OpenPlayerNameDialog(); // sub_1001B81
INT_PTR OpenHighScoresDialog(); // sub_1001BAA
LRESULT CALLBACK MainWinProc(HWND hMainWnd, UINT Msg, WPARAM wParam, LPARAM lParam); // MainWinProc
HRSRC WINAPI FindBitmapResource(short resId); // sub_10023CD
unsigned int WINAPI CalculateBitmapSize(int width, int height); // sub_10023F1
int LoadBitmapResources(); // sub_1002414
BOOL DestroyBitmapResources(); // sub_1002607
BOOL CleanupResources(); // sub_100263C
int WINAPI DrawMineFieldCell(int cellX, int cellY); // sub_1002646
int WINAPI DrawEntireMineField(HDC hdc); // sub_10026A7
int RefreshMineField(); // sub_100272E
int WINAPI DrawDigitBitmap(HDC hdc, int xDest, int digitIndex); // sub_1002752
DWORD WINAPI DrawRemainingMinesCount(HDC hdc); // sub_1002785
int RefreshRemainingMinesCount(); // sub_1002801
DWORD WINAPI DrawGameTimer(HDC hdc); // sub_1002825
int RefreshGameTimer(); // sub_10028B5
int WINAPI DrawSmileyButtonBitmap(HDC hdc, int stateIndex); // sub_10028D9
int WINAPI RefreshSmileyButton(int stateIndex); // sub_1002913
HGDIOBJ WINAPI SetupDrawingPenAndROP(HDC hdc, char drawMode); // sub_100293D
int WINAPI DrawBorderLines(HDC hdc, int x, int topY, int rightX, int y, int borderLayers, int drawMode); // sub_1002971
int WINAPI DrawMainWindowBorders(HDC hdc); // sub_1002A22
int WINAPI OnPaint(HDC hdc); // OnPaint
int RefreshWindowContent(); // sub_1002AF0
int InitBitmapAndResetMineField(); // sub_1002B14
int WINAPI ReadRegValueWithRangeLimit(int regKeyIndex, int defaultValue, int minValue, int maxValue); // sub_1002B27
LPWSTR WINAPI ReadRegStringValue(int regKeyIndex, LPBYTE lpData); // sub_1002B80
LSTATUS InitRegistrySettings(); // sub_1002BC2
LSTATUS WINAPI WriteRegDwordValue(int regKeyIndex, DWORD Data); // sub_1002D55
LSTATUS WINAPI WriteRegStringValue(int regKeyIndex, LPCWSTR lpString); // sub_1002D7A
LSTATUS SaveSettingsToRegistry(); // sub_1002DAB
int WINAPI UpdateMineFieldCellAndDraw(int cellX, int cellY, char displayState); // sub_1002EAB
int ResetMineFieldData(); // sub_1002ED5
int WINAPI CountAdjacentMines(int cellX, int cellY); // sub_1002F3B
int WINAPI DrawMinesOnGameOver(char displayMode); // sub_1002F80
void GameTimerTick(); // sub_1002FE0
int WINAPI OpenMineFieldCell(int cellX, int cellY); // sub_1003008
int WINAPI AutoExpandBlankCells(int cellX, int cellY); // sub_1003084
int WINAPI CountAdjacentFlags(int cellX, int cellY); // sub_1003119
char WINAPI RestoreCellDisplayState(int cellX, int cellY); // sub_100316B
char WINAPI HighlightCellDisplayState(int cellX, int cellY); // sub_10031A0
char WINAPI HandleCellHighlightOnMouseMove(int newX, int newY); // sub_10031D4
int PauseGame(); // sub_100341C
int ResumeGame(); // sub_100344C
int WINAPI UpdateRemainingMinesDisplay(int delta); // sub_100346A
void WINAPI HandleGameOver(int bWin); // sub_100347C
void WINAPI HandleLeftClickOnCell(int cellX, int cellY); // sub_1003512
void WINAPI HandleMiddleClickOnCell(int cellX, int cellY); // sub_10035B7
void ResetGame(); // sub_100367A
void WINAPI HandleRightClickOnCell(int cellX, int cellY); // sub_100374F
int HandleCellOperationOnMouseUp(); // sub_10037E1
int InitSoundPlayback(); // sub_10038C2
BOOL StopSoundPlayback(); // sub_10038D7
void WINAPI PlayGameSoundEffect(int soundId); // sub_10038ED
int WINAPI GenerateRandomCellIndex(int range); // sub_1003940
int WINAPI ShowGameMessageBox(unsigned short msgId); // sub_1003950
int WINAPI LoadGameStringResource(unsigned short resId, LPWSTR lpBuffer, int cchBufferMax); // sub_10039E7
UINT WINAPI ReadINIValueWithRangeLimit(int keyIndex, INT nDefault, int minValue, int maxValue); // sub_1003A12
DWORD WINAPI ReadINIStringValue(int keyIndex, LPWSTR lpReturnedString); // sub_1003A87
LSTATUS InitGameSettings(); // sub_1003AB0
DWORD WINAPI SetMenuItemCheckState(unsigned short menuId, int bChecked); // sub_1003CC4
void WINAPI UpdateMenuDisplayState(int newState); // sub_1003CE5
INT ShowAboutDialog(); // sub_1003D1D
int WINAPI OpenHelpDocument(short helpType, int helpCommand); // sub_1003D76
int WINAPI GetDlgItemIntWithRangeLimit(HWND hDlg, int nIDDlgItem, int minValue, int maxValue); // sub_1003DF6
unsigned int SetFloatingPointControlWord(); // sub_1003FF4
INT_PTR WINAPI CallHtmlHelpFunction(INT_PTR hwndCaller, INT_PTR helpPath, int command, INT_PTR data); // sub_1004062
BOOL WINAPI GetHHCtrlOcxPath(LPBYTE lpData); // sub_10040FB
const CHAR REG_DEFAULT_VALUE_NAME[4] = { '\0', '\0', '\0', '\0' };
int g_gameStatusArray[4] = { 1, 143, 141, 142 };
int g_defaultMinesPerDifficulty[] = { 10, 40, 99 };    // 简单10雷，中等40雷，困难99雷
int g_defaultFieldHeightPerDifficulty[] = { 9, 16, 16 }; // 简单9行，中等16行，困难16行
int g_defaultFieldWidthPerDifficulty[] = { 9, 16, 30 };  // 简单9列，中等16列，困难30列
wchar_t g_cheatCode_XYZZY[6] = L"XYZZY";
DWORD g_helpContextIds[] =
{
  707, 1003,
  708, 1004,
  709, 1004,
  710, 1004,
  701, 1004,
  703, 1004,
  705, 1004,
  702, 1004,
  704, 1004,
  706, 1004,
  0,   0
};
const LPCWSTR g_regKeyNames[] =
{
  L"Difficulty",
  L"Mines",
  L"Height",
  L"Width",
  L"Xpos",
  L"Ypos",
  L"Sound",
  L"Mark",
  L"Menu",
  L"Tick",
  L"Color",
  L"Time1",
  L"Name1",
  L"Time2",
  L"Name2",
  L"Time3",
  L"Name3",
  L"AlreadyPlayed"
};
int nMouseGridX = -1;
int nMouseGridY = -1;
int bMouseCaptured = 0;
int nMouseButtonState = 0;
int bWindowInactive = 1;
int bInMenuLoop = 0;
int nCheatCodeCount = 0;
HGDIOBJ hDrawPen = (HGDIOBJ)(INT_PTR)0xDE3010E1;
int bConfigModified = 0;
int nSmileyBtnState = 0;
int bTimerRunning = 0;
int nTimerStateBackup = 0;
HMODULE hHHCtrlModule = NULL;
int bHHCtrlLoadFailed = 0;
INT_PTR pfnHtmlHelp = 0;
int nRemainingMinesDisplay = 40;
int nAutoExpandGridX[100] = { 0 };
int nTotalMines;
int nMineFieldWidth;
int nMineFieldHeight;
char arrMineFieldData[832];
char arrMineFieldDisplay[832];
int nDifficultyLevel;
UINT nCurDifficultyMines;
UINT nMineFieldHeightConfig;
UINT nMineFieldWidthConfig;
int nWindowPosX;
int nWindowPosY;
int nSoundState;
int bMarkMode;
int nTickMode;
int nMenuDisplayState;
int bColorMode;
int nBestTimeEasy;
int nBestTimeMedium;
int nBestTimeHard;
WCHAR wszBestPlayerNameEasy[32];
WCHAR wszBestPlayerNameMedium[32];
WCHAR wszBestPlayerNameHard[32];
int nAutoExpandCount;
int nGameTimerSeconds;
int nOpenedSafeGrids;
int nTotalOpenedGrids;
int nAutoExpandGridY[100];
HKEY hRegKeyWinMine;
HGLOBAL hResBitmap24x24;
HGLOBAL hResBitmap16x16;
BITMAPINFO* pBmpInfo13x23;
int arrBmpOffset24x24[5];
char arrBmpTempBuffer[12];
INT_PTR arrCompatibleBmp16x16[16];
int arrBmpOffset16x16[16];
BITMAPINFO* pBmpInfo24x24;
BITMAPINFO* pBmpInfo16x16;
HGLOBAL hResBitmap13x23;
HDC arrBmpDC16x16[16];
int arrBmpOffset13x23[12];
int nWindowBorderWidth = 2;
HMENU hMainMenu;
WCHAR wszTempBuffer[32];
WCHAR wszFormatString[32];
int nWindowBottomY;
HWND hMainWnd;
int nWindowRightX;
HINSTANCE hAppInstance;
int nMenuHeight;
int bWindowInitFlag;
WCHAR wszDefaultString[32];
int nWindowTitleHeight;
int nWindowScrollHeight;
int nWindowClientHeight;
BOOL WINAPI HandleSmileyButtonInteraction(LPARAM lParam)
{
  struct tagMSG Msg;
  RECT rc;
  POINT mousePoint = { (unsigned short)lParam, HIWORD(lParam) };

  Msg.pt = mousePoint;
  rc.left = (nWindowRightX - 24) >> 1;
  rc.right = rc.left + 24;
  rc.top = 16;
  rc.bottom = 40;

  if ( !PtInRect(&rc, mousePoint) )
    return 0;

  SetCapture(hMainWnd);
  RefreshSmileyButton(4);
  MapWindowPoints(hMainWnd, 0, (LPPOINT)&rc, 2u);
  int bInsideRect = 1;
  do
  {
    while ( 1 )
    {
      while ( !PeekMessageW(&Msg, hMainWnd, 0x200u, 0x20Du, 1u) )
        ;
      if ( Msg.message != 512 )
        break;
      if ( PtInRect(&rc, Msg.pt) )
      {
        if ( !bInsideRect )
        {
          bInsideRect = 1;
          RefreshSmileyButton(4);
        }
      }
      else if ( bInsideRect )
      {
        bInsideRect = 0;
        RefreshSmileyButton(nSmileyBtnState);
      }
    }
  }
  while ( Msg.message != 514 );
  if ( bInsideRect && PtInRect(&rc, Msg.pt) )
  {
    nSmileyBtnState = 0;
    RefreshSmileyButton(0);
    ResetGame();
  }
  ReleaseCapture();
  return 1;
}
DWORD UpdateMenuCheckStates()
{
  SetMenuItemCheckState(0x209u, (WORD)nDifficultyLevel == 0);
  SetMenuItemCheckState(0x20Au, (WORD)nDifficultyLevel == 1);
  SetMenuItemCheckState(0x20Bu, (WORD)nDifficultyLevel == 2);
  SetMenuItemCheckState(0x20Cu, (WORD)nDifficultyLevel == 3);
  SetMenuItemCheckState(0x211u, bColorMode);
  SetMenuItemCheckState(0x20Fu, bMarkMode);
  return SetMenuItemCheckState(0x20Eu, nSoundState);
}
BOOL WINAPI SetDlgItemTimeAndName(HWND hDlg, int nIDDlgItem, int timeValue, LPCWSTR lpString)
{
  WCHAR String[32];

  wsprintfW(String, wszFormatString, timeValue);
  SetDlgItemTextW(hDlg, nIDDlgItem, String);
  return SetDlgItemTextW(hDlg, nIDDlgItem + 1, lpString);
}
INT_PTR WINAPI HighScoresDialogProc(HWND hDlg, UINT uMsg, HWND wParam, LPARAM lParam)
{
  switch ( uMsg )
  {
    case 0x53u:
      WinHelpW(*(HWND *)(lParam + 12), L"winmine.hlp", 0xCu, (ULONG_PTR)g_helpContextIds);
      break;
    case 0x7Bu:
      WinHelpW(wParam, L"winmine.hlp", 0xAu, (ULONG_PTR)g_helpContextIds);
      break;
    case 0x110u:
LABEL_11:
      SetDlgItemTimeAndName(hDlg, 701, nBestTimeEasy, wszBestPlayerNameEasy);
      SetDlgItemTimeAndName(hDlg, 703, nBestTimeMedium, wszBestPlayerNameMedium);
      SetDlgItemTimeAndName(hDlg, 705, nBestTimeHard, wszBestPlayerNameHard);
      return 1;
    default:
      if ( uMsg == 273 && (WORD)wParam )
      {
        if ( (unsigned short)wParam <= 2u || (unsigned short)wParam == 100 || (unsigned short)wParam == 109 )
        {
          EndDialog(hDlg, 1);
          return 1;
        }
        if ( (unsigned short)wParam == 707 )
        {
          nBestTimeHard = 999;
          nBestTimeMedium = 999;
          nBestTimeEasy = 999;
          lstrcpyW(wszBestPlayerNameEasy, wszDefaultString);
          lstrcpyW(wszBestPlayerNameMedium, wszDefaultString);
          lstrcpyW(wszBestPlayerNameHard, wszDefaultString);
          bConfigModified = 1;
          goto LABEL_11;
        }
      }
      break;
  }
  return 0;
}
INT_PTR WINAPI PlayerNameDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if ( uMsg == 272 )
  {
    WCHAR String[128];
    LoadGameStringResource(nDifficultyLevel + 9, String, 128);
    SetDlgItemTextW(hDlg, 601, String);
    SendMessageW(GetDlgItem(hDlg, 602), 0xC5u, 0x20u, 0);
    WCHAR *pDisplayName = wszBestPlayerNameEasy;
    if ( (WORD)nDifficultyLevel )
    {
      pDisplayName = wszBestPlayerNameMedium;
      if ( (WORD)nDifficultyLevel != 1 )
        pDisplayName = wszBestPlayerNameHard;
    }
    SetDlgItemTextW(hDlg, 602, pDisplayName);
  }
  else
  {
    if ( uMsg != 273
      || !(WORD)wParam
      || (unsigned short)wParam > 2u && (unsigned short)wParam != 100 && (unsigned short)wParam != 109 )
    {
      return 0;
    }
    WCHAR *pSaveTarget = wszBestPlayerNameEasy;
    if ( (WORD)nDifficultyLevel )
    {
      pSaveTarget = wszBestPlayerNameMedium;
      if ( (WORD)nDifficultyLevel != 1 )
        pSaveTarget = wszBestPlayerNameHard;
    }
    GetDlgItemTextW(hDlg, 602, pSaveTarget, 32);
    EndDialog(hDlg, 1);
  }
  return 1;
}
int WINAPI GetAdjustedSystemMetrics(int nIndex)
{
  if ( nIndex == 0 )
  {
    int result = GetSystemMetrics(78);
    return result ? result : GetSystemMetrics(0);
  }
  if ( nIndex == 1 )
  {
    int result = GetSystemMetrics(79);
    return result ? result : GetSystemMetrics(1);
  }
  return GetSystemMetrics(nIndex);
}
void WINAPI AdjustMainWindowPosAndSize(char flags)
{
    int xOverflow, screenHeight, adjustedY, yOverflow, windowHeight;
    struct tagRECT rcItem;
    struct tagRECT rcSecondItem;
    struct tagRECT rc;

    int bDoubleRowMenu = 0;

    if (hMainWnd)
    {
        nWindowClientHeight = nWindowTitleHeight;
        if ((nMenuDisplayState & 1) == 0)
        {
            nWindowClientHeight = nWindowTitleHeight + nMenuHeight;
            if (hMainMenu)
            {
                if (GetMenuItemRect(hMainWnd, hMainMenu, 0, &rcItem) &&
                    GetMenuItemRect(hMainWnd, hMainMenu, 1u, &rcSecondItem) &&
                    rcItem.top != rcSecondItem.top)
                {
                    nWindowClientHeight += nMenuHeight;
                    bDoubleRowMenu = 1;
                }
            }
        }

        nWindowRightX = 24 + 16 * nMineFieldWidth;
        nWindowBottomY = 67 + 16 * nMineFieldHeight;

                xOverflow = nWindowPosX + nWindowRightX - GetAdjustedSystemMetrics(0);
        if (xOverflow > 0)
        {
            flags |= 2u;
            nWindowPosX -= xOverflow;
        }

        screenHeight = GetAdjustedSystemMetrics(1);
        adjustedY = nWindowPosY;
        yOverflow = nWindowPosY + nWindowBottomY - screenHeight;
        if (yOverflow > 0)
        {
            flags |= 2u;
            adjustedY = nWindowPosY - yOverflow;
            nWindowPosY -= yOverflow;
        }

        if (!bWindowInitFlag)
        {
            if ((flags & 2) != 0)
            {
                MoveWindow(hMainWnd, nWindowPosX, adjustedY,
                    nWindowRightX + nWindowBorderWidth+13,      // 加上边框宽度
                    nWindowBottomY + nWindowClientHeight+13,
                    1);
            }

            if (bDoubleRowMenu && hMainMenu &&
                GetMenuItemRect(hMainWnd, hMainMenu, 0, &rcItem) &&
                GetMenuItemRect(hMainWnd, hMainMenu, 1u, &rcSecondItem) &&
                rcItem.top == rcSecondItem.top)
            {
                nWindowClientHeight -= nMenuHeight;
                windowHeight = nWindowBottomY + nWindowClientHeight;
                MoveWindow(hMainWnd, nWindowPosX, nWindowPosY,
                    nWindowRightX + nWindowBorderWidth,  // 加上边框宽度
                    windowHeight, 1);
            }
            if ((flags & 4) != 0)
            {
                SetRect(&rc, 0, 0, nWindowRightX, nWindowBottomY);
                InvalidateRect(hMainWnd, &rc, 1);
            }
        }
    }
}
void OpenCustomDifficultyDialog()
{
  DialogBoxParamW(hAppInstance, (LPCWSTR)0x50, hMainWnd, DialogFunc, 0);
  nDifficultyLevel = 3;
  UpdateMenuCheckStates();
  bConfigModified = 1;
  ResetGame();
}
INT_PTR OpenPlayerNameDialog()
{
  INT_PTR result;
  result = DialogBoxParamW(hAppInstance, (LPCWSTR)0x258, hMainWnd, PlayerNameDialogProc, 0);
  bConfigModified = 1;
  return result;
}
INT_PTR OpenHighScoresDialog()
{
  return DialogBoxParamW(hAppInstance, (LPCWSTR)0x2BC, hMainWnd, (DLGPROC)HighScoresDialogProc, 0);
}
LRESULT CALLBACK MainWinProc(
    HWND hMainWnd,    // 主窗口句柄
    UINT uMsg,        // 窗口消息类型
    WPARAM wParam,    // 消息参数1
    LPARAM lParam     // 消息参数2（鼠标/命令等数据）
)
{
    LPARAM mouseLParamCopy;
    HDC paintDC;
    WPARAM sysCmdType;
    BOOL isCheatCodeValid;
    int cheatGridX;
    int cheatGridY;
    HDC screenDC;
    struct tagPAINTSTRUCT paintStruct;

    mouseLParamCopy = lParam;

    // 处理鼠标相关消息（WM_MOUSEMOVE之后的消息：左键/右键/中键按下/抬起等）
    if (uMsg > (unsigned int)WM_MOUSEMOVE)
    {
        switch (uMsg)
        {
        case WM_LBUTTONDOWN: // 左键按下
            if (bWindowInactive)
                goto LABEL_WINDOW_INACTIVE; // 窗口非激活处理

            // 点击笑脸按钮则直接返回
            if (HandleSmileyButtonInteraction(lParam))
                return 0;

            // 游戏未运行则走默认窗口过程
            if ((g_gameStatusArray[0] & 1) == 0)
                return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);

            // 标记鼠标按钮状态（是否同时按下其他键）
            nMouseButtonState = (wParam & 6) != 0;
            goto LABEL_SET_MOUSE_CAPTURE; // 设置鼠标捕获
            break;

        case WM_LBUTTONUP:   // 左键抬起
        case WM_RBUTTONUP:   // 右键抬起
        case WM_MBUTTONUP:   // 中键抬起
            if (bMouseCaptured)
                goto LABEL_RELEASE_MOUSE_CAPTURE; // 释放鼠标捕获
            return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);

        case WM_RBUTTONDOWN: // 右键按下
            if (bWindowInactive)
                goto LABEL_WINDOW_INACTIVE; // 窗口非激活处理

            // 游戏未运行则走默认窗口过程
            if ((g_gameStatusArray[0] & 1) == 0)
                return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);

            // 已捕获鼠标：高亮处理+更新状态
            if (bMouseCaptured)
            {
                HandleCellHighlightOnMouseMove(-3, -3);
                nMouseButtonState = 1;
                PostMessageW(hMainWnd, WM_MOUSEMOVE, wParam, lParam);
                return 0;
            }

            // 按下其他键则跳转到设置鼠标捕获
            if ((wParam & 1) != 0)
                goto LABEL_SET_MOUSE_CAPTURE;

            // 非菜单循环时处理右键点击格子
            if (!bInMenuLoop)
                HandleRightClickOnCell(
                    ((unsigned short)lParam + 4) >> 4,  // 计算网格X坐标
                    (int)(HIWORD(lParam) - 39) >> 4        // 计算网格Y坐标
                );
            return 0;

        case WM_MBUTTONDOWN: // 中键按下
            if (bWindowInactive)
            {
            LABEL_WINDOW_INACTIVE: // 窗口非激活标签
                bWindowInactive = 0;
                return 0;
            }

            // 游戏未运行则走默认窗口过程
            if ((g_gameStatusArray[0] & 1) == 0)
                return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);

            nMouseButtonState = 1;
        LABEL_SET_MOUSE_CAPTURE: // 设置鼠标捕获标签
            SetCapture(hMainWnd);       // 捕获鼠标
            nMouseGridX = -1;           // 重置鼠标网格X
            nMouseGridY = -1;           // 重置鼠标网格Y
            bMouseCaptured = 1;         // 标记鼠标已捕获
            RefreshSmileyButton(1);     // 刷新笑脸按钮为按下状态
            mouseLParamCopy = lParam;   // 备份鼠标参数
            break;

        case WM_ENTERMENULOOP: // 进入菜单循环
            bInMenuLoop = 1;
            return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);

        case WM_EXITMENULOOP: // 退出菜单循环
            bInMenuLoop = 0;
            return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);

        default: // 其他消息走默认处理
            return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
        }

    LABEL_MOUSE_MOVE_HANDLE: // 鼠标移动处理标签
        if (bMouseCaptured)
        {
            if ((g_gameStatusArray[0] & 1) != 0)
            {
                // 游戏运行中：处理鼠标移动时的格子高亮
                HandleCellHighlightOnMouseMove(
                    ((unsigned short)lParam + 4) >> 4,
                    (int)(HIWORD(lParam) - 39) >> 4
                );
            }
            else
            {
            LABEL_RELEASE_MOUSE_CAPTURE: // 释放鼠标捕获标签
                bMouseCaptured = 0;
                ReleaseCapture(); // 释放鼠标捕获
                if ((g_gameStatusArray[0] & 1) != 0)
                    HandleCellOperationOnMouseUp(); // 处理鼠标抬起后的格子操作
                else
                    HandleCellHighlightOnMouseMove(-2, -2); // 重置高亮
            }
            return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
        }

        // 无作弊码输入：走默认窗口过程
        if (!nCheatCodeCount)
            return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);

        // 作弊码逻辑：检查输入是否有效
        isCheatCodeValid = nCheatCodeCount <= 5;
        if (nCheatCodeCount == 5)
        {
            if ((wParam & 8) != 0)
                goto LABEL_CHEAT_CODE_ACTIVE; // 作弊码生效
            isCheatCodeValid = 1;
        }
        if (isCheatCodeValid)
            return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);

    LABEL_CHEAT_CODE_ACTIVE: // 作弊码生效标签
        // 计算作弊码对应的网格坐标
        cheatGridX = ((unsigned short)mouseLParamCopy + 4) >> 4;
        cheatGridY = (int)(HIWORD(mouseLParamCopy) - 39) >> 4;
        nMouseGridX = cheatGridX;
        nMouseGridY = cheatGridY;

        // 坐标有效时：在屏幕(0,0)像素标记地雷/非地雷（XYZZY作弊码）
        if (cheatGridX > 0 && cheatGridY > 0 && cheatGridX <= nMineFieldWidth && cheatGridY <= nMineFieldHeight)
        {
            screenDC = GetDC(0); // 获取屏幕DC
            // 地雷=黑色(0)，非地雷=白色(0xFFFFFF)
            SetPixel(screenDC, 0, 0, arrMineFieldData[32 * nMouseGridY + nMouseGridX] < 0 ? 0 : 0xFFFFFF);
            ReleaseDC(0, screenDC); // 释放屏幕DC
        }
        return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
    }

    // 处理鼠标移动消息
    if (uMsg == WM_MOUSEMOVE)
        goto LABEL_MOUSE_MOVE_HANDLE;

    // 处理键盘/窗口生命周期相关消息（≤WM_KEYDOWN）
    if (uMsg <= (unsigned int)WM_KEYDOWN)
    {
        if (uMsg == WM_KEYDOWN) // 键盘按键按下
        {
            switch (wParam)
            {
            case 0x10u: // Shift键：切换作弊码激活状态
                if (nCheatCodeCount >= 5)
                    nCheatCodeCount ^= 0x14u;
                break;

            case 0x73u: // S键：切换音效
                if (nSoundState > 1)
                {
                    if (nSoundState == 3)
                    {
                        StopSoundPlayback();
                        nSoundState = 2;
                    }
                    else
                    {
                        nSoundState = InitSoundPlayback();
                    }
                }
                break;

            case 0x74u: // T键：更新菜单显示状态1
                if (nMenuDisplayState)
                    UpdateMenuDisplayState(1);
                break;

            case 0x75u: // U键：更新菜单显示状态2
                if (nMenuDisplayState)
                    UpdateMenuDisplayState(2);
                break;

            default: // 其他键：输入XYZZY作弊码
                if (nCheatCodeCount < 5)
                    nCheatCodeCount = (g_cheatCode_XYZZY[nCheatCodeCount] == (wchar_t)wParam) ? nCheatCodeCount + 1 : 0;
                break;
            }
            return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
        }

        if (uMsg != WM_DESTROY) // 非窗口销毁消息
        {
            if (uMsg == WM_ACTIVATE) // 窗口激活/失活
            {
                if ((WORD)wParam == 2)
                    bWindowInactive = 1; // 窗口失活标记
                return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
            }

            if (uMsg != WM_PAINT) // 非绘制消息
            {
                if (uMsg == WM_WINDOWPOSCHANGED && (g_gameStatusArray[0] & 8) == 0)
                {
                    WINDOWPOS *wp = (WINDOWPOS *)lParam;
                    nWindowPosX = wp->x;
                    nWindowPosY = wp->y;
                }
                return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
            }

            // WM_PAINT：绘制窗口内容
            paintDC = BeginPaint(hMainWnd, &paintStruct);
            OnPaint(paintDC);
            EndPaint(hMainWnd, &paintStruct);
            return 0;
        }

        // WM_DESTROY：窗口销毁
        KillTimer(hMainWnd, 1u);    // 停止游戏计时器
        PostQuitMessage(0);         // 发送退出消息
        return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
    }

    // 处理其他消息（WM_KEYDOWN之后，如WM_COMMAND/WM_SYSCOMMAND/WM_TIMER）
    switch (uMsg)
    {
    case WM_COMMAND: // 菜单/按钮命令
        if ((unsigned short)wParam > 0x210u) // 帮助/关于等命令
        {
            if ((unsigned short)wParam != 529) // 非颜色模式切换
            {
                switch ((unsigned short)wParam)
                {
                case 0x24Eu: OpenHelpDocument(3, 0); break;  // 帮助-目录
                case 0x24Fu: OpenHelpDocument(1, 2); break;  // 帮助-操作方法
                case 0x250u: OpenHelpDocument(4, 0); break;  // 帮助-快捷键
                case 0x251u: ShowAboutDialog(); return 0;    // 关于对话框
                }
                return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
            }

            // 切换颜色模式（单色/彩色）
            bColorMode = !bColorMode;
            DestroyBitmapResources();
            if (LoadBitmapResources())
            {
                RefreshWindowContent();
                goto LABEL_UPDATE_CONFIG; // 更新配置
            }
            ShowGameMessageBox(5); // 加载位图失败提示
        }
        else // 难度/重置/高分等命令
        {
            if ((unsigned short)wParam == 528) // 高分榜
            {
                OpenHighScoresDialog();
                return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
            }

            if ((unsigned short)wParam == 510) // 重置游戏
            {
                ResetGame();
                return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
            }

            if ((unsigned short)wParam != WM_MOUSEMOVE)
            {
                if ((unsigned short)wParam <= 0x208u) // 无效命令
                    return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);

                if ((unsigned short)wParam <= 0x20Bu) // 难度选择（简单/中等/困难）
                {
                    nDifficultyLevel = (int)(wParam - 521);
                    int difficultyIdx = (unsigned short)(wParam - 521);
                    // 边界检查：防止非法索引
                    if (difficultyIdx >= 0 && difficultyIdx < 3)
                    {
                        nCurDifficultyMines = g_defaultMinesPerDifficulty[difficultyIdx];
                        nMineFieldHeightConfig = g_defaultFieldHeightPerDifficulty[difficultyIdx];
                        nMineFieldWidthConfig = g_defaultFieldWidthPerDifficulty[difficultyIdx];
                    }
                    else
                    {
                        // 非法索引时默认简单难度
                        nCurDifficultyMines = 10;
                        nMineFieldHeightConfig = 9;
                        nMineFieldWidthConfig = 9;
                    }
                    ResetGame();
                }
                else // 自定义难度/音效/标记模式
                {
                    switch ((unsigned short)wParam)
                    {
                    case 0x20Cu: // 自定义难度
                        OpenCustomDifficultyDialog();
                        return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);

                    case 0x20Eu: // 音效开关
                        if (nSoundState)
                        {
                            StopSoundPlayback();
                            nSoundState = 0;
                        }
                        else
                        {
                            nSoundState = InitSoundPlayback();
                        }
                        break;

                    case 0x20Fu: // 标记模式切换
                        bMarkMode = !bMarkMode;
                        break;

                    default: // 其他命令
                        return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
                    }
                }

            LABEL_UPDATE_CONFIG: // 配置修改后更新标签
                bConfigModified = 1;
                UpdateMenuDisplayState(nMenuDisplayState); // 更新菜单显示
                return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
            }

            ShowWindow(hMainWnd, 0); // 隐藏窗口
        }

        // 发送关闭窗口消息
        SendMessageW(hMainWnd, 0x112u, 0xF060u, 0);
        return 0;

    case WM_SYSCOMMAND: // 系统命令（最小化/最大化/暂停等）
        sysCmdType = wParam & 0xFFF0; // 提取系统命令类型
        if (sysCmdType == 61472) // SC_MINIMIZE/暂停游戏
        {
            PauseGame();
            g_gameStatusArray[0] |= 0xAu;
        }
        else if (sysCmdType == 61728) // SC_RESTORE/恢复游戏
        {
            g_gameStatusArray[0] = g_gameStatusArray[0] & 0xF5;
            ResumeGame();
            bWindowInactive = 0;
        }
        return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);

    case WM_TIMER: // 计时器消息（游戏计时）
        GameTimerTick();
        return 0;
    }

    // 未处理的消息：走默认窗口过程
    return DefWindowProcW(hMainWnd, uMsg, wParam, lParam);
}
HICON g_GameIcon;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  WNDCLASSW WndClass;
  struct tagMSG Msg;
  INITCOMMONCONTROLSEX picce;
  HACCEL hAccTable;

  hAppInstance = hInstance;
  InitGameSettings();
  if ( nCmdShow == 7 || (bWindowInitFlag = 0, nCmdShow == 2) )
    bWindowInitFlag = 1;
  picce.dwSize = 8;
  picce.dwICC = 5885;
  InitCommonControlsEx(&picce);
  g_GameIcon = LoadIconW(hAppInstance, (LPCWSTR)0x64);
  WndClass.style = 0;
  WndClass.lpfnWndProc = MainWinProc;
  WndClass.cbClsExtra = 0;
  WndClass.cbWndExtra = 0;
  WndClass.hInstance = hAppInstance;
  WndClass.hIcon = g_GameIcon;
  WndClass.hCursor = LoadCursorW(0, (LPCWSTR)0x7F00);
  WndClass.hbrBackground = (HBRUSH)GetStockObject(1);
  WndClass.lpszMenuName = 0;
  WndClass.lpszClassName = wszTempBuffer;
  if ( !RegisterClassW(&WndClass) )
    return 0;
  hMainMenu = LoadMenuW(hAppInstance, (LPCWSTR)0x1F4);
  hAccTable = LoadAcceleratorsW(hAppInstance, (LPCWSTR)0x1F5);
  InitRegistrySettings();
  hMainWnd = CreateWindowExW(
           0,
           wszTempBuffer,
           wszTempBuffer,
           0xCA0000u,
           nWindowPosX - nWindowBorderWidth,
           nWindowPosY - nWindowClientHeight,
           nWindowBorderWidth + nWindowRightX,
           nWindowClientHeight + nWindowBottomY,
           0,
           0,
           hAppInstance,
           0);
  if ( !hMainWnd )
  {
    ShowGameMessageBox(1000);
    return 0;
  }
  AdjustMainWindowPosAndSize(1);
  if ( !InitBitmapAndResetMineField() )
  {
    ShowGameMessageBox(5);
    return 0;
  }
  UpdateMenuDisplayState(nMenuDisplayState);
  ResetGame();
  ShowWindow(hMainWnd, 1);
  UpdateWindow(hMainWnd);
  bWindowInitFlag = 0;
  while ( GetMessageW(&Msg, 0, 0, 0) )
  {
    if ( !TranslateAcceleratorW(hMainWnd, hAccTable, &Msg) )
    {
      TranslateMessage(&Msg);
      DispatchMessageW(&Msg);
    }
  }
  CleanupResources();
  if ( bConfigModified )
    SaveSettingsToRegistry();
  return Msg.wParam;
}

HRSRC WINAPI FindBitmapResource(short resId)
{
  return FindResourceW(hAppInstance, (LPCWSTR)(unsigned short)(resId + (bColorMode == 0)), (LPCWSTR)2);
}


unsigned int WINAPI CalculateBitmapSize(int width, int height)
{
    int bitsPerPixel = (bColorMode != 0) ? 4 : 1;

    int bytesPerRow = ((width * bitsPerPixel + 31) / 32) * 4;

    return height * bytesPerRow;
}
int LoadBitmapResources()
{
    // 位图资源查找句柄（对应16x16/13x23/24x24三种位图）
    HRSRC hRes16x16;
    HRSRC hRes13x23;
    HRSRC hRes24x24;

    // 绘制画笔临时句柄（最终赋值给全局hDrawPen）
    HPEN hDrawPenTemp;

    // 位图尺寸/偏移计算变量
    unsigned int bitmap16x16Size;       // 16x16单张子位图的字节大小
    int* pBmpOffset16x16;               // 16x16位图偏移数组指针
    int bmpInfo16x16TotalSize;          // 16x16位图INFO头+调色板总大小
    int current16x16Offset;             // 16x16位图当前偏移值（循环累加）

    unsigned int bitmap13x23Size;       // 13x23单张子位图的字节大小
    int* pBmpOffset13x23;               // 13x23位图偏移数组指针
    int bmpInfo13x23TotalSize;          // 13x23位图INFO头+调色板总大小

    int bmpInfo24x24TotalSize;          // 24x24位图INFO头+调色板总大小
    unsigned int bitmap24x24Size;       // 24x24单张子位图的字节大小
    int* pBmpOffset24x24;               // 24x24位图偏移数组指针

    // 设备上下文相关
    HDC hMainWndDC;                     // 主窗口DC（用于创建兼容DC/位图）
    int i;                              // 通用循环变量
    HDC hCompatibleDC;                  // 兼容DC（用于绘制位图）
    HBITMAP hCompatibleBitmap;          // 兼容位图（与主窗口DC兼容）

    
    hResBitmap24x24 = NULL;
    hResBitmap13x23 = NULL;
    hResBitmap16x16 = NULL;

    // 步骤1：查找并加载16x16/13x23/24x24位图资源（资源ID：410/420/430）
    hRes16x16 = FindBitmapResource(410);
    if (hRes16x16) hResBitmap16x16 = LoadResource(hAppInstance, hRes16x16);

    hRes13x23 = FindBitmapResource(420);
    if (hRes13x23) hResBitmap13x23 = LoadResource(hAppInstance, hRes13x23);

    hRes24x24 = FindBitmapResource(430);
    if (hRes24x24) hResBitmap24x24 = LoadResource(hAppInstance, hRes24x24);
    if (!hResBitmap16x16 || !hResBitmap13x23 || !hResBitmap24x24)
        return 0;
    pBmpInfo16x16 = (BITMAPINFO*)LockResource(hResBitmap16x16);
    pBmpInfo13x23 = (BITMAPINFO*)LockResource(hResBitmap13x23);
    pBmpInfo24x24 = (BITMAPINFO*)LockResource(hResBitmap24x24);
    if (bColorMode)
    {
        hDrawPenTemp = CreatePen(PS_SOLID, 1, 0x808080u);
    }
    else
    {
        hDrawPenTemp = (HPEN)GetStockObject(BLACK_PEN);
    }
    // 赋值给全局绘制画笔句柄
    hDrawPen = hDrawPenTemp;

    if (bColorMode)
    {
        // 彩色模式：4位色深 → 16色调色板（每个调色板项4字节）
        // BITMAPINFOHEADER(40字节) + 16*4 = 104字节
        bmpInfo16x16TotalSize = 40 + 16 * 4;
        bmpInfo13x23TotalSize = 40 + 16 * 4;
        bmpInfo24x24TotalSize = 40 + 16 * 4;
    }
    else
    {
        // 单色模式：1位色深 → 2色调色板（黑/白）
        // BITMAPINFOHEADER(40字节) + 2*4 = 48字节
        bmpInfo16x16TotalSize = 40 + 2 * 4;
        bmpInfo13x23TotalSize = 40 + 2 * 4;
        bmpInfo24x24TotalSize = 40 + 2 * 4;
    }

    bitmap16x16Size = CalculateBitmapSize(16, 16);

    pBmpOffset16x16 = arrBmpOffset16x16;
    current16x16Offset = bmpInfo16x16TotalSize;
    for (i = 0; i < 16; i++)
    {
        // 填充当前子位图的偏移值
        *pBmpOffset16x16++ = current16x16Offset;
        // 累加偏移（指向下一个子位图）
        current16x16Offset += bitmap16x16Size;
    }

    bitmap13x23Size = CalculateBitmapSize(13, 23);

    pBmpOffset13x23 = arrBmpOffset13x23;
    for (i = 0; i < 12; i++)
    {
        *pBmpOffset13x23++ = bmpInfo13x23TotalSize;
        bmpInfo13x23TotalSize += bitmap13x23Size;
    }
    bitmap24x24Size = CalculateBitmapSize(24, 24);

    pBmpOffset24x24 = arrBmpOffset24x24;
    for (i = 0; i < 5; i++)
    {
        *pBmpOffset24x24++ = bmpInfo24x24TotalSize;
        bmpInfo24x24TotalSize += bitmap24x24Size;
    }
    hMainWndDC = GetDC(hMainWnd);
    for (i = 0; i < 16; ++i)
    {
        // 创建与主窗口DC兼容的内存DC
        hCompatibleDC = CreateCompatibleDC(hMainWndDC);
        arrBmpDC16x16[i] = hCompatibleDC;

        // 创建与主窗口DC兼容的位图（16x16尺寸）
        hCompatibleBitmap = CreateCompatibleBitmap(hMainWndDC, 16, 16);
        arrCompatibleBmp16x16[i] = (INT_PTR)hCompatibleBitmap;

        // 将兼容位图选入兼容DC
        SelectObject(arrBmpDC16x16[i], (HGDIOBJ)arrCompatibleBmp16x16[i]);

        // 将DIB位图数据绘制到兼容DC中（核心绘制逻辑）
        SetDIBitsToDevice(
            arrBmpDC16x16[i],        // 目标DC
            0, 0,                    // 目标X/Y坐标
            16, 16,                  // 目标宽度/高度
            0, 0,                    // 源位图起始X/Y
            0, 16,                   // 源位图扫描线范围（0到16行）
            (char*)pBmpInfo16x16 + arrBmpOffset16x16[i], // 源位图数据偏移
            pBmpInfo16x16,           // BITMAPINFO结构体指针
            DIB_RGB_COLORS           // 颜色表类型（RGB直接颜色）
        );
    }

    // 释放主窗口DC（避免资源泄漏）
    ReleaseDC(hMainWnd, hMainWndDC);

    // 所有位图资源加载/初始化成功
    return 1;
}

BOOL DestroyBitmapResources()
{
  int i;
  BOOL result;
  if ( hDrawPen )
    DeleteObject(hDrawPen);
  for ( i = 0; i < 16; ++i )
  {
    DeleteDC(arrBmpDC16x16[i]);
    result = DeleteObject((HGDIOBJ)arrCompatibleBmp16x16[i]);
  }
  return result;
}
BOOL CleanupResources()
{
  DestroyBitmapResources();
  return StopSoundPlayback();
}
int WINAPI DrawMineFieldCell(int cellX, int cellY)
{
  HDC DC = GetDC(hMainWnd);
  BitBlt(DC, 16 * cellX - 4, 16 * cellY + 39, 16, 16, arrBmpDC16x16[arrMineFieldData[32 * cellY + cellX] & 0x1F], 0, 0, 0xCC0020u);
  return ReleaseDC(hMainWnd, DC);
}
// 绘制整个雷区
// 参数：hdc - 绘制目标设备上下文
// 返回值：成功绘制的行数（等于雷区高度 nMineFieldHeight）
int WINAPI DrawEntireMineField(HDC hdc)
{
    int drawnRowCount = 1;
    if (nMineFieldHeight < 1)
        return drawnRowCount;

    int rowCounter = 1;
    int currentDrawY = 55;
    char* pCurrentRowDisplayData = arrMineFieldDisplay + 32;

    do
    {
        int colCounter = 1;
        int currentDrawX = 12;

        while (colCounter <= nMineFieldWidth)
        {
            int cellDisplayState = pCurrentRowDisplayData[colCounter] & 0x1F;
            BitBlt(hdc, currentDrawX, currentDrawY, 16, 16,
                   arrBmpDC16x16[cellDisplayState], 0, 0, 0xCC0020u);
            currentDrawX += 16;
            ++colCounter;
        }

        drawnRowCount = ++rowCounter;
        currentDrawY += 16;
        pCurrentRowDisplayData += 32;
    } while (rowCounter <= nMineFieldHeight);

    return drawnRowCount;
}
int RefreshMineField()
{
  HDC DC = GetDC(hMainWnd);
  DrawEntireMineField(DC);
  return ReleaseDC(hMainWnd, DC);
}
int WINAPI DrawDigitBitmap(HDC hdc, int xDest, int digitIndex)
{
  return SetDIBitsToDevice(
           hdc,
           xDest,
           16,
           0xDu,
           0x17u,
           0,
           0,
           0,
           0x17u,
           (char *)pBmpInfo13x23 + arrBmpOffset13x23[digitIndex],
           pBmpInfo13x23,
           0);
}
DWORD WINAPI DrawRemainingMinesCount(HDC hdc)
{
  DWORD savedLayout = GetLayout(hdc);
  if ( (savedLayout & 1) != 0 )
    SetLayout(hdc, 0);

  int hundreds, remainder;
  if ( nRemainingMinesDisplay >= 0 )
  {
    hundreds = nRemainingMinesDisplay / 100;
    remainder = nRemainingMinesDisplay % 100;
  }
  else
  {
    hundreds = 11;
    remainder = -nRemainingMinesDisplay % 100;
  }
  DrawDigitBitmap(hdc, 17, hundreds);
  DrawDigitBitmap(hdc, 30, remainder / 10);
  DWORD result = DrawDigitBitmap(hdc, 43, remainder % 10);
  if ( (savedLayout & 1) != 0 )
    return SetLayout(hdc, savedLayout);
  return result;
}
int RefreshRemainingMinesCount()
{
  HDC DC = GetDC(hMainWnd);
  DrawRemainingMinesCount(DC);
  return ReleaseDC(hMainWnd, DC);
}
DWORD WINAPI DrawGameTimer(HDC hdc)
{
    int timerValue = nGameTimerSeconds;
    DWORD Layout = GetLayout(hdc);

    if ((Layout & 1) != 0)
        SetLayout(hdc, 0);

    int timerHundreds = timerValue / 100;
    int timerRemainder = timerValue % 100;

    DrawDigitBitmap(hdc, nWindowRightX - 56, timerHundreds);
    DrawDigitBitmap(hdc, nWindowRightX - 43, timerRemainder / 10);
    DrawDigitBitmap(hdc, nWindowRightX - 30, timerRemainder % 10);

    if ((Layout & 1) != 0)
        return SetLayout(hdc, Layout);

    return 0;
}

int RefreshGameTimer()
{
  HDC DC = GetDC(hMainWnd);
  DrawGameTimer(DC);
  return ReleaseDC(hMainWnd, DC);
}
int WINAPI DrawSmileyButtonBitmap(HDC hdc, int stateIndex)
{
  return SetDIBitsToDevice(
           hdc,
           (nWindowRightX - 24) >> 1,
           16,
           0x18u,
           0x18u,
           0,
           0,
           0,
           0x18u,
           (char *)pBmpInfo24x24 + arrBmpOffset24x24[stateIndex],
           pBmpInfo24x24,
           0);
}
int WINAPI RefreshSmileyButton(int stateIndex)
{
  HDC DC = GetDC(hMainWnd);
  DrawSmileyButtonBitmap(DC, stateIndex);
  return ReleaseDC(hMainWnd, DC);
}
HGDIOBJ WINAPI SetupDrawingPenAndROP(HDC hdc, char drawMode)
{
  if ( (drawMode & 1) != 0 )
    return (HGDIOBJ)SetROP2(hdc, 16);
  SetROP2(hdc, 13);
  return SelectObject(hdc, hDrawPen);
}
int WINAPI DrawBorderLines(HDC hdc, int x, int topY, int rightX, int bottomY, int borderLayers, int drawMode)
{
    int borderLayerCount = 0;
    SetupDrawingPenAndROP(hdc, drawMode);

    if (borderLayers > 0)
    {
        borderLayerCount = borderLayers;
        int currentLayer = borderLayers;
        do
        {
            MoveToEx(hdc, x, --bottomY, 0);
            LineTo(hdc, x++, topY);
            LineTo(hdc, rightX--, topY++);
        } while (--currentLayer);
    }

    int nextBorderLayerCount = borderLayerCount + 1;
    if (drawMode < 2)
        SetupDrawingPenAndROP(hdc, drawMode ^ 1);

    int retVal = nextBorderLayerCount - 1;
    if (nextBorderLayerCount != 1)
    {
        int reverseBorderLayerCount = nextBorderLayerCount - 1;
        do
        {
            MoveToEx(hdc, x--, ++bottomY, 0);
            LineTo(hdc, ++rightX, bottomY);
            retVal = LineTo(hdc, rightX, --topY);
        } while (--reverseBorderLayerCount);
    }

    return retVal;
}



int WINAPI DrawMainWindowBorders(HDC hdc)
{
    int borderRight = nWindowRightX;
    int borderBottom = nWindowBottomY;
    int posX, posRight;

    borderRight -= 1;
    borderBottom -= 1;
    DrawBorderLines(hdc, 0, 0, borderRight, borderBottom, 3, 1);

    borderBottom += -9;
    borderRight -= 9;
    DrawBorderLines(hdc, 9, 52, borderRight, borderBottom, 3, 0);

    DrawBorderLines(hdc, 9, 9, borderRight, 45, 2, 0);

    DrawBorderLines(hdc, 16, 15, 56, 39, 1, 0);

    posX = nWindowRightX;
    posX -= 57;
    posRight = posX + 40;
    DrawBorderLines(hdc, posX, 15, posRight, 39, 1, 0);

    posX = nWindowRightX;
    posX += -24;
    posX >>= 1;
    posX -= 1;
    posRight = posX + 25;
    DrawBorderLines(hdc, posX, 15, posRight, 40, 1, 2);

    return 0;
}
int WINAPI OnPaint(HDC hdc)
{
  DrawMainWindowBorders(hdc);// 绘制边框
  DrawRemainingMinesCount(hdc);// 绘制剩余雷数
  DrawSmileyButtonBitmap(hdc, nSmileyBtnState);// 绘制笑脸按钮
  DrawGameTimer(hdc);// 绘制计时器
  return DrawEntireMineField(hdc);
}
int RefreshWindowContent()
{
  HDC DC = GetDC(hMainWnd);
  OnPaint(DC);
  return ReleaseDC(hMainWnd, DC);
}
int InitBitmapAndResetMineField()
{
  int result;
  result = LoadBitmapResources();
  if ( result )
  {
    ResetMineFieldData();
    return 1;
  }
  return result;
}
int WINAPI ReadRegValueWithRangeLimit(int regKeyIndex, int defaultValue, int minValue, int maxValue)
{
    int retVal;
    int tempLimitValue;
    const WCHAR* regKeyName;
    DWORD dataSize;

    regKeyName = g_regKeyNames[regKeyIndex];
    dataSize = 4;

    if (RegQueryValueExW(hRegKeyWinMine, regKeyName, 0, 0, (LPBYTE)&regKeyIndex, &dataSize))
        return defaultValue;

    tempLimitValue = maxValue;
    if (maxValue >= regKeyIndex)
        tempLimitValue = regKeyIndex;

    retVal = minValue;
    if (minValue <= tempLimitValue)
    {
        retVal = maxValue;
        if (maxValue >= regKeyIndex)
            return regKeyIndex;
    }

    return retVal;
}
LPWSTR WINAPI ReadRegStringValue(int regKeyIndex, LPBYTE lpData)
{
  LPWSTR result;
  const WCHAR *keyName;
  DWORD cbData;

  keyName = g_regKeyNames[regKeyIndex];
  cbData = 64;
  result = (LPWSTR)RegQueryValueExW(hRegKeyWinMine, keyName, 0, 0, lpData, &cbData);
  if ( result )
    return lstrcpyW((LPWSTR)lpData, wszDefaultString);
  return result;
}
LSTATUS InitRegistrySettings()
{
  HWND DesktopWindow;
  HDC DC;
  int DeviceCaps;
  DWORD dwDisposition;

  RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\winmine", 0, 0, 0, 0x20019u, 0, &hRegKeyWinMine, &dwDisposition);
  nMineFieldHeightConfig = ReadRegValueWithRangeLimit(2, 9, 9, 25);
  nMineFieldHeight = nMineFieldHeightConfig;
  nMineFieldWidthConfig = ReadRegValueWithRangeLimit(3, 9, 9, 30);
  nMineFieldWidth = nMineFieldWidthConfig;
  nDifficultyLevel = ReadRegValueWithRangeLimit(0, 0, 0, 3);
  nCurDifficultyMines = ReadRegValueWithRangeLimit(1, 10, 10, 999);
  nWindowPosX = ReadRegValueWithRangeLimit(4, 80, 0, 1024);
  nWindowPosY = ReadRegValueWithRangeLimit(5, 80, 0, 1024);
  nSoundState = ReadRegValueWithRangeLimit(6, 0, 0, 3);
  bMarkMode = ReadRegValueWithRangeLimit(7, 1, 0, 1);
  nTickMode = ReadRegValueWithRangeLimit(9, 0, 0, 1);
  nMenuDisplayState = ReadRegValueWithRangeLimit(8, 0, 0, 2);
  nBestTimeEasy = ReadRegValueWithRangeLimit(11, 999, 0, 999);
  nBestTimeMedium = ReadRegValueWithRangeLimit(13, 999, 0, 999);
  nBestTimeHard = ReadRegValueWithRangeLimit(15, 999, 0, 999);
  ReadRegStringValue(12, (LPBYTE)wszBestPlayerNameEasy);
  ReadRegStringValue(14, (LPBYTE)wszBestPlayerNameMedium);
  ReadRegStringValue(16, (LPBYTE)wszBestPlayerNameHard);
  DesktopWindow = GetDesktopWindow();
  DC = GetDC(DesktopWindow);
  DeviceCaps = GetDeviceCaps(DC, 24);
  bColorMode = ReadRegValueWithRangeLimit(10, DeviceCaps != 2, 0, 1);
  DesktopWindow = GetDesktopWindow();
  ReleaseDC(DesktopWindow, DC);
  if ( nSoundState == 3 )
    nSoundState = InitSoundPlayback();
  return RegCloseKey(hRegKeyWinMine);
}

LSTATUS WINAPI WriteRegDwordValue(int regKeyIndex, DWORD Data)
{
  return RegSetValueExW(hRegKeyWinMine, g_regKeyNames[regKeyIndex], 0, 4u, (const BYTE *)&Data, 4u);
}
LSTATUS WINAPI WriteRegStringValue(int regKeyIndex, LPCWSTR lpString)
{
  int strLen = lstrlenW(lpString);
  return RegSetValueExW(hRegKeyWinMine, g_regKeyNames[regKeyIndex], 0, 1u, (const BYTE *)lpString, 2 * strLen + 2);
}
LSTATUS SaveSettingsToRegistry()
{
  DWORD dwDisposition = 0;
  RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\winmine", 0, 0, 0, 0x20006u, 0, &hRegKeyWinMine, &dwDisposition);
  WriteRegDwordValue(0, nDifficultyLevel);
  WriteRegDwordValue(2, nMineFieldHeightConfig);
  WriteRegDwordValue(3, nMineFieldWidthConfig);
  WriteRegDwordValue(1, nCurDifficultyMines);
  WriteRegDwordValue(7, bMarkMode);
  WriteRegDwordValue(17, 1u);
  WriteRegDwordValue(10, bColorMode);
  WriteRegDwordValue(6, nSoundState);
  WriteRegDwordValue(4, nWindowPosX);
  WriteRegDwordValue(5, nWindowPosY);
  WriteRegDwordValue(11, nBestTimeEasy);
  WriteRegDwordValue(13, nBestTimeMedium);
  WriteRegDwordValue(15, nBestTimeHard);
  WriteRegStringValue(12, wszBestPlayerNameEasy);
  WriteRegStringValue(14, wszBestPlayerNameMedium);
  WriteRegStringValue(16, wszBestPlayerNameHard);
  return RegCloseKey(hRegKeyWinMine);
}
int WINAPI UpdateMineFieldCellAndDraw(int cellX, int cellY, char displayState)
{
  arrMineFieldData[32 * cellY + cellX] = displayState | arrMineFieldData[32 * cellY + cellX] & 0xE0;
  return DrawMineFieldCell(cellX, cellY);
}
int ResetMineFieldData()
{
    int x, y;
    int width = nMineFieldWidth;   // 游戏宽度
    int height = nMineFieldHeight;  // 游戏高度

    // 安全检查
    if (width < 1 || width > 30 || height < 1 || height > 24)
    {
        width = 9;
        height = 9;
        nMineFieldWidth = 9;
        nMineFieldHeight = 9;
    }

    // 清空整个数组
    memset(arrMineFieldData, 0, sizeof(arrMineFieldData));
    memset(arrMineFieldDisplay, 0, sizeof(arrMineFieldDisplay));

    // 1. 设置边界为 16（不可点击区域）
    for (x = 0; x <= width + 1; x++)
    {
        arrMineFieldData[32 * 0 + x] = 16;              // 顶部边界
        arrMineFieldData[32 * (height + 1) + x] = 16;   // 底部边界
    }
    for (y = 0; y <= height + 1; y++)
    {
        arrMineFieldData[32 * y + 0] = 16;              // 左侧边界
        arrMineFieldData[32 * y + (width + 1)] = 16;    // 右侧边界
    }

    // 2. 初始化雷区内部为 15（未打开的格子）
    for (y = 1; y <= height; y++)
    {
        for (x = 1; x <= width; x++)
        {
            arrMineFieldData[32 * y + x] = 15;   // 15 = 未打开的格子
            arrMineFieldDisplay[32 * y + x] = 15;   // 显示缓冲区也要初始化
        }
    }

    return 0;
}
int WINAPI CountAdjacentMines(int cellX, int cellY)
{
  int count = 0;
  char *pRow = &arrMineFieldData[32 * cellY - 32];
  int rowsLeft = 3;
  do
  {
    for ( int col = cellX - 1; col <= cellX + 1; ++col )
    {
      if ( pRow[col] < 0 )
        ++count;
    }
    pRow += 32;
    --rowsLeft;
  }
  while ( rowsLeft );
  return count;
}

int WINAPI DrawMinesOnGameOver(char displayMode)
{
    if (nMineFieldHeight < 1)
        return RefreshMineField();

    int y = 1;
    char* rowPtr = arrMineFieldDisplay + 32;

    while (y <= nMineFieldHeight)
    {
        int x = 1;
        while (x <= nMineFieldWidth)
        {
            char cellValue = arrMineFieldData[32 * y + x];

            // ===== 新增：同步已打开的格子 =====
            if ((cellValue & 0x40) != 0)  // 格子已打开
            {
                rowPtr[x] = cellValue;  // 直接复制状态（包括数字0-8）
            }
            // ===== 处理未打开的格子 =====
            else if ((rowPtr[x] & 0x40) == 0)  // 未打开的格子
            {
                if (cellValue < 0)  // 是地雷
                {
                    char currentDisplay = rowPtr[x] & 0x1F;

                    if (currentDisplay == 12)  // 12 = 已经是爆炸地雷
                    {
                        // 保持红色爆炸地雷不变
                    }
                    else if (currentDisplay == 14)  // 14 = 正确的旗子
                    {
                        // 保持旗子不变
                    }
                    else  // 未标记的地雷
                    {
                        rowPtr[x] = 10;  // 10 = 显示普通黑色地雷
                    }
                }
                else  // 不是地雷
                {
                    if ((rowPtr[x] & 0x1F) == 14)  // 错误的旗子
                    {
                        rowPtr[x] = 11;  // 11 = 错误标记（红叉）
                    }
                }
            }
            ++x;
        }
        ++y;
        rowPtr += 32;
    }

    return RefreshMineField();
}
void GameTimerTick()
{
  if ( bTimerRunning )
  {
    if ( nGameTimerSeconds < 999 )
    {
      ++nGameTimerSeconds;
      RefreshGameTimer();
      PlayGameSoundEffect(1);
    }
  }
}
int WINAPI OpenMineFieldCell(int cellX, int cellY)
{
  int cellIndex = cellX + 32 * cellY;
  int result = arrMineFieldData[cellIndex];
  if ( (result & 0x40) == 0 )
  {
    result &= 0x1Fu;
    if ( result != 16 && result != 14 )
    {
      ++nTotalOpenedGrids;
      int adjacentMines = CountAdjacentMines(cellX, cellY);
      arrMineFieldData[cellIndex] = adjacentMines | 0x40;
      result = DrawMineFieldCell(cellX, cellY);
      if ( !adjacentMines )
      {
        int queueIdx = nAutoExpandCount;
        nAutoExpandGridX[nAutoExpandCount] = cellX;
        nAutoExpandGridY[queueIdx] = cellY;
        result = queueIdx + 1;
        nAutoExpandCount = result;
        if ( result == 100 )
          nAutoExpandCount = 0;
      }
    }
  }
  return result;
}
int WINAPI AutoExpandBlankCells(int cellX, int cellY)
{
  int queueIdx = 1;
  nAutoExpandCount = 1;
  int result = OpenMineFieldCell(cellX, cellY);
  if ( nAutoExpandCount != 1 )
  {
    do
    {
      int expandX = nAutoExpandGridX[queueIdx];
      int expandY = nAutoExpandGridY[queueIdx] - 1;
      OpenMineFieldCell(expandX - 1, expandY);
      OpenMineFieldCell(expandX, expandY);
      OpenMineFieldCell(expandX + 1, expandY++);
      OpenMineFieldCell(expandX - 1, expandY);
      OpenMineFieldCell(expandX + 1, expandY++);
      OpenMineFieldCell(expandX - 1, expandY);
      OpenMineFieldCell(expandX, expandY);
      result = OpenMineFieldCell(expandX + 1, expandY);
      if ( ++queueIdx == 100 )
        queueIdx = 0;
    }
    while ( queueIdx != nAutoExpandCount );
  }
  return result;
}
int WINAPI CountAdjacentFlags(int cellX, int cellY)
{
  int count = 0;
  char *pRow = &arrMineFieldData[32 * cellY - 32];
  int rowsLeft = 3;
  do
  {
    for ( int col = cellX - 1; col <= cellX + 1; ++col )
    {
      if ( (pRow[col] & 0x1F) == 14 )
        ++count;
    }
    pRow += 32;
    --rowsLeft;
  }
  while ( rowsLeft );
  return count;
}
char WINAPI RestoreCellDisplayState(int cellX, int cellY)
{
  char *pCell = &arrMineFieldData[32 * cellY + cellX];
  int state = *pCell & 0x1F;
  if ( state == 13 )
  {
    state = 9;
  }
  else if ( state == 15 )
  {
    state = 0;
  }
  char result = state | *pCell & 0xE0;
  *pCell = result;
  return result;
}
char WINAPI HighlightCellDisplayState(int cellX, int cellY)
{
  char *pCell = &arrMineFieldData[32 * cellY + cellX];
  int state = *pCell & 0x1F;
  char newState;
  if ( state == 9 )
  {
    newState = 13;
LABEL_5:
    state = newState;
    goto LABEL_6;
  }
  if ( (*pCell & 0x1F) == 0 )
  {
    newState = 15;
    goto LABEL_5;
  }
LABEL_6:
  char result = state | *pCell & 0xE0;
  *pCell = result;
  return result;
}
char WINAPI HandleCellHighlightOnMouseMove(int newX, int newY)
{
  int curX = newX;
  int retVal = nMouseGridX;
  int prevGridY = nMouseGridY;
  if ( newX != nMouseGridX || newY != nMouseGridY )
  {
    int oldX = nMouseGridX;
    int oldY = nMouseGridY;
    nMouseGridX = newX;
    nMouseGridY = newY;
    if ( nMouseButtonState )
    {
      BOOL bNewInBounds = newX > 0 && newY > 0 && newX <= nMineFieldWidth && newY <= nMineFieldHeight;
      BOOL bOldInBounds = oldX > 0 && prevGridY > 0 && oldX <= nMineFieldWidth && prevGridY <= nMineFieldHeight;
      int oldTop = oldY - 1;
      if ( oldY - 1 <= 1 )
        oldTop = 1;
      int oldBottom = oldY + 1;
      if ( oldY + 1 >= nMineFieldHeight )
        oldBottom = nMineFieldHeight;
      int newTop = newY - 1;
      if ( newY - 1 <= 1 )
        newTop = 1;
      int newBottom = newY + 1;
      if ( newY + 1 >= nMineFieldHeight )
        newBottom = nMineFieldHeight;
      int oldLeft = oldX - 1;
      if ( oldX - 1 <= 1 )
        oldLeft = 1;
      int oldRight = oldX + 1;
      if ( oldX + 1 >= nMineFieldWidth )
        oldRight = nMineFieldWidth;
      int newLeft = newX - 1;
      if ( newX - 1 <= 1 )
        newLeft = 1;
      retVal = newX + 1;
      int newRight = newX + 1;
      if ( newX + 1 >= nMineFieldWidth )
        newRight = nMineFieldWidth;
      if ( bOldInBounds )
      {
        int oldRowIter = oldTop;
        if ( oldTop <= oldBottom )
        {
          char *pOldRow = &arrMineFieldData[32 * oldTop];
          do
          {
            int oldColIter = oldLeft;
            for ( retVal = oldLeft; oldColIter <= oldRight; ++oldColIter )
            {
              if ( (pOldRow[oldColIter] & 0x40) == 0 )
                retVal = HighlightCellDisplayState(oldColIter, oldRowIter);
            }
            ++oldRowIter;
            pOldRow += 32;
          }
          while ( oldRowIter <= oldBottom );
        }
      }
      if ( bNewInBounds )
      {
        int newRowIter = newTop;
        if ( newTop <= newBottom )
        {
          char *pNewRow = &arrMineFieldData[32 * newTop];
          do
          {
            for ( int col = newLeft; col <= newRight; ++col )
            {
              if ( (pNewRow[col] & 0x40) == 0 )
                retVal = RestoreCellDisplayState(col, newRowIter);
            }
            ++newRowIter;
            pNewRow += 32;
          }
          while ( newRowIter <= newBottom );
        }
      }
      if ( bOldInBounds )
      {
        for ( int row = oldTop; row <= oldBottom; ++row )
        {
          for ( int drawCol = oldLeft; drawCol <= oldRight; ++drawCol )
            retVal = DrawMineFieldCell(drawCol, row);
        }
      }
      if ( bNewInBounds )
      {
        for ( int drawRow = newTop; drawRow <= newBottom; ++drawRow )
        {
          for ( int drawCol2 = newLeft; drawCol2 <= newRight; ++drawCol2 )
            retVal = DrawMineFieldCell(drawCol2, drawRow);
        }
      }
    }
    else
    {
      if ( retVal > 0 && prevGridY > 0 && retVal <= nMineFieldWidth && prevGridY <= nMineFieldHeight )
      {
        retVal = 32 * prevGridY;
        if ( (arrMineFieldData[32 * prevGridY + oldX] & 0x40) == 0 )
        {
          HighlightCellDisplayState(oldX, prevGridY);
          retVal = DrawMineFieldCell(oldX, oldY);
          curX = newX;
        }
      }
      if ( curX > 0 && newY > 0 && curX <= nMineFieldWidth && newY <= nMineFieldHeight )
      {
        retVal = arrMineFieldData[32 * newY + curX];
        if ( (retVal & 0x40) == 0 )
        {
          retVal = retVal & 0x1F;
          if ( (BYTE)retVal != 14 )
          {
            int curGridY = nMouseGridY;
            int curGridX = nMouseGridX;
            RestoreCellDisplayState(nMouseGridX, nMouseGridY);
            retVal = DrawMineFieldCell(curGridX, curGridY);
          }
        }
      }
    }
  }
  return retVal;
}
int PauseGame()
{
  int result = StopSoundPlayback();
  if ( (g_gameStatusArray[0] & 2) == 0 )
  {
    result = bTimerRunning;
    nTimerStateBackup = bTimerRunning;
  }
  if ( (g_gameStatusArray[0] & 1) != 0 )
    bTimerRunning = 0;
  g_gameStatusArray[0] |= 2u;
  return result;
}
int ResumeGame()
{
  int result = 0;
  if ( (g_gameStatusArray[0] & 1) != 0 )
  {
    result = nTimerStateBackup;
    bTimerRunning = nTimerStateBackup;
  }
  g_gameStatusArray[0] = g_gameStatusArray[0] & 0xFD;
  return result;
}
int WINAPI UpdateRemainingMinesDisplay(int delta)
{
  nRemainingMinesDisplay += delta;
  return RefreshRemainingMinesCount();
}
void WINAPI HandleGameOver(int bWin)
{
  bTimerRunning = 0;
  nSmileyBtnState = (bWin != 0) + 2;
  RefreshSmileyButton((bWin != 0) + 2);
  DrawMinesOnGameOver(4 * (bWin != 0) + 10);
  if ( bWin && nRemainingMinesDisplay )
    UpdateRemainingMinesDisplay(-nRemainingMinesDisplay);
  PlayGameSoundEffect(3 - (bWin != 0));
  g_gameStatusArray[0] = 16;
  if ( bWin && (WORD)nDifficultyLevel != 3 && nGameTimerSeconds < *(&nBestTimeEasy + (unsigned short)nDifficultyLevel) )
  {
    *(&nBestTimeEasy + (unsigned short)nDifficultyLevel) = nGameTimerSeconds;
    OpenPlayerNameDialog();
    OpenHighScoresDialog();
  }
}

void WINAPI HandleLeftClickOnCell(int cellX, int cellY)
{
    char* pCell = &arrMineFieldData[32 * cellY + cellX];

    if (*pCell >= 0)
    {
        AutoExpandBlankCells(cellX, cellY);
        if (nTotalOpenedGrids == nOpenedSafeGrids)
            HandleGameOver(1);
    }
    else if (nTotalOpenedGrids)
    {
        UpdateMineFieldCellAndDraw(cellX, cellY, 12);

        arrMineFieldDisplay[32 * cellY + cellX] = 12;
        DrawMineFieldCell(cellX, cellY);

        HandleGameOver(0);
    }
    else
    {
        int searchY = 1;
        int searchX;
        char* pSearchRow;
        if (nMineFieldHeight > 1)
        {
            for (pSearchRow = arrMineFieldDisplay; ; pSearchRow += 32)
            {
                searchX = 1;
                if (nMineFieldWidth > 1)
                    break;
            LABEL_8:
                if (++searchY >= nMineFieldHeight)
                    return;
            }
            while (pSearchRow[searchX] < 0)
            {
                if (++searchX >= nMineFieldWidth)
                    goto LABEL_8;
            }
            *pCell = 15;
            arrMineFieldData[32 * searchY + searchX] |= 0x80u;
            AutoExpandBlankCells(cellX, cellY);
        }
    }
}

void WINAPI HandleMiddleClickOnCell(int cellX, int cellY)
{
  int bHitMine = 0;
  char cellData = arrMineFieldData[32 * cellY + cellX];
  if ( (cellData & 0x40) != 0 && (cellData & 0x1F) == CountAdjacentFlags(cellX, cellY) )
  {
    int rowY = cellY - 1;
    int bottomY = cellY + 1;
    int rightX, leftX;
    char *pRow;
    if ( cellY - 1 > cellY + 1 )
      goto LABEL_15;
    rightX = cellX + 1;
    leftX = cellX - 1;
    pRow = &arrMineFieldData[32 * rowY];
    do
    {
      for ( int col = leftX; col <= rightX; ++col )
      {
        if ( (pRow[col] & 0x1F) == 14 || pRow[col] >= 0 )
        {
          AutoExpandBlankCells(col, rowY);
        }
        else
        {
          bHitMine = 1;
          UpdateMineFieldCellAndDraw(col, rowY, 76);
        }
      }
      ++rowY;
      pRow += 32;
    }
    while ( rowY <= bottomY );
    if ( bHitMine )
    {
      HandleGameOver(0);
    }
    else
    {
LABEL_15:
      if ( nTotalOpenedGrids == nOpenedSafeGrids )
        HandleGameOver(1);
    }
  }
  else
  {
    HandleCellHighlightOnMouseMove(-2, -2);
  }
}
void ResetGame()
{
  bTimerRunning = 0;
  char resizeMode = (nMineFieldWidthConfig == nMineFieldWidth && nMineFieldHeightConfig == nMineFieldHeight) ? 4 : 6;
  nMineFieldWidth = nMineFieldWidthConfig;
  nMineFieldHeight = nMineFieldHeightConfig;
  ResetMineFieldData();
  nSmileyBtnState = 0;
  if ( nCurDifficultyMines >= (UINT)(nMineFieldWidth * nMineFieldHeight) )
    nCurDifficultyMines = nMineFieldWidth * nMineFieldHeight - 10;
  nTotalMines = nCurDifficultyMines;
  int randX, randY;
  do
  {
    do
    {
      randX = GenerateRandomCellIndex(nMineFieldWidth) + 1;
      randY = GenerateRandomCellIndex(nMineFieldHeight) + 1;
    }
    while ( arrMineFieldData[32 * randY + randX] < 0 );
    arrMineFieldData[32 * randY + randX] |= 0x80u;
    --nTotalMines;
  }
  while ( nTotalMines );
  nGameTimerSeconds = 0;
  nTotalMines = nCurDifficultyMines;
  nRemainingMinesDisplay = nCurDifficultyMines;
  nTotalOpenedGrids = 0;
  nOpenedSafeGrids = nMineFieldWidth * nMineFieldHeight - nCurDifficultyMines;
  g_gameStatusArray[0] = 1;
  UpdateRemainingMinesDisplay(0);
  AdjustMainWindowPosAndSize(resizeMode);
}
void WINAPI HandleRightClickOnCell(int cellX, int cellY)
{
  if ( cellX > 0 && cellY > 0 && cellX <= nMineFieldWidth && cellY <= nMineFieldHeight )
  {
    char *pCell = &arrMineFieldData[32 * cellY + cellX];
    if ( (*pCell & 0x40) == 0 )
    {
      char cellState = *pCell & 0x1F;
      char newState;
      if ( cellState == 14 )
      {
        newState = 2 * (bMarkMode == 0) + 13;
        UpdateRemainingMinesDisplay(1);
      }
      else if ( cellState == 13 )
      {
        newState = 15;
      }
      else
      {
        newState = 14;
        UpdateRemainingMinesDisplay(-1);
      }
      UpdateMineFieldCellAndDraw(cellX, cellY, newState);
      if ( (*pCell & 0x1F) == 14 && nTotalOpenedGrids == nOpenedSafeGrids )
        HandleGameOver(1);
    }
  }
}
int HandleCellOperationOnMouseUp()
{
  int gridX = nMouseGridX;
  if ( nMouseGridX > 0 )
  {
    int gridY = nMouseGridY;
    if ( nMouseGridY > 0 && nMouseGridX <= nMineFieldWidth && nMouseGridY <= nMineFieldHeight )
    {
      if ( !nTotalOpenedGrids && !nGameTimerSeconds )
      {
        PlayGameSoundEffect(1);
        ++nGameTimerSeconds;
        RefreshGameTimer();
        bTimerRunning = 1;
        if ( !SetTimer(hMainWnd, 1u, 0x3E8u, 0) )
          ShowGameMessageBox(4u);
        gridX = nMouseGridX;
        gridY = nMouseGridY;
      }
      if ( (g_gameStatusArray[0] & 1) == 0 )
      {
        gridY = -2;
        gridX = -2;
        nMouseGridY = -2;
        nMouseGridX = -2;
      }
      if ( nMouseButtonState )
      {
        HandleMiddleClickOnCell(gridX, gridY);
      }
      else
      {
        char cellData = arrMineFieldData[32 * gridY + gridX];
        if ( (cellData & 0x40) == 0 && (cellData & 0x1F) != 14 )
          HandleLeftClickOnCell(gridX, gridY);
      }
    }
  }
  return RefreshSmileyButton(nSmileyBtnState);
}
int InitSoundPlayback()
{
  return PlaySoundW(0, 0, 0x40u) + 2;
}
BOOL StopSoundPlayback()
{
    BOOL ret=FALSE;
    if (nSoundState == 3)
    {
        ret = PlaySoundW(NULL, NULL, 0x40u);
    }
    return ret;
}
void WINAPI PlayGameSoundEffect(int soundId)
{
  if ( nSoundState == 3 )
  {
    switch ( soundId )
    {
      case 1:
        PlaySoundW((LPCWSTR)0x1B0, hAppInstance, 0x40005u);
        break;
      case 2:
        PlaySoundW((LPCWSTR)0x1B1, hAppInstance, 0x40005u);
        break;
      case 3:
        PlaySoundW((LPCWSTR)0x1B2, hAppInstance, 0x40005u);
        break;
    }
  }
}
int WINAPI GenerateRandomCellIndex(int range)
{
  return rand() % range;
}
int WINAPI ShowGameMessageBox(unsigned short msgId)
{
  WCHAR wszTempBuffer[128];
  WCHAR Caption[128];

  if ( msgId >= 0x3E7u )
  {
    LoadStringW(hAppInstance, 6u, Caption, 128);
    wsprintfW(wszTempBuffer, Caption, msgId);
  }
  else
  {
    LoadStringW(hAppInstance, msgId, wszTempBuffer, 128);
  }
  LoadStringW(hAppInstance, 3u, Caption, 128);
  return MessageBoxW(0, wszTempBuffer, Caption, 0x10u);
}
int WINAPI LoadGameStringResource(unsigned short resId, LPWSTR lpBuffer, int cchBufferMax)
{
  int result;
  result = LoadStringW(hAppInstance, resId, lpBuffer, cchBufferMax);
  if ( !result )
    return ShowGameMessageBox(0x3E9u);
  return result;
}
UINT WINAPI ReadINIValueWithRangeLimit(int keyIndex, INT nDefault, int minValue, int maxValue)
{
  signed int PrivateProfileIntW;

  PrivateProfileIntW = GetPrivateProfileIntW(wszTempBuffer, g_regKeyNames[keyIndex], nDefault, L"entpack.ini");
  if ( PrivateProfileIntW > maxValue )
    PrivateProfileIntW = maxValue;
  if ( PrivateProfileIntW < minValue )
    PrivateProfileIntW = minValue;
  return PrivateProfileIntW;
}
DWORD WINAPI ReadINIStringValue(int keyIndex, LPWSTR lpReturnedString)
{
  return GetPrivateProfileStringW(wszTempBuffer, g_regKeyNames[keyIndex], wszDefaultString, lpReturnedString, 0x20u, L"entpack.ini");
}
LSTATUS InitGameSettings()
{
  unsigned short TickCount;
  int alreadyPlayed;
  LSTATUS result;
  HWND DesktopWindow;
  HDC DC;
  int DeviceCaps;
  DWORD dwDisposition;

  TickCount = (unsigned short)GetTickCount();
  srand(TickCount);
  LoadGameStringResource(1u, wszTempBuffer, 32);
  LoadGameStringResource(7u, wszFormatString, 32);
  LoadGameStringResource(8u, wszDefaultString, 32);
  nWindowTitleHeight = GetSystemMetrics(4) + 1;
  nMenuHeight = GetSystemMetrics(15) + 1;
  nWindowScrollHeight = GetSystemMetrics(6) + 1;
  nWindowBorderWidth = GetSystemMetrics(5) + 1;
  if ( RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\winmine", 0, 0, 0, 0x20019u, 0, &hRegKeyWinMine, &dwDisposition)
    || (alreadyPlayed = ReadRegValueWithRangeLimit(17, 0, 0, 1), result = RegCloseKey(hRegKeyWinMine), !alreadyPlayed) )
  {
    nMineFieldHeightConfig = ReadINIValueWithRangeLimit(2, 9, 9, 25);
    nMineFieldWidthConfig = ReadINIValueWithRangeLimit(3, 9, 9, 30);
    nDifficultyLevel = ReadINIValueWithRangeLimit(0, 0, 0, 3);
    nCurDifficultyMines = ReadINIValueWithRangeLimit(1, 10, 10, 999);
    nWindowPosX = ReadINIValueWithRangeLimit(4, 80, 0, 1024);
    nWindowPosY = ReadINIValueWithRangeLimit(5, 80, 0, 1024);
    nSoundState = ReadINIValueWithRangeLimit(6, 0, 0, 3);
    bMarkMode = ReadINIValueWithRangeLimit(7, 1, 0, 1);
    nTickMode = ReadINIValueWithRangeLimit(9, 0, 0, 1);
    nMenuDisplayState = ReadINIValueWithRangeLimit(8, 0, 0, 2);
    nBestTimeEasy = ReadINIValueWithRangeLimit(11, 999, 0, 999);
    nBestTimeMedium = ReadINIValueWithRangeLimit(13, 999, 0, 999);
    nBestTimeHard = ReadINIValueWithRangeLimit(15, 999, 0, 999);
    ReadINIStringValue(12, wszBestPlayerNameEasy);
    ReadINIStringValue(14, wszBestPlayerNameMedium);
    ReadINIStringValue(16, wszBestPlayerNameHard);
    DesktopWindow = GetDesktopWindow();
    DC = GetDC(DesktopWindow);
    DeviceCaps = GetDeviceCaps(DC, 24);
    bColorMode = ReadINIValueWithRangeLimit(10, DeviceCaps != 2, 0, 1);
    DesktopWindow = GetDesktopWindow();
    ReleaseDC(DesktopWindow, DC);
    if ( nSoundState == 3 )
      nSoundState = InitSoundPlayback();
    return SaveSettingsToRegistry();
  }
  return result;
}

DWORD WINAPI SetMenuItemCheckState(unsigned short menuId, int bChecked)
{
  return CheckMenuItem(hMainMenu, menuId, bChecked != 0 ? 8 : 0);
}
void WINAPI UpdateMenuDisplayState(int newState)
{
  nMenuDisplayState = newState;
  UpdateMenuCheckStates();
  SetMenu(hMainWnd, (nMenuDisplayState & 1) == 0 ? hMainMenu : 0);
  AdjustMainWindowPosAndSize(2);
}
INT ShowAboutDialog()
{
  HICON IconW;
  WCHAR szApp[128];
  WCHAR szOtherStuff[128];

  LoadGameStringResource(0xCu, szApp, 128);
  LoadGameStringResource(0xDu, szOtherStuff, 128);
  IconW = LoadIconW(hAppInstance, (LPCWSTR)0x64);
  return ShellAboutW(hMainWnd, szApp, szOtherStuff, IconW);
}
int WINAPI OpenHelpDocument(short helpType, int helpCommand)
{
  CHAR Filename[260];
  char *pTerminator;

  if ( helpType == 4 )
  {
    memcpy(Filename, "NTHelp.chm", 11);
    pTerminator = Filename + 10;
  }
  else
  {
    DWORD pathLen = GetModuleFileNameA(hAppInstance, Filename, 0xFAu);
    CHAR *pExt = &Filename[pathLen - 1];
    if ( (int)(pathLen - 253 + 252) > 4 && *(pExt - 3) == 46 )
      pExt -= 3;
    memcpy(pExt, ".chm", 4);
    pTerminator = pExt + 4;
  }
  *pTerminator = '\0';
  HWND DesktopWindow = GetDesktopWindow();
  return (int)CallHtmlHelpFunction((INT_PTR)DesktopWindow, (INT_PTR)Filename, helpCommand, 0);
}
int WINAPI GetDlgItemIntWithRangeLimit(HWND hDlg, int nIDDlgItem, int minValue, int maxValue)
{
  int result;
  result = GetDlgItemInt(hDlg, nIDDlgItem, &nIDDlgItem, 0);
  if ( result < minValue )
    return minValue;
  if ( result > maxValue )
    return maxValue;
  return result;
}
unsigned int SetFloatingPointControlWord()
{
    // 使用 Windows 的 _controlfp 或 _control87
#if defined(_MSC_VER)
    return _controlfp(0x10000u, 0x30000u);
#else
    return 0; // 或其他默认值
#endif
}
INT_PTR WINAPI CallHtmlHelpFunction(INT_PTR hwndCaller, INT_PTR helpPath, int command, INT_PTR data)
{
  HMODULE LibraryA;
  FARPROC ProcAddress;
  CHAR LibFileName[260];

  LibraryA = hHHCtrlModule;
  if ( hHHCtrlModule || bHHCtrlLoadFailed )
    goto LABEL_13;
  if ( GetHHCtrlOcxPath((LPBYTE)LibFileName) )
    hHHCtrlModule = LoadLibraryA(LibFileName);
  LibraryA = hHHCtrlModule;
  if ( hHHCtrlModule || (LibraryA = LoadLibraryA("hhctrl.ocx"), (hHHCtrlModule = LibraryA) != 0) )
  {
LABEL_13:
    ProcAddress = (FARPROC)pfnHtmlHelp;
    if ( pfnHtmlHelp )
      return ((INT_PTR (WINAPI *)(INT_PTR, INT_PTR, int, INT_PTR))ProcAddress)(hwndCaller, helpPath, command, data);
    ProcAddress = GetProcAddress(LibraryA, (LPCSTR)0xE);
    pfnHtmlHelp = (INT_PTR)ProcAddress;
    if ( ProcAddress )
      return ((INT_PTR (WINAPI *)(INT_PTR, INT_PTR, int, INT_PTR))ProcAddress)(hwndCaller, helpPath, command, data);
  }
  bHHCtrlLoadFailed = 1;
  return 0;
}
BOOL WINAPI GetHHCtrlOcxPath(LPBYTE lpData)
{
  BOOL querySuccess;
  DWORD cbData;
  HKEY phkResult;

  if ( RegOpenKeyExA(
         HKEY_CLASSES_ROOT,
         "CLSID\\{ADB880A6-D8FF-11CF-9377-00AA003B7A11}\\InprocServer32",
         0,
         0x20019u,
         &phkResult) )
  {
    return 0;
  }
  cbData = 260;
  querySuccess = RegQueryValueExA(phkResult, REG_DEFAULT_VALUE_NAME, 0, 0, lpData, &cbData) == 0;
  RegCloseKey(phkResult);
  return querySuccess;
}
INT_PTR WINAPI DialogFunc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, 100, nMineFieldWidthConfig, 0);
        SetDlgItemInt(hDlg, 101, nMineFieldHeightConfig, 0);
        SetDlgItemInt(hDlg, 102, nCurDifficultyMines, 0);
        return 1;

    case WM_COMMAND:
        if (!LOWORD(wParam))
            return 0;

        if (LOWORD(wParam) > 2u && LOWORD(wParam) != 100 && LOWORD(wParam) != 109)
            return 0;

        nMineFieldWidthConfig = GetDlgItemIntWithRangeLimit(hDlg, 100, 9, 30);
        nMineFieldHeightConfig = GetDlgItemIntWithRangeLimit(hDlg, 101, 9, 25);
        nCurDifficultyMines = GetDlgItemIntWithRangeLimit(hDlg, 102, 10, 999);

        if (nMineFieldWidthConfig * nMineFieldHeightConfig - 10 < nCurDifficultyMines)
        {
            nCurDifficultyMines = nMineFieldWidthConfig * nMineFieldHeightConfig - 10;
            SetDlgItemInt(hDlg, 102, nCurDifficultyMines, 0);
            SetFocus(GetDlgItem(hDlg, 102));
            SendMessageW(GetDlgItem(hDlg, 102), 0xB1u, 0, -1);
            return 1;
        }

        EndDialog(hDlg, 1);
        return 1;

    default:
        return 0;
    }
}


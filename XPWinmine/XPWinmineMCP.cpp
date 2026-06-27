#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include "Resource.h"
#include "XPWinmineMCP.h"

#define MCP_PIPE_NAME L"\\\\.\\pipe\\XPWinmineMCP"
#define MCP_BUFFER_SIZE 65536

#define TILE_DISPLAY_MASK 0x1F
#define MINE_CELL_MARK    0x80
#define MINE_CELL_FLAG    0x40
#define MINE_CELL_EXPLODED 0x20
#define GAME_STATUS_ACTIVE 0x01

#define TILE_EMPTY      0
#define TILE_FLAG       14
#define TILE_UNOPENED   15
#define SMILEY_NORMAL   0
#define SMILEY_WORRY    1
#define SMILEY_WIN      2
#define SMILEY_DEAD     3
#define SMILEY_PRESSED  4

extern char  arrMineFieldData[864];
extern int   nDifficultyLevel;
extern UINT  nCurDifficultyMines;
extern UINT  nMineFieldWidthConfig;
extern UINT  nMineFieldHeightConfig;
extern int   nMineFieldWidth;
extern int   nMineFieldHeight;
extern int   nGameTimerSeconds;
extern int   nTotalMines;
extern int   nRemainingMinesDisplay;
extern int   nTotalOpenedGrids;
extern int   nOpenedSafeGrids;
extern int   g_gameStatusArray[4];
extern int   nSmileyBtnState;
extern int   bTimerRunning;
extern HWND  hMainWnd;
extern HINSTANCE hAppInstance;
extern int   nMouseGridX;
extern int   nMouseGridY;
extern int   bMouseCaptured;
extern int   nMouseButtonState;
extern int   nBestTimeEasy;
extern int   nBestTimeMedium;
extern int   nBestTimeHard;
extern WCHAR wszBestPlayerNameEasy[32];
extern WCHAR wszBestPlayerNameMedium[32];
extern WCHAR wszBestPlayerNameHard[32];

int WINAPI AutoExpandBlankCells(int cellX, int cellY);
void WINAPI HandleRightClickOnCell(int cellX, int cellY);
void WINAPI HandleLeftClickOnCell(int cellX, int cellY);
void WINAPI HandleMiddleClickOnCell(int cellX, int cellY);
void WINAPI HandleGameOver(int bWin);
void ResetGame();
BOOL WINAPI SaveGameSnapshotToFile(LPCWSTR lpFilePath);
BOOL WINAPI LoadGameSnapshotFromFile(LPCWSTR lpFilePath);
BOOL WINAPI UndoGameState();
BOOL WINAPI RedoGameState();

static HANDLE g_hPipeThread = NULL;
static volatile BOOL g_bPipeRunning = FALSE;

static BOOL UTF8ToWide(const char* src, WCHAR* dst, int dstCount)
{
    if (!src || !dst || dstCount <= 0)
        return FALSE;

    int converted = MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, dstCount);
    return converted > 0;
}

static const char* GetGameStatusName(int status)
{
    if (status & GAME_STATUS_ACTIVE)
        return "running";
    if (status & 0x08)
        return "paused";
    if (status & 0x10)
        return "finished";
    return "idle";
}

static const char* GetSmileyName(int state)
{
    switch (state) {
        case 0:  return "normal";
        case 1:  return "worried";
        case 2:  return "dead";
        case 3:  return "win";
        case 4:  return "pressed";
        default: return "unknown";
    }
}

static int WriteJSONString(char* buf, int offset, const char* key, const char* value)
{
    return sprintf_s(buf + offset, MCP_BUFFER_SIZE - offset, "\"%s\":\"%s\"", key, value);
}

static int WriteJSONInt(char* buf, int offset, const char* key, int value)
{
    return sprintf_s(buf + offset, MCP_BUFFER_SIZE - offset, "\"%s\":%d", key, value);
}

static int WriteJSONBool(char* buf, int offset, const char* key, BOOL value)
{
    return sprintf_s(buf + offset, MCP_BUFFER_SIZE - offset, "\"%s\":%s", key, value ? "true" : "false");
}

static int WriteComma(char* buf, int offset)
{
    if (offset > 0 && buf[offset - 1] != '{' && buf[offset - 1] != '[')
        buf[offset++] = ',';
    return offset;
}

static int EscapeJSON(const char* src, char* dst, int dstSize)
{
    int j = 0;
    for (int i = 0; src[i] && j < dstSize - 2; i++) {
        switch (src[i]) {
            case '"':  dst[j++] = '\\'; dst[j++] = '"'; break;
            case '\\': dst[j++] = '\\'; dst[j++] = '\\'; break;
            case '\n': dst[j++] = '\\'; dst[j++] = 'n'; break;
            case '\r': dst[j++] = '\\'; dst[j++] = 'r'; break;
            case '\t': dst[j++] = '\\'; dst[j++] = 't'; break;
            default:   dst[j++] = src[i]; break;
        }
    }
    dst[j] = '\0';
    return j;
}

static int BuildBoardJSON(char* buf)
{
    int off = 0;
    off += sprintf_s(buf + off, MCP_BUFFER_SIZE - off, "{\"board\":[");

    for (int y = 1; y <= nMineFieldHeight; y++) {
        if (y > 1) buf[off++] = ',';
        buf[off++] = '[';

        for (int x = 1; x <= nMineFieldWidth; x++) {
            if (x > 1) buf[off++] = ',';
            char cell = arrMineFieldData[32 * y + x];
            int display = cell & TILE_DISPLAY_MASK;
            int hasMine = ((unsigned char)cell & MINE_CELL_MARK) ? 1 : 0;
            int isOpened = ((unsigned char)cell & MINE_CELL_FLAG) ? 1 : 0;
            int isExploded = ((unsigned char)cell & MINE_CELL_EXPLODED) ? 1 : 0;

            off += sprintf_s(buf + off, MCP_BUFFER_SIZE - off,
                "{\"d\":%d,\"m\":%d,\"o\":%d,\"x\":%d}",
                display, hasMine, isOpened, isExploded);
        }
        buf[off++] = ']';
    }
    buf[off++] = ']';
    buf[off++] = '}';
    buf[off] = '\0';
    return off;
}

static int HandleGetState(char* response)
{
    int off = 0;
    off += sprintf_s(response + off, MCP_BUFFER_SIZE - off, "{");

    int gameStatus = g_gameStatusArray[0];
    off = WriteComma(response, off);
    off += WriteJSONString(response, off, "game_status", GetGameStatusName(gameStatus));

    off = WriteComma(response, off);
    off += WriteJSONString(response, off, "smiley", GetSmileyName(nSmileyBtnState));

    off = WriteComma(response, off);
    off += WriteJSONInt(response, off, "difficulty", nDifficultyLevel);

    off = WriteComma(response, off);
    off += WriteJSONInt(response, off, "board_width", nMineFieldWidth);

    off = WriteComma(response, off);
    off += WriteJSONInt(response, off, "board_height", nMineFieldHeight);

    off = WriteComma(response, off);
    off += WriteJSONInt(response, off, "total_mines", nTotalMines);

    off = WriteComma(response, off);
    off += WriteJSONInt(response, off, "mines_remaining", nRemainingMinesDisplay);

    off = WriteComma(response, off);
    off += WriteJSONInt(response, off, "time_elapsed", nGameTimerSeconds);

    off = WriteComma(response, off);
    off += WriteJSONInt(response, off, "opened_cells", nTotalOpenedGrids);

    off = WriteComma(response, off);
    off += WriteJSONInt(response, off, "safe_cells_remaining", nOpenedSafeGrids);

    off = WriteComma(response, off);
    off += WriteJSONBool(response, off, "timer_running", bTimerRunning);

    off = WriteComma(response, off);
    off += WriteJSONInt(response, off, "best_time_beginner", nBestTimeEasy);
    off = WriteComma(response, off);
    off += WriteJSONInt(response, off, "best_time_intermediate", nBestTimeMedium);
    off = WriteComma(response, off);
    off += WriteJSONInt(response, off, "best_time_expert", nBestTimeHard);

    off += sprintf_s(response + off, MCP_BUFFER_SIZE - off, "}");
    return off;
}

static int HandleGetCell(int x, int y, char* response)
{
    if (x < 1 || x > nMineFieldWidth || y < 1 || y > nMineFieldHeight) {
        return sprintf_s(response, MCP_BUFFER_SIZE, "{\"error\":\"cell out of bounds\"}");
    }

    char cell = arrMineFieldData[32 * y + x];
    int display = cell & TILE_DISPLAY_MASK;
    int hasMine = ((unsigned char)cell & MINE_CELL_MARK) ? 1 : 0;
    int isOpened = ((unsigned char)cell & MINE_CELL_FLAG) ? 1 : 0;
    int isExploded = ((unsigned char)cell & MINE_CELL_EXPLODED) ? 1 : 0;

    int adjacentMines = 0;
    if (isOpened && !hasMine && display <= 8)
        adjacentMines = display;

    return sprintf_s(response, MCP_BUFFER_SIZE,
        "{\"x\":%d,\"y\":%d,\"display\":%d,\"has_mine\":%d,\"is_opened\":%d,"
        "\"is_exploded\":%d,\"adjacent_mines\":%d}",
        x, y, display, hasMine, isOpened, isExploded, adjacentMines);
}

static int HandleGetAdjacent(int x, int y, char* response)
{
    if (x < 1 || x > nMineFieldWidth || y < 1 || y > nMineFieldHeight) {
        return sprintf_s(response, MCP_BUFFER_SIZE, "{\"error\":\"cell out of bounds\"}");
    }

    int off = sprintf_s(response, MCP_BUFFER_SIZE, "{\"cells\":[");
    int first = 1;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight) continue;

            if (!first) response[off++] = ',';
            first = 0;

            char cell = arrMineFieldData[32 * ny + nx];
            int display = cell & TILE_DISPLAY_MASK;
            int hasMine = ((unsigned char)cell & MINE_CELL_MARK) ? 1 : 0;
            int isOpened = ((unsigned char)cell & MINE_CELL_FLAG) ? 1 : 0;

            off += sprintf_s(response + off, MCP_BUFFER_SIZE - off,
                "{\"x\":%d,\"y\":%d,\"display\":%d,\"has_mine\":%d,\"is_opened\":%d}",
                nx, ny, display, hasMine, isOpened);
        }
    }
    off += sprintf_s(response + off, MCP_BUFFER_SIZE - off, "]}");
    return off;
}

static int HandleGetBoard(char* response)
{
    return BuildBoardJSON(response);
}

static int HandleAnalyzeBoard(char* response)
{
    int off = sprintf_s(response, MCP_BUFFER_SIZE, "{\"safe_cells\":[");
    int safeFirst = 1;
    int safeCount = 0;

    int offMines = 0;
    char mineBuf[MCP_BUFFER_SIZE];
    mineBuf[0] = '\0';
    int mineFirst = 1;
    int mineCount = 0;

    for (int y = 1; y <= nMineFieldHeight; y++) {
        for (int x = 1; x <= nMineFieldWidth; x++) {
            char cell = arrMineFieldData[32 * y + x];
            int isOpened = ((unsigned char)cell & MINE_CELL_FLAG) ? 1 : 0;
            int display = cell & TILE_DISPLAY_MASK;

            if (!isOpened && display != TILE_FLAG) continue;
            if (!isOpened || display > 8) continue;

            int adjacentMines = display;
            int flaggedCount = 0;
            int unopenedCount = 0;
            int lastUnopenedX = 0, lastUnopenedY = 0;
            int lastUnopenedHasMine = 0;

            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = x + dx, ny = y + dy;
                    if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight) continue;
                    char nc = arrMineFieldData[32 * ny + nx];
                    int nd = nc & TILE_DISPLAY_MASK;
                    int no = ((unsigned char)nc & MINE_CELL_FLAG) ? 1 : 0;
                    int nm = ((unsigned char)nc & MINE_CELL_MARK) ? 1 : 0;
                    if (nd == TILE_FLAG) flaggedCount++;
                    if (!no && nd != TILE_FLAG) {
                        unopenedCount++;
                        lastUnopenedX = nx;
                        lastUnopenedY = ny;
                        lastUnopenedHasMine = nm;
                    }
                }
            }

            if (adjacentMines == flaggedCount && unopenedCount > 0) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx, ny = y + dy;
                        if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight) continue;
                        char nc = arrMineFieldData[32 * ny + nx];
                        int nd = nc & TILE_DISPLAY_MASK;
                        int no = ((unsigned char)nc & MINE_CELL_FLAG) ? 1 : 0;
                        if (!no && nd != TILE_FLAG) {
                            if (!safeFirst) response[off++] = ',';
                            safeFirst = 0;
                            off += sprintf_s(response + off, MCP_BUFFER_SIZE - off,
                                "{\"x\":%d,\"y\":%d}", nx, ny);
                            safeCount++;
                        }
                    }
                }
            }
        }
    }
    off += sprintf_s(response + off, MCP_BUFFER_SIZE - off, "],\"mine_cells\":[");

    for (int y = 1; y <= nMineFieldHeight; y++) {
        for (int x = 1; x <= nMineFieldWidth; x++) {
            char cell = arrMineFieldData[32 * y + x];
            int isOpened = ((unsigned char)cell & MINE_CELL_FLAG) ? 1 : 0;
            int display = cell & TILE_DISPLAY_MASK;

            if (!isOpened || display > 8) continue;

            int adjacentMines = display;
            int unopenedCount = 0;
            int lastUX = 0, lastUY = 0;

            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = x + dx, ny = y + dy;
                    if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight) continue;
                    char nc = arrMineFieldData[32 * ny + nx];
                    int nd = nc & TILE_DISPLAY_MASK;
                    int no = ((unsigned char)nc & MINE_CELL_FLAG) ? 1 : 0;
                    if (!no && nd != TILE_FLAG) {
                        unopenedCount++;
                        lastUX = nx; lastUY = ny;
                    }
                }
            }

            if (adjacentMines > 0 && unopenedCount == adjacentMines) {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx, ny = y + dy;
                        if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight) continue;
                        char nc = arrMineFieldData[32 * ny + nx];
                        int nd = nc & TILE_DISPLAY_MASK;
                        int no = ((unsigned char)nc & MINE_CELL_FLAG) ? 1 : 0;
                        if (!no && nd != TILE_FLAG) {
                            if (!mineFirst) {
                                int mOff = (int)strlen(mineBuf);
                                mineBuf[mOff++] = ',';
                                mineBuf[mOff] = '\0';
                            }
                            mineFirst = 0;
                            int mOff = (int)strlen(mineBuf);
                            mOff += sprintf_s(mineBuf + mOff, MCP_BUFFER_SIZE - mOff,
                                "{\"x\":%d,\"y\":%d}", nx, ny);
                            mineCount++;
                        }
                    }
                }
            }
        }
    }

    off += sprintf_s(response + off, MCP_BUFFER_SIZE - off, "%s", mineBuf);
    off += sprintf_s(response + off, MCP_BUFFER_SIZE - off,
        "],\"safe_count\":%d,\"mine_count\":%d}", safeCount, mineCount);
    return off;
}

static void SimulateClick(int x, int y, int button)
{
    if (!hMainWnd) return;

    int cellSize = 16;
    int boardLeft = 12;
    int boardTop = 55;
    int screenX = boardLeft + (x - 1) * cellSize + cellSize / 2;
    int screenY = boardTop + (y - 1) * cellSize + cellSize / 2;
    LPARAM lParam = MAKELPARAM(screenX, screenY);

    SetForegroundWindow(hMainWnd);
    PostMessageW(hMainWnd, WM_ACTIVATE, WA_ACTIVE, 0);
    Sleep(30);

    if (button == 0) {
        PostMessageW(hMainWnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
        Sleep(50);
        PostMessageW(hMainWnd, WM_LBUTTONUP, 0, lParam);
    } else if (button == 1) {
        PostMessageW(hMainWnd, WM_RBUTTONDOWN, MK_RBUTTON, lParam);
        Sleep(50);
        PostMessageW(hMainWnd, WM_RBUTTONUP, 0, lParam);
    } else if (button == 2) {
        PostMessageW(hMainWnd, WM_MBUTTONDOWN, MK_MBUTTON, lParam);
        Sleep(50);
        PostMessageW(hMainWnd, WM_MBUTTONUP, 0, lParam);
    }
}

static void HandleReset()
{
    if (!hMainWnd) return;
    int cellSize = 16;
    int faceX = (nMineFieldWidth * cellSize) / 2 + 12 - 12;
    int faceY = 28;
    LPARAM lParam = MAKELPARAM(faceX, faceY);
    PostMessageW(hMainWnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
    Sleep(50);
    PostMessageW(hMainWnd, WM_LBUTTONUP, 0, lParam);
}

static void HandleSetDifficulty(int level)
{
    if (!hMainWnd) return;
    if (level < 0 || level > 3) return;

    WORD cmd;
    switch (level) {
        case 0: cmd = 521; break;
        case 1: cmd = 522; break;
        case 2: cmd = 523; break;
        case 3: cmd = 524; break;
        default: return;
    }
    PostMessageW(hMainWnd, WM_COMMAND, cmd, 0);
}

static BOOL ProcessCommand(const char* cmd, char* response, int responseSize)
{
    response[0] = '\0';

    if (strcmp(cmd, "PING") == 0) {
        sprintf_s(response, responseSize, "{\"status\":\"ok\"}");
        return TRUE;
    }

    if (strcmp(cmd, "GET_STATE") == 0) {
        HandleGetState(response);
        return TRUE;
    }

    if (strcmp(cmd, "GET_BOARD") == 0) {
        HandleGetBoard(response);
        return TRUE;
    }

    if (strcmp(cmd, "ANALYZE") == 0) {
        HandleAnalyzeBoard(response);
        return TRUE;
    }

    if (strncmp(cmd, "GET_CELL ", 9) == 0) {
        int x = 0, y = 0;
        if (sscanf_s(cmd + 9, "%d %d", &x, &y) == 2) {
            HandleGetCell(x, y, response);
        } else {
            sprintf_s(response, responseSize, "{\"error\":\"usage: GET_CELL x y\"}");
        }
        return TRUE;
    }

    if (strncmp(cmd, "GET_ADJACENT ", 13) == 0) {
        int x = 0, y = 0;
        if (sscanf_s(cmd + 13, "%d %d", &x, &y) == 2) {
            HandleGetAdjacent(x, y, response);
        } else {
            sprintf_s(response, responseSize, "{\"error\":\"usage: GET_ADJACENT x y\"}");
        }
        return TRUE;
    }

    if (strncmp(cmd, "CLICK ", 6) == 0) {
        int x = 0, y = 0;
        if (sscanf_s(cmd + 6, "%d %d", &x, &y) == 2) {
            SimulateClick(x, y, 0);
            sprintf_s(response, responseSize,
                "{\"status\":\"ok\",\"action\":\"click\",\"x\":%d,\"y\":%d}", x, y);
        } else {
            sprintf_s(response, responseSize, "{\"error\":\"usage: CLICK x y\"}");
        }
        return TRUE;
    }

    if (strncmp(cmd, "RCLICK ", 7) == 0) {
        int x = 0, y = 0;
        if (sscanf_s(cmd + 7, "%d %d", &x, &y) == 2) {
            SimulateClick(x, y, 1);
            sprintf_s(response, responseSize,
                "{\"status\":\"ok\",\"action\":\"rclick\",\"x\":%d,\"y\":%d}", x, y);
        } else {
            sprintf_s(response, responseSize, "{\"error\":\"usage: RCLICK x y\"}");
        }
        return TRUE;
    }

    if (strncmp(cmd, "MCLICK ", 7) == 0) {
        int x = 0, y = 0;
        if (sscanf_s(cmd + 7, "%d %d", &x, &y) == 2) {
            SimulateClick(x, y, 2);
            sprintf_s(response, responseSize,
                "{\"status\":\"ok\",\"action\":\"mclick\",\"x\":%d,\"y\":%d}", x, y);
        } else {
            sprintf_s(response, responseSize, "{\"error\":\"usage: MCLICK x y\"}");
        }
        return TRUE;
    }

    if (strncmp(cmd, "OPEN ", 5) == 0) {
        int x = 0, y = 0;
        if (sscanf_s(cmd + 5, "%d %d", &x, &y) == 2) {
            if ((g_gameStatusArray[0] & GAME_STATUS_ACTIVE) != 0) {
                if (!nTotalOpenedGrids && !nGameTimerSeconds) {
                    nGameTimerSeconds = 1;
                    bTimerRunning = 1;
                    SetTimer(hMainWnd, 1, 1000, NULL);
                }
                HandleLeftClickOnCell(x, y);
            }
            sprintf_s(response, responseSize,
                "{\"status\":\"ok\",\"action\":\"open\",\"x\":%d,\"y\":%d}", x, y);
        } else {
            sprintf_s(response, responseSize, "{\"error\":\"usage: OPEN x y\"}");
        }
        return TRUE;
    }

    if (strncmp(cmd, "FLAG ", 5) == 0) {
        int x = 0, y = 0;
        if (sscanf_s(cmd + 5, "%d %d", &x, &y) == 2) {
            if (x >= 1 && x <= nMineFieldWidth && y >= 1 && y <= nMineFieldHeight) {
                Sleep(500);
                HandleRightClickOnCell(x, y);
            }
            sprintf_s(response, responseSize,
                "{\"status\":\"ok\",\"action\":\"flag\",\"x\":%d,\"y\":%d}", x, y);
        } else {
            sprintf_s(response, responseSize, "{\"error\":\"usage: FLAG x y\"}");
        }
        return TRUE;
    }

    if (strncmp(cmd, "CHORD ", 6) == 0) {
        int x = 0, y = 0;
        if (sscanf_s(cmd + 6, "%d %d", &x, &y) == 2) {
            if (x >= 1 && x <= nMineFieldWidth && y >= 1 && y <= nMineFieldHeight) {
                HandleMiddleClickOnCell(x, y);
            }
            sprintf_s(response, responseSize,
                "{\"status\":\"ok\",\"action\":\"chord\",\"x\":%d,\"y\":%d}", x, y);
        } else {
            sprintf_s(response, responseSize, "{\"error\":\"usage: CHORD x y\"}");
        }
        return TRUE;
    }

    if (strcmp(cmd, "UNDO") == 0) {
        sprintf_s(response, responseSize,
            "{\"status\":\"%s\",\"action\":\"undo\"}", UndoGameState() ? "ok" : "noop");
        return TRUE;
    }

    if (strcmp(cmd, "REDO") == 0) {
        sprintf_s(response, responseSize,
            "{\"status\":\"%s\",\"action\":\"redo\"}", RedoGameState() ? "ok" : "noop");
        return TRUE;
    }

    if (strncmp(cmd, "SAVE_SNAPSHOT ", 14) == 0) {
        WCHAR path[MAX_PATH];
        if (!UTF8ToWide(cmd + 14, path, MAX_PATH)) {
            sprintf_s(response, responseSize, "{\"error\":\"invalid utf8 path\"}");
            return TRUE;
        }

        if (SaveGameSnapshotToFile(path)) {
            char escaped[MAX_PATH * 2];
            EscapeJSON(cmd + 14, escaped, sizeof(escaped));
            sprintf_s(response, responseSize,
                "{\"status\":\"ok\",\"action\":\"save_snapshot\",\"path\":\"%s\"}", escaped);
        } else {
            sprintf_s(response, responseSize, "{\"error\":\"save snapshot failed\"}");
        }
        return TRUE;
    }

    if (strncmp(cmd, "LOAD_SNAPSHOT ", 14) == 0) {
        WCHAR path[MAX_PATH];
        if (!UTF8ToWide(cmd + 14, path, MAX_PATH)) {
            sprintf_s(response, responseSize, "{\"error\":\"invalid utf8 path\"}");
            return TRUE;
        }

        if (LoadGameSnapshotFromFile(path)) {
            char escaped[MAX_PATH * 2];
            EscapeJSON(cmd + 14, escaped, sizeof(escaped));
            sprintf_s(response, responseSize,
                "{\"status\":\"ok\",\"action\":\"load_snapshot\",\"path\":\"%s\"}", escaped);
        } else {
            sprintf_s(response, responseSize, "{\"error\":\"load snapshot failed\"}");
        }
        return TRUE;
    }

    if (strncmp(cmd, "AUTOPLAY ", 9) == 0) {
        int maxSteps = 0, speedMs = 0;
        if (sscanf_s(cmd + 9, "%d %d", &maxSteps, &speedMs) < 1)
            maxSteps = 200;
        if (maxSteps <= 0) maxSteps = 200;
        if (speedMs <= 0) speedMs = 0;

        int off = sprintf_s(response, responseSize,
            "{\"status\":\"ok\",\"action\":\"autoplay\",\"steps\":[");

        int stepCount = 0, stepFirst = 1;

        #define MAX_UNK 24
        while (stepCount < maxSteps && (g_gameStatusArray[0] & GAME_STATUS_ACTIVE) != 0) {
            int changed = 0;
            int unkX[MAX_UNK], unkY[MAX_UNK], unkCnt = 0;
            struct { int x, y, mines, cnt; int cx[8], cy[8]; } cons[400];
            int nCons = 0;

            // Build constraints from board
            for (int y = 1; y <= nMineFieldHeight; y++) {
                for (int x = 1; x <= nMineFieldWidth; x++) {
                    char c = arrMineFieldData[32 * y + x];
                    if (!(((unsigned char)c & MINE_CELL_FLAG))) continue;
                    int nd = c & TILE_DISPLAY_MASK;
                    if (nd > 8) continue;
                    int flags = 0; int cx[8], cy[8], cu = 0;
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            int nx = x + dx, ny = y + dy;
                            if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight) continue;
                            char nc = arrMineFieldData[32 * ny + nx];
                            if ((nc & TILE_DISPLAY_MASK) == TILE_FLAG) { flags++; continue; }
                            if (((unsigned char)nc & MINE_CELL_FLAG)) continue;
                            if (cu < 8) { cx[cu] = nx; cy[cu] = ny; cu++; }
                        }
                    }
                    int rem = nd - flags;
                    if (cu > 0 && nCons < 400) {
                        for (int k = 0; k < cu; k++) {
                            int seen = 0;
                            for (int u = 0; u < unkCnt; u++)
                                if (unkX[u] == cx[k] && unkY[u] == cy[k]) { seen = 1; break; }
                            if (!seen && unkCnt < MAX_UNK) { unkX[unkCnt] = cx[k]; unkY[unkCnt] = cy[k]; unkCnt++; }
                        }
                        cons[nCons].x = x; cons[nCons].y = y; cons[nCons].mines = rem; cons[nCons].cnt = cu;
                        for (int k = 0; k < cu; k++) { cons[nCons].cx[k] = cx[k]; cons[nCons].cy[k] = cy[k]; }
                        nCons++;
                    }
                }
            }

            // Step 1: Open definitely safe cells (remaining mines == 0)
            for (int i = 0; i < nCons; i++) {
                if (cons[i].mines == 0) {
                    for (int k = 0; k < cons[i].cnt; k++) {
                        int nx = cons[i].cx[k], ny = cons[i].cy[k];
                        if ((arrMineFieldData[32 * ny + nx] & TILE_DISPLAY_MASK) == TILE_UNOPENED) {
                            HandleLeftClickOnCell(nx, ny);
                            if (!stepFirst) response[off++] = ',';
                            stepFirst = 0;
                            off += sprintf_s(response + off, responseSize - off, "{\"t\":\"open\",\"x\":%d,\"y\":%d}", nx, ny);
                            changed = 1; stepCount++;
                            if (speedMs) Sleep(speedMs);
                        }
                    }
                }
            }
            if (changed || stepCount >= maxSteps || !((g_gameStatusArray[0] & GAME_STATUS_ACTIVE))) {
                if (!changed || stepCount >= maxSteps) break;
                continue;
            }

            // Step 2: Backtracking enumeration
            if (unkCnt > 0 && unkCnt <= MAX_UNK) {
                int totalValid = 0, mineCnt[MAX_UNK] = {0};
                unsigned long long maxMask = 1ULL << unkCnt;
                for (unsigned long long mask = 0; mask < maxMask; mask++) {
                    int ok = 1;
                    for (int ci = 0; ci < nCons && ok; ci++) {
                        int found = 0;
                        for (int k = 0; k < cons[ci].cnt; k++)
                            for (int u = 0; u < unkCnt; u++)
                                if (unkX[u] == cons[ci].cx[k] && unkY[u] == cons[ci].cy[k])
                                    { if (mask & (1ULL << u)) found++; break; }
                        if (found != cons[ci].mines) ok = 0;
                    }
                    if (ok) { totalValid++; for (int u = 0; u < unkCnt; u++) if (mask & (1ULL << u)) mineCnt[u]++; }
                }

                if (totalValid > 0) {
                    // Find 0% cells (definitely safe) 鈫?open
                    for (int u = 0; u < unkCnt; u++) {
                        if (mineCnt[u] == 0 && (arrMineFieldData[32 * unkY[u] + unkX[u]] & TILE_DISPLAY_MASK) == TILE_UNOPENED) {
                            HandleLeftClickOnCell(unkX[u], unkY[u]);
                            if (!stepFirst) response[off++] = ',';
                            stepFirst = 0;
                            off += sprintf_s(response + off, responseSize - off, "{\"t\":\"safe\",\"x\":%d,\"y\":%d}", unkX[u], unkY[u]);
                            changed = 1; stepCount++;
                            if (speedMs) Sleep(speedMs);
                        }
                    }

                    // Find 100% cells (definitely mines) 鈫?flag
                    if (!changed) {
                        for (int u = 0; u < unkCnt; u++) {
                            if (mineCnt[u] == totalValid && (arrMineFieldData[32 * unkY[u] + unkX[u]] & TILE_DISPLAY_MASK) == TILE_UNOPENED) {
                                HandleRightClickOnCell(unkX[u], unkY[u]);
                                if (!stepFirst) response[off++] = ',';
                                stepFirst = 0;
                                off += sprintf_s(response + off, responseSize - off, "{\"t\":\"flag\",\"x\":%d,\"y\":%d}", unkX[u], unkY[u]);
                                changed = 1; stepCount++;
                                Sleep(500);
                            }
                        }
                    }
                }
            }

            // Step 3: If still no progress, guess safest cell
            if (!changed && unkCnt > 0 && unkCnt <= MAX_UNK) {
                int bestX = 0, bestY = 0; float bestP = 2.0f;
                int totalValid = 0, mineCnt[MAX_UNK] = {0};
                unsigned long long maxMask = 1ULL << unkCnt;
                for (unsigned long long mask = 0; mask < maxMask; mask++) {
                    int ok = 1;
                    for (int ci = 0; ci < nCons && ok; ci++) {
                        int found = 0;
                        for (int k = 0; k < cons[ci].cnt; k++)
                            for (int u = 0; u < unkCnt; u++)
                                if (unkX[u] == cons[ci].cx[k] && unkY[u] == cons[ci].cy[k])
                                    { if (mask & (1ULL << u)) found++; break; }
                        if (found != cons[ci].mines) ok = 0;
                    }
                    if (ok) { totalValid++; for (int u = 0; u < unkCnt; u++) if (mask & (1ULL << u)) mineCnt[u]++; }
                }
                if (totalValid > 0) {
                    for (int u = 0; u < unkCnt; u++) {
                        float p = (float)mineCnt[u] / totalValid;
                        if (p < bestP && (arrMineFieldData[32 * unkY[u] + unkX[u]] & TILE_DISPLAY_MASK) == TILE_UNOPENED)
                            { bestP = p; bestX = unkX[u]; bestY = unkY[u]; }
                    }
                }
                if (bestX > 0) {
                    HandleLeftClickOnCell(bestX, bestY);
                    if (!stepFirst) response[off++] = ',';
                    stepFirst = 0;
                    off += sprintf_s(response + off, responseSize - off, "{\"t\":\"guess\",\"x\":%d,\"y\":%d,\"p\":%.2f}", bestX, bestY, bestP);
                    changed = 1; stepCount++;
                    if (speedMs) Sleep(speedMs);
                }
            }

            // No constraints at all 鈫?click any unopened cell
            if (!changed && unkCnt == 0) {
                for (int y = 1; y <= nMineFieldHeight; y++) {
                    for (int x = 1; x <= nMineFieldWidth; x++) {
                        char c = arrMineFieldData[32 * y + x];
                        if (!(((unsigned char)c & MINE_CELL_FLAG)) && (c & TILE_DISPLAY_MASK) != TILE_FLAG) {
                            HandleLeftClickOnCell(x, y);
                            if (!stepFirst) response[off++] = ',';
                            stepFirst = 0;
                            off += sprintf_s(response + off, responseSize - off, "{\"t\":\"open\",\"x\":%d,\"y\":%d}", x, y);
                            changed = 1; stepCount++;
                            if (speedMs) Sleep(speedMs);
                            break;
                        }
                    }
                    if (changed) break;
                }
            }

            if (!changed) break;
        }
        #undef MAX_UNK

        int won = (nSmileyBtnState == 3) ? 1 : 0;

        int openSteps = 0, flagSteps = 0, guessSteps = 0, safeSteps = 0;
        const char* stepPtr = response;
        while (*stepPtr) {
            if (strncmp(stepPtr, "{\"t\":\"open\"", 10) == 0) openSteps++;
            else if (strncmp(stepPtr, "{\"t\":\"flag\"", 10) == 0) flagSteps++;
            else if (strncmp(stepPtr, "{\"t\":\"guess\"", 11) == 0) guessSteps++;
            else if (strncmp(stepPtr, "{\"t\":\"safe\"", 10) == 0) safeSteps++;
            stepPtr = strchr(stepPtr + 1, '{');
            if (!stepPtr) break;
        }

        off += sprintf_s(response + off, responseSize - off,
            "],\"total_steps\":%d,\"won\":%d,"
            "\"open\":%d,\"flag\":%d,\"guess\":%d,\"safe\":%d,"
            "\"time\":%d,\"opened\":%d,\"mines_left\":%d,"
            "\"final_state\":{"
            "\"status\":\"%s\",\"smiley\":\"%s\",\"difficulty\":%d,"
            "\"board_width\":%d,\"board_height\":%d,"
            "\"total_mines\":%d,\"mines_remaining\":%d,"
            "\"opened_cells\":%d,\"safe_cells_remaining\":%d}",
            stepCount, won, openSteps, flagSteps, guessSteps, safeSteps,
            nGameTimerSeconds, nTotalOpenedGrids, nRemainingMinesDisplay,
            (won ? "won" : "lost"), GetSmileyName(nSmileyBtnState), nDifficultyLevel,
            nMineFieldWidth, nMineFieldHeight,
            nTotalMines, nRemainingMinesDisplay,
            nTotalOpenedGrids, nOpenedSafeGrids);
        off += sprintf_s(response + off, responseSize - off, "}");

        return TRUE;
    }

    if (strcmp(cmd, "RESET") == 0) {
        ResetGame();
        sprintf_s(response, responseSize, "{\"status\":\"ok\",\"action\":\"reset\"}");
        return TRUE;
    }

    if (strncmp(cmd, "SET_DIFFICULTY ", 15) == 0) {
        int level = atoi(cmd + 15);
        HandleSetDifficulty(level);
        sprintf_s(response, responseSize,
            "{\"status\":\"ok\",\"action\":\"set_difficulty\",\"level\":%d}", level);
        return TRUE;
    }

    sprintf_s(response, responseSize, "{\"error\":\"unknown command: %s\"}", cmd);
    return TRUE;
}

static unsigned __stdcall PipeServerThread(void* lpParam)
{
    HWND hWnd = (HWND)lpParam;

    while (g_bPipeRunning) {
        HANDLE hPipe = CreateNamedPipeW(
            MCP_PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1,
            MCP_BUFFER_SIZE,
            MCP_BUFFER_SIZE,
            0,
            NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            Sleep(1000);
            continue;
        }

        BOOL bConnected = ConnectNamedPipe(hPipe, NULL);
        if (!bConnected && GetLastError() != ERROR_PIPE_CONNECTED) {
            CloseHandle(hPipe);
            Sleep(100);
            continue;
        }

        char cmdBuffer[4096];
        char* responseBuffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MCP_BUFFER_SIZE);

        if (!responseBuffer) {
            CloseHandle(hPipe);
            continue;
        }

        while (g_bPipeRunning) {
            DWORD bytesRead = 0;
            BOOL bRead = ReadFile(hPipe, cmdBuffer, sizeof(cmdBuffer) - 1, &bytesRead, NULL);
            if (!bRead || bytesRead == 0) break;

            cmdBuffer[bytesRead] = '\0';

            char* end = strchr(cmdBuffer, '\n');
            if (end) *end = '\0';
            end = strchr(cmdBuffer, '\r');
            if (end) *end = '\0';

            if (strlen(cmdBuffer) == 0) continue;

            ProcessCommand(cmdBuffer, responseBuffer, MCP_BUFFER_SIZE);

            size_t respLen = strlen(responseBuffer);
            if (respLen + 1 < MCP_BUFFER_SIZE) {
                responseBuffer[respLen] = '\n';
                responseBuffer[respLen + 1] = '\0';
            }

            DWORD bytesWritten = 0;
            WriteFile(hPipe, responseBuffer, (DWORD)(respLen + 1), &bytesWritten, NULL);
        }

        HeapFree(GetProcessHeap(), 0, responseBuffer);

        FlushFileBuffers(hPipe);
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }

    return 0;
}

BOOL StartMCPPipeServer(HWND hWnd, HINSTANCE hInst)
{
    if (g_bPipeRunning) return TRUE;

    g_bPipeRunning = TRUE;
    g_hPipeThread = (HANDLE)_beginthreadex(NULL, 0, PipeServerThread, (void*)hWnd, 0, NULL);

    return g_hPipeThread != NULL;
}

void StopMCPPipeServer()
{
    g_bPipeRunning = FALSE;

    HANDLE hPipe = CreateFileW(MCP_PIPE_NAME, GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe);
    }

    if (g_hPipeThread) {
        WaitForSingleObject(g_hPipeThread, 3000);
        CloseHandle(g_hPipeThread);
        g_hPipeThread = NULL;
    }
}

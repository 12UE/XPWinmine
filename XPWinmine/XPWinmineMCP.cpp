#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <math.h>
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

static float ClampProbability(float value)
{
    if (value < 0.0f)
        return 0.0f;
    if (value > 1.0f)
        return 1.0f;
    return value;
}

static int CountAdjacentCells(int x, int y)
{
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0)
                continue;

            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 1 && nx <= nMineFieldWidth && ny >= 1 && ny <= nMineFieldHeight)
                count++;
        }
    }
    return count;
}

static int CountAdjacentOpenedNumbers(int x, int y)
{
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0)
                continue;

            int nx = x + dx;
            int ny = y + dy;
            if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight)
                continue;

            char cell = arrMineFieldData[32 * ny + nx];
            if (((unsigned char)cell & MINE_CELL_FLAG) && (cell & TILE_DISPLAY_MASK) >= 1 && (cell & TILE_DISPLAY_MASK) <= 8)
                count++;
        }
    }
    return count;
}

static int FindUnknownIndex(const int* unkX, const int* unkY, int unkCnt, int x, int y)
{
    for (int i = 0; i < unkCnt; i++) {
        if (unkX[i] == x && unkY[i] == y)
            return i;
    }
    return -1;
}

static float EstimateLocalMineRisk(int x, int y, float fallbackRisk)
{
    float risk = fallbackRisk;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0)
                continue;

            int nx = x + dx;
            int ny = y + dy;
            if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight)
                continue;

            char numberCell = arrMineFieldData[32 * ny + nx];
            if (!(((unsigned char)numberCell & MINE_CELL_FLAG)))
                continue;

            int display = numberCell & TILE_DISPLAY_MASK;
            if (display < 1 || display > 8)
                continue;

            int flags = 0;
            int unknownCount = 0;
            for (int ndy = -1; ndy <= 1; ndy++) {
                for (int ndx = -1; ndx <= 1; ndx++) {
                    if (ndx == 0 && ndy == 0)
                        continue;

                    int sx = nx + ndx;
                    int sy = ny + ndy;
                    if (sx < 1 || sx > nMineFieldWidth || sy < 1 || sy > nMineFieldHeight)
                        continue;

                    char sideCell = arrMineFieldData[32 * sy + sx];
                    if ((sideCell & TILE_DISPLAY_MASK) == TILE_FLAG)
                        flags++;
                    else if (!(((unsigned char)sideCell & MINE_CELL_FLAG)))
                        unknownCount++;
                }
            }

            if (unknownCount > 0) {
                float localRisk = (float)(display - flags) / (float)unknownCount;
                if (localRisk > risk)
                    risk = localRisk;
            }
        }
    }

    return ClampProbability(risk);
}

static BOOL SelectBestGuess(const int* unkX, const int* unkY, int unkCnt,
    const float* exactProb, const int* hasExactProb,
    int* outX, int* outY, float* outRisk)
{
    int hiddenCount = 0;
    int exactCount = 0;
    float exactMineExpectation = 0.0f;

    for (int y = 1; y <= nMineFieldHeight; y++) {
        for (int x = 1; x <= nMineFieldWidth; x++) {
            char cell = arrMineFieldData[32 * y + x];
            if (((unsigned char)cell & MINE_CELL_FLAG) || (cell & TILE_DISPLAY_MASK) == TILE_FLAG)
                continue;
            hiddenCount++;
        }
    }

    for (int i = 0; i < unkCnt; i++) {
        if (!hasExactProb[i])
            continue;

        char cell = arrMineFieldData[32 * unkY[i] + unkX[i]];
        if (((unsigned char)cell & MINE_CELL_FLAG) || (cell & TILE_DISPLAY_MASK) == TILE_FLAG)
            continue;

        exactCount++;
        exactMineExpectation += exactProb[i];
    }

    int residualCells = hiddenCount - exactCount;
    float residualRisk = 1.0f;
    if (residualCells > 0)
        residualRisk = ClampProbability((float)(nRemainingMinesDisplay - exactMineExpectation) / (float)residualCells);

    int bestX = 0;
    int bestY = 0;
    float bestRisk = 2.0f;
    int bestInfo = -1;
    int bestAdjacency = 9;
    int bestCenterDistance = -1;

    for (int y = 1; y <= nMineFieldHeight; y++) {
        for (int x = 1; x <= nMineFieldWidth; x++) {
            char cell = arrMineFieldData[32 * y + x];
            if (((unsigned char)cell & MINE_CELL_FLAG) || (cell & TILE_DISPLAY_MASK) == TILE_FLAG)
                continue;

            int u = FindUnknownIndex(unkX, unkY, unkCnt, x, y);
            float risk = (u >= 0 && hasExactProb[u])
                ? exactProb[u]
                : EstimateLocalMineRisk(x, y, residualRisk);

            int info = CountAdjacentOpenedNumbers(x, y);
            int adjacency = CountAdjacentCells(x, y);
            int centerDistance = abs(x * 2 - (nMineFieldWidth + 1)) + abs(y * 2 - (nMineFieldHeight + 1));

            int better = (risk < bestRisk - 0.005f);
            if (!better && fabs(risk - bestRisk) < 0.005f) {
                if (info != bestInfo)
                    better = info > bestInfo;
                else if (adjacency != bestAdjacency)
                    better = adjacency > bestAdjacency;
                else if (centerDistance != bestCenterDistance)
                    better = centerDistance < bestCenterDistance;
            }

            if (better) {
                bestX = x;
                bestY = y;
                bestRisk = risk;
                bestInfo = info;
                bestAdjacency = adjacency;
                bestCenterDistance = centerDistance;
            }
        }
    }

    if (bestX <= 0 || bestY <= 0)
        return FALSE;

    *outX = bestX;
    *outY = bestY;
    *outRisk = ClampProbability(bestRisk);
    return TRUE;
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

        if (!(g_gameStatusArray[0] & GAME_STATUS_ACTIVE)) {
            ResetGame();
        }

        int stepCount = 0, stepFirst = 1;

        #define MAX_UNK 64
        #define MAX_CONS 400
        while (stepCount < maxSteps && (g_gameStatusArray[0] & GAME_STATUS_ACTIVE) != 0) {
            int changed = 0;
            int unkX[MAX_UNK], unkY[MAX_UNK], unkCnt = 0;
            struct Cons { int x, y, mines, cnt; int cx[8], cy[8]; } cons[MAX_CONS];
            int nCons = 0;

            // ── Build constraints ──
            for (int y = 1; y <= nMineFieldHeight; y++) {
                for (int x = 1; x <= nMineFieldWidth; x++) {
                    char c = arrMineFieldData[32 * y + x];
                    if (!(((unsigned char)c & MINE_CELL_FLAG))) continue;
                    int nd = c & 0x1F;
                    if (nd > 8) continue;
                    int flags = 0; int cx[8], cy[8], cu = 0;
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            int nx = x + dx, ny = y + dy;
                            if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight) continue;
                            char nc = arrMineFieldData[32 * ny + nx];
                            if ((nc & 0x1F) == TILE_FLAG) { flags++; continue; }
                            if (((unsigned char)nc & MINE_CELL_FLAG)) continue;
                            if (cu < 8) { cx[cu] = nx; cy[cu] = ny; cu++; }
                        }
                    }
                    int rem = nd - flags;
                    if (cu > 0 && nCons < MAX_CONS) {
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

            // ── Step 1: mines==0 → open all neighbors ──
            for (int i = 0; i < nCons; i++) {
                if (cons[i].mines == 0) {
                    for (int k = 0; k < cons[i].cnt; k++) {
                        int nx = cons[i].cx[k], ny = cons[i].cy[k];
                        if ((arrMineFieldData[32 * ny + nx] & 0x1F) == TILE_UNOPENED) {
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
            if (changed) continue;

            // ── Step 1.5: mines == count → flag all ──
            for (int i = 0; i < nCons; i++) {
                if (cons[i].mines == cons[i].cnt) {
                    for (int k = 0; k < cons[i].cnt; k++) {
                        int nx = cons[i].cx[k], ny = cons[i].cy[k];
                        if ((arrMineFieldData[32 * ny + nx] & 0x1F) == TILE_UNOPENED) {
                            HandleRightClickOnCell(nx, ny);
                            if (!stepFirst) response[off++] = ',';
                            stepFirst = 0;
                            off += sprintf_s(response + off, responseSize - off, "{\"t\":\"flag\",\"x\":%d,\"y\":%d}", nx, ny);
                            changed = 1; stepCount++;
                            if (speedMs) Sleep(speedMs);
                        }
                    }
                }
            }
            if (changed) continue;

            // ── Step 2: Chord ──
            for (int y = 1; y <= nMineFieldHeight && !changed; y++) {
                for (int x = 1; x <= nMineFieldWidth && !changed; x++) {
                    char c = arrMineFieldData[32 * y + x];
                    if (!(((unsigned char)c & MINE_CELL_FLAG))) continue;
                    int nd = c & 0x1F;
                    if (nd < 1 || nd > 8) continue;
                    int flags = 0, unopened = 0;
                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            if (dx == 0 && dy == 0) continue;
                            int nx = x + dx, ny = y + dy;
                            if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight) continue;
                            char nc = arrMineFieldData[32 * ny + nx];
                            if ((nc & 0x1F) == TILE_FLAG) flags++;
                            else if (!(((unsigned char)nc & MINE_CELL_FLAG))) unopened++;
                        }
                    }
                    if (flags == nd && unopened > 0) {
                        HandleMiddleClickOnCell(x, y);
                        if (!stepFirst) response[off++] = ',';
                        stepFirst = 0;
                        off += sprintf_s(response + off, responseSize - off, "{\"t\":\"chord\",\"x\":%d,\"y\":%d}", x, y);
                        changed = 1; stepCount++;
                        if (speedMs) Sleep(speedMs);
                    }
                }
            }
            if (changed) continue;

            // ── Step 3: Constraint intersection analysis ──
            // Subset rule: if A's unknowns ⊆ B's unknowns and A.mines == B.mines, then B\A are safe
            // Superset rule: if A's unknowns ⊆ B's unknowns and B.mines - A.mines == |B\A|, then B\A are mines
            if (unkCnt > 0) {
                for (int ai = 0; ai < nCons && !changed; ai++) {
                    for (int bi = 0; bi < nCons && !changed; bi++) {
                        if (ai == bi) continue;
                        if (cons[ai].cnt == 0 || cons[bi].cnt == 0) continue;
                        // Check if cons[ai] is subset of cons[bi]
                        int subset = 1, diffCnt = 0, diffX[8], diffY[8];
                        for (int k = 0; k < cons[bi].cnt; k++) {
                            int found = 0;
                            for (int j = 0; j < cons[ai].cnt; j++)
                                if (cons[bi].cx[k] == cons[ai].cx[j] && cons[bi].cy[k] == cons[ai].cy[j]) { found = 1; break; }
                            if (!found && diffCnt < 8) { diffX[diffCnt] = cons[bi].cx[k]; diffY[diffCnt] = cons[bi].cy[k]; diffCnt++; }
                        }
                        if (diffCnt == cons[bi].cnt - cons[ai].cnt) {
                            int mA = cons[ai].mines, mB = cons[bi].mines;
                            int dM = mB - mA;
                            if (dM == 0 && diffCnt > 0) {
                                // Subset, equal mines → diff cells are safe
                                for (int d = 0; d < diffCnt; d++) {
                                    int nx = diffX[d], ny = diffY[d];
                                    if ((arrMineFieldData[32 * ny + nx] & 0x1F) == TILE_UNOPENED) {
                                        HandleLeftClickOnCell(nx, ny);
                                        if (!stepFirst) response[off++] = ',';
                                        stepFirst = 0;
                                        off += sprintf_s(response + off, responseSize - off, "{\"t\":\"safe\",\"x\":%d,\"y\":%d}", nx, ny);
                                        changed = 1; stepCount++;
                                        if (speedMs) Sleep(speedMs);
                                    }
                                }
                            } else if (dM == diffCnt && dM > 0) {
                                // Subset, mine diff == cell count → diff cells are mines
                                for (int d = 0; d < diffCnt; d++) {
                                    int nx = diffX[d], ny = diffY[d];
                                    if ((arrMineFieldData[32 * ny + nx] & 0x1F) == TILE_UNOPENED) {
                                        HandleRightClickOnCell(nx, ny);
                                        if (!stepFirst) response[off++] = ',';
                                        stepFirst = 0;
                                        off += sprintf_s(response + off, responseSize - off, "{\"t\":\"flag\",\"x\":%d,\"y\":%d}", nx, ny);
                                        changed = 1; stepCount++;
                                        if (speedMs) Sleep(speedMs);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (changed) continue;

            // ── Step 4: Grouped full enumeration ──
            if (unkCnt > 0 && unkCnt <= 22) {
                int tValid = 0, mCnt[64] = {0};
                unsigned long long maxM = 1ULL << unkCnt;
                for (unsigned long long mask = 0; mask < maxM; mask++) {
                    int ok = 1;
                    for (int ci = 0; ci < nCons && ok; ci++) {
                        int found = 0;
                        for (int k = 0; k < cons[ci].cnt; k++)
                            for (int u = 0; u < unkCnt; u++)
                                if (unkX[u] == cons[ci].cx[k] && unkY[u] == cons[ci].cy[k])
                                    { if (mask & (1ULL << u)) found++; break; }
                        if (found != cons[ci].mines) ok = 0;
                    }
                    if (ok) { tValid++; for (int u = 0; u < unkCnt; u++) if (mask & (1ULL << u)) mCnt[u]++; }
                }
                if (tValid > 0) {
                    for (int u = 0; u < unkCnt && !changed; u++)
                        if (mCnt[u] == 0 && (arrMineFieldData[32 * unkY[u] + unkX[u]] & 0x1F) == TILE_UNOPENED) {
                            HandleLeftClickOnCell(unkX[u], unkY[u]);
                            if (!stepFirst) response[off++] = ',';
                            stepFirst = 0;
                            off += sprintf_s(response + off, responseSize - off, "{\"t\":\"safe\",\"x\":%d,\"y\":%d}", unkX[u], unkY[u]);
                            changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                        }
                    for (int u = 0; u < unkCnt && !changed; u++)
                        if (mCnt[u] == tValid && (arrMineFieldData[32 * unkY[u] + unkX[u]] & 0x1F) == TILE_UNOPENED) {
                            HandleRightClickOnCell(unkX[u], unkY[u]);
                            if (!stepFirst) response[off++] = ',';
                            stepFirst = 0;
                            off += sprintf_s(response + off, responseSize - off, "{\"t\":\"flag\",\"x\":%d,\"y\":%d}", unkX[u], unkY[u]);
                            changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                        }
                    if (!changed) {
                        float exactProb[64] = { 0.0f };
                        int hasExactProb[64] = { 0 };
                        for (int u = 0; u < unkCnt; u++) {
                            exactProb[u] = (float)mCnt[u] / tValid;
                            hasExactProb[u] = 1;
                        }

                        int bestX = 0, bestY = 0;
                        float bestP = 0.0f;
                        if (SelectBestGuess(unkX, unkY, unkCnt, exactProb, hasExactProb, &bestX, &bestY, &bestP)) {
                            HandleLeftClickOnCell(bestX, bestY);
                            if (!stepFirst) response[off++] = ',';
                            stepFirst = 0;
                            off += sprintf_s(response + off, responseSize - off, "{\"t\":\"guess\",\"x\":%d,\"y\":%d,\"p\":%.2f}", bestX, bestY, bestP);
                            changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                        }
                    }
                }
            }
            // ── Step 4b: Grouped enumeration for large unkCnt ──
            else if (!changed && unkCnt > 22) {
                int cid[64], cids = 0;
                for (int u = 0; u < unkCnt && u < 64; u++) cid[u] = -1;
                for (int u = 0; u < unkCnt && u < 64; u++) {
                    if (cid[u] >= 0) continue;
                    int q[64], h = 0, t = 0;
                    q[t++] = u; cid[u] = cids;
                    while (h < t) {
                        int cur = q[h++];
                        for (int ci = 0; ci < nCons; ci++) {
                            int inCons = 0;
                            for (int k = 0; k < cons[ci].cnt; k++)
                                if (cons[ci].cx[k] == unkX[cur] && cons[ci].cy[k] == unkY[cur]) { inCons = 1; break; }
                            if (!inCons) continue;
                            for (int k = 0; k < cons[ci].cnt; k++)
                                for (int v = 0; v < unkCnt && v < 64; v++)
                                    if (cid[v] < 0 && cons[ci].cx[k] == unkX[v] && cons[ci].cy[k] == unkY[v])
                                        { q[t++] = v; cid[v] = cids; break; }
                        }
                    }
                    cids++;
                }
                float prob[64]; int has[64] = {0};
                for (int g = 0; g < cids && !changed; g++) {
                    int gu[64], gn = 0;
                    for (int u = 0; u < unkCnt && u < 64; u++)
                        if (cid[u] == g) gu[gn++] = u;
                    if (gn < 2 || gn > 20) continue;
                    int gc[400], gcn = 0;
                    for (int ci = 0; ci < nCons; ci++) {
                        int touches = 0;
                        for (int k = 0; k < cons[ci].cnt && !touches; k++)
                            for (int ui = 0; ui < gn && !touches; ui++) {
                                int u = gu[ui];
                                if (cons[ci].cx[k] == unkX[u] && cons[ci].cy[k] == unkY[u]) touches = 1;
                            }
                        if (touches) gc[gcn++] = ci;
                    }
                    if (gcn == 0) continue;
                    int tv = 0, mc[64] = {0};
                    unsigned long long mskM = 1ULL << gn;
                    for (unsigned long long msk = 0; msk < mskM; msk++) {
                        int ok = 1;
                        for (int cii = 0; cii < gcn && ok; cii++) {
                            int ci = gc[cii], f = 0;
                            for (int k = 0; k < cons[ci].cnt; k++)
                                for (int ui = 0; ui < gn; ui++)
                                    if (cons[ci].cx[k] == unkX[gu[ui]] && cons[ci].cy[k] == unkY[gu[ui]])
                                        { if (msk & (1ULL << ui)) f++; break; }
                            if (f != cons[ci].mines) ok = 0;
                        }
                        if (ok) { tv++; for (int ui = 0; ui < gn; ui++) if (msk & (1ULL << ui)) mc[ui]++; }
                    }
                    if (tv == 0) continue;
                    for (int ui = 0; ui < gn; ui++) { prob[gu[ui]] = (float)mc[ui] / tv; has[gu[ui]] = 1; }
                    for (int ui = 0; ui < gn && !changed; ui++)
                        if (mc[ui] == 0 && (arrMineFieldData[32 * unkY[gu[ui]] + unkX[gu[ui]]] & 0x1F) == TILE_UNOPENED) {
                            HandleLeftClickOnCell(unkX[gu[ui]], unkY[gu[ui]]);
                            if (!stepFirst) response[off++] = ','; stepFirst = 0;
                            off += sprintf_s(response + off, responseSize - off, "{\"t\":\"safe\",\"x\":%d,\"y\":%d}", unkX[gu[ui]], unkY[gu[ui]]);
                            changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                        }
                    for (int ui = 0; ui < gn && !changed; ui++)
                        if (mc[ui] == tv && (arrMineFieldData[32 * unkY[gu[ui]] + unkX[gu[ui]]] & 0x1F) == TILE_UNOPENED) {
                            HandleRightClickOnCell(unkX[gu[ui]], unkY[gu[ui]]);
                            if (!stepFirst) response[off++] = ','; stepFirst = 0;
                            off += sprintf_s(response + off, responseSize - off, "{\"t\":\"flag\",\"x\":%d,\"y\":%d}", unkX[gu[ui]], unkY[gu[ui]]);
                            changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                        }
                }
                if (!changed) {
                    int bestX = 0, bestY = 0;
                    float bestP = 0.0f;
                    if (SelectBestGuess(unkX, unkY, unkCnt, prob, has, &bestX, &bestY, &bestP)) {
                        HandleLeftClickOnCell(bestX, bestY);
                        if (!stepFirst) response[off++] = ','; stepFirst = 0;
                        off += sprintf_s(response + off, responseSize - off, "{\"t\":\"guess\",\"x\":%d,\"y\":%d,\"p\":%.2f}", bestX, bestY, bestP);
                        changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                    }
                }
            }

            // ── Step 5: No constraints → corners/edges first for safer opening ──
            if (!changed) {
                int cx = nMineFieldWidth / 2, cy = nMineFieldHeight / 2;
                int qx = nMineFieldWidth / 4, qy = nMineFieldHeight / 4;
                if (qx < 1) qx = 1; if (qy < 1) qy = 1;
                int pts[][2] = {
                    {1, 1}, {nMineFieldWidth, 1}, {1, nMineFieldHeight}, {nMineFieldWidth, nMineFieldHeight},
                    {cx, cy / 2}, {cx, nMineFieldHeight - cy / 2 + 1},
                    {qx, qy}, {nMineFieldWidth - qx + 1, nMineFieldHeight - qy + 1},
                    {cx, cy}
                };
                int nPts = sizeof(pts) / sizeof(pts[0]);
                for (int i = 0; i < nPts && !changed; i++) {
                    int tx = pts[i][0], ty = pts[i][1];
                    if (tx < 1 || tx > nMineFieldWidth || ty < 1 || ty > nMineFieldHeight) continue;
                    char cc = arrMineFieldData[32 * ty + tx];
                    if (!(((unsigned char)cc & MINE_CELL_FLAG)) && (cc & 0x1F) != TILE_FLAG) {
                        HandleLeftClickOnCell(tx, ty);
                        if (!stepFirst) response[off++] = ',';
                        stepFirst = 0;
                        off += sprintf_s(response + off, responseSize - off, "{\"t\":\"open\",\"x\":%d,\"y\":%d}", tx, ty);
                        changed = 1; stepCount++;
                        if (speedMs) Sleep(speedMs);
                    }
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

// Auto-play thread parameters
struct AutoPlayParams {
    int maxSteps;
    int speedMs;
};

static unsigned __stdcall AutoPlayThread(void* lpParam)
{
    AutoPlayParams* params = (AutoPlayParams*)lpParam;
    int maxSteps = params->maxSteps;
    int speedMs = params->speedMs;
    delete params;

    int stepCount = 0;
    #define AP_MAX_UNK 64
    #define AP_MAX_CONS 400

    while (stepCount < maxSteps && (g_gameStatusArray[0] & 0x01) != 0) {
        int changed = 0;
        int unkX[AP_MAX_UNK], unkY[AP_MAX_UNK], unkCnt = 0;
        struct APCons { int x, y, mines, cnt; int cx[8], cy[8]; } cons[AP_MAX_CONS];
        int nCons = 0;

        // Build constraints
        for (int y = 1; y <= nMineFieldHeight; y++) {
            for (int x = 1; x <= nMineFieldWidth; x++) {
                char c = arrMineFieldData[32 * y + x];
                if (!(((unsigned char)c & 0x40))) continue;
                int nd = c & 0x1F;
                if (nd > 8) continue;
                int flags = 0; int cx[8], cy[8], cu = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx, ny = y + dy;
                        if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight) continue;
                        char nc = arrMineFieldData[32 * ny + nx];
                        if ((nc & 0x1F) == 14) { flags++; continue; }
                        if (((unsigned char)nc & 0x40)) continue;
                        if (cu < 8) { cx[cu] = nx; cy[cu] = ny; cu++; }
                    }
                }
                int rem = nd - flags;
                if (cu > 0 && nCons < AP_MAX_CONS) {
                    for (int k = 0; k < cu; k++) {
                        int seen = 0;
                        for (int u = 0; u < unkCnt; u++)
                            if (unkX[u] == cx[k] && unkY[u] == cy[k]) { seen = 1; break; }
                        if (!seen && unkCnt < AP_MAX_UNK) { unkX[unkCnt] = cx[k]; unkY[unkCnt] = cy[k]; unkCnt++; }
                    }
                    cons[nCons].x = x; cons[nCons].y = y; cons[nCons].mines = rem; cons[nCons].cnt = cu;
                    for (int k = 0; k < cu; k++) { cons[nCons].cx[k] = cx[k]; cons[nCons].cy[k] = cy[k]; }
                    nCons++;
                }
            }
        }

        // Step 1: mines==0 -> open all
        for (int i = 0; i < nCons; i++) {
            if (cons[i].mines == 0) {
                for (int k = 0; k < cons[i].cnt; k++) {
                    int nx = cons[i].cx[k], ny = cons[i].cy[k];
                    if ((arrMineFieldData[32 * ny + nx] & 0x1F) == 15) {
                        HandleLeftClickOnCell(nx, ny);
                        changed = 1; stepCount++;
                        if (speedMs) Sleep(speedMs);
                    }
                }
            }
        }
        if (changed) continue;

        // Step 1.5: mines == cnt -> flag all
        for (int i = 0; i < nCons; i++) {
            if (cons[i].mines == cons[i].cnt) {
                for (int k = 0; k < cons[i].cnt; k++) {
                    int nx = cons[i].cx[k], ny = cons[i].cy[k];
                    if ((arrMineFieldData[32 * ny + nx] & 0x1F) == 15) {
                        HandleRightClickOnCell(nx, ny);
                        changed = 1; stepCount++;
                        if (speedMs) Sleep(speedMs);
                    }
                }
            }
        }
        if (changed) continue;

        // Step 2: Chord
        for (int y = 1; y <= nMineFieldHeight && !changed; y++) {
            for (int x = 1; x <= nMineFieldWidth && !changed; x++) {
                char c = arrMineFieldData[32 * y + x];
                if (!(((unsigned char)c & 0x40))) continue;
                int nd = c & 0x1F;
                if (nd < 1 || nd > 8) continue;
                int flags = 0, unopened = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = x + dx, ny = y + dy;
                        if (nx < 1 || nx > nMineFieldWidth || ny < 1 || ny > nMineFieldHeight) continue;
                        char nc = arrMineFieldData[32 * ny + nx];
                        if ((nc & 0x1F) == 14) flags++;
                        else if (!(((unsigned char)nc & 0x40))) unopened++;
                    }
                }
                if (flags == nd && unopened > 0) {
                    HandleMiddleClickOnCell(x, y);
                    changed = 1; stepCount++;
                    if (speedMs) Sleep(speedMs);
                }
            }
        }
        if (changed) continue;

        // Step 3: Constraint intersection analysis
        if (unkCnt > 0) {
            for (int ai = 0; ai < nCons && !changed; ai++) {
                for (int bi = 0; bi < nCons && !changed; bi++) {
                    if (ai == bi || cons[ai].cnt == 0 || cons[bi].cnt == 0) continue;
                    int diffCnt = 0, diffX[8], diffY[8];
                    for (int k = 0; k < cons[bi].cnt; k++) {
                        int found = 0;
                        for (int j = 0; j < cons[ai].cnt; j++)
                            if (cons[bi].cx[k] == cons[ai].cx[j] && cons[bi].cy[k] == cons[ai].cy[j]) { found = 1; break; }
                        if (!found && diffCnt < 8) { diffX[diffCnt] = cons[bi].cx[k]; diffY[diffCnt] = cons[bi].cy[k]; diffCnt++; }
                    }
                    if (diffCnt == cons[bi].cnt - cons[ai].cnt) {
                        int dM = cons[bi].mines - cons[ai].mines;
                        if (dM == 0 && diffCnt > 0) {
                            for (int d = 0; d < diffCnt; d++) {
                                if ((arrMineFieldData[32 * diffY[d] + diffX[d]] & 0x1F) == 15) {
                                    HandleLeftClickOnCell(diffX[d], diffY[d]);
                                    changed = 1; stepCount++;
                                    if (speedMs) Sleep(speedMs);
                                }
                            }
                        } else if (dM == diffCnt && dM > 0) {
                            for (int d = 0; d < diffCnt; d++) {
                                if ((arrMineFieldData[32 * diffY[d] + diffX[d]] & 0x1F) == 15) {
                                    HandleRightClickOnCell(diffX[d], diffY[d]);
                                    changed = 1; stepCount++;
                                    if (speedMs) Sleep(speedMs);
                                }
                            }
                        }
                    }
                }
            }
        }
        if (changed) continue;

        // Step 4: Full enumeration (when unkCnt <= 22)
        if (unkCnt > 0 && unkCnt <= 22) {
            int tValid = 0, mCnt[64] = {0};
            unsigned long long maxM = 1ULL << unkCnt;
            for (unsigned long long mask = 0; mask < maxM; mask++) {
                int ok = 1;
                for (int ci = 0; ci < nCons && ok; ci++) {
                    int found = 0;
                    for (int k = 0; k < cons[ci].cnt; k++)
                        for (int u = 0; u < unkCnt; u++)
                            if (unkX[u] == cons[ci].cx[k] && unkY[u] == cons[ci].cy[k])
                                { if (mask & (1ULL << u)) found++; break; }
                    if (found != cons[ci].mines) ok = 0;
                }
                if (ok) { tValid++; for (int u = 0; u < unkCnt; u++) if (mask & (1ULL << u)) mCnt[u]++; }
            }
            if (tValid > 0) {
                for (int u = 0; u < unkCnt && !changed; u++)
                    if (mCnt[u] == 0 && (arrMineFieldData[32 * unkY[u] + unkX[u]] & 0x1F) == 15) {
                        HandleLeftClickOnCell(unkX[u], unkY[u]);
                        changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                    }
                for (int u = 0; u < unkCnt && !changed; u++)
                    if (mCnt[u] == tValid && (arrMineFieldData[32 * unkY[u] + unkX[u]] & 0x1F) == 15) {
                        HandleRightClickOnCell(unkX[u], unkY[u]);
                        changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                    }
                if (!changed) {
                    float exactProb[64] = { 0.0f };
                    int hasExactProb[64] = { 0 };
                    for (int u = 0; u < unkCnt; u++) {
                        exactProb[u] = (float)mCnt[u] / tValid;
                        hasExactProb[u] = 1;
                    }

                    int bestX = 0, bestY = 0;
                    float bestP = 0.0f;
                    if (SelectBestGuess(unkX, unkY, unkCnt, exactProb, hasExactProb, &bestX, &bestY, &bestP)) {
                        HandleLeftClickOnCell(bestX, bestY);
                        changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                    }
                }
            }
        }
        else if (unkCnt > 22) {
            int cid[64], cids = 0;
            for (int u = 0; u < unkCnt && u < 64; u++) cid[u] = -1;
            for (int u = 0; u < unkCnt && u < 64; u++) {
                if (cid[u] >= 0) continue;
                int q[64], h = 0, t = 0;
                q[t++] = u; cid[u] = cids;
                while (h < t) {
                    int cur = q[h++];
                    for (int ci = 0; ci < nCons; ci++) {
                        int inCons = 0;
                        for (int k = 0; k < cons[ci].cnt; k++)
                            if (cons[ci].cx[k] == unkX[cur] && cons[ci].cy[k] == unkY[cur]) { inCons = 1; break; }
                        if (!inCons) continue;
                        for (int k = 0; k < cons[ci].cnt; k++)
                            for (int v = 0; v < unkCnt && v < 64; v++)
                                if (cid[v] < 0 && cons[ci].cx[k] == unkX[v] && cons[ci].cy[k] == unkY[v])
                                    { q[t++] = v; cid[v] = cids; break; }
                    }
                }
                cids++;
            }

            float prob[64] = { 0.0f };
            int has[64] = { 0 };
            for (int g = 0; g < cids && !changed; g++) {
                int gu[64], gn = 0;
                for (int u = 0; u < unkCnt && u < 64; u++)
                    if (cid[u] == g) gu[gn++] = u;
                if (gn < 2 || gn > 20) continue;

                int gc[400], gcn = 0;
                for (int ci = 0; ci < nCons; ci++) {
                    int touches = 0;
                    for (int k = 0; k < cons[ci].cnt && !touches; k++)
                        for (int ui = 0; ui < gn && !touches; ui++) {
                            int u = gu[ui];
                            if (cons[ci].cx[k] == unkX[u] && cons[ci].cy[k] == unkY[u]) touches = 1;
                        }
                    if (touches) gc[gcn++] = ci;
                }
                if (gcn == 0) continue;

                int tv = 0, mc[64] = { 0 };
                unsigned long long mskM = 1ULL << gn;
                for (unsigned long long msk = 0; msk < mskM; msk++) {
                    int ok = 1;
                    for (int cii = 0; cii < gcn && ok; cii++) {
                        int ci = gc[cii], f = 0;
                        for (int k = 0; k < cons[ci].cnt; k++)
                            for (int ui = 0; ui < gn; ui++)
                                if (cons[ci].cx[k] == unkX[gu[ui]] && cons[ci].cy[k] == unkY[gu[ui]])
                                    { if (msk & (1ULL << ui)) f++; break; }
                        if (f != cons[ci].mines) ok = 0;
                    }
                    if (ok) { tv++; for (int ui = 0; ui < gn; ui++) if (msk & (1ULL << ui)) mc[ui]++; }
                }
                if (tv == 0) continue;

                for (int ui = 0; ui < gn; ui++) { prob[gu[ui]] = (float)mc[ui] / tv; has[gu[ui]] = 1; }
                for (int ui = 0; ui < gn && !changed; ui++)
                    if (mc[ui] == 0 && (arrMineFieldData[32 * unkY[gu[ui]] + unkX[gu[ui]]] & 0x1F) == TILE_UNOPENED) {
                        HandleLeftClickOnCell(unkX[gu[ui]], unkY[gu[ui]]);
                        changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                    }
                for (int ui = 0; ui < gn && !changed; ui++)
                    if (mc[ui] == tv && (arrMineFieldData[32 * unkY[gu[ui]] + unkX[gu[ui]]] & 0x1F) == TILE_UNOPENED) {
                        HandleRightClickOnCell(unkX[gu[ui]], unkY[gu[ui]]);
                        changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                    }
            }

            if (!changed) {
                int bestX = 0, bestY = 0;
                float bestP = 0.0f;
                if (SelectBestGuess(unkX, unkY, unkCnt, prob, has, &bestX, &bestY, &bestP)) {
                    HandleLeftClickOnCell(bestX, bestY);
                    changed = 1; stepCount++; if (speedMs) Sleep(speedMs);
                }
            }
        }

        // Step 5: No progress -> corners/edges first for safer opening
        if (!changed) {
            int cx = nMineFieldWidth / 2, cy = nMineFieldHeight / 2;
            int qx = nMineFieldWidth / 4, qy = nMineFieldHeight / 4;
            if (qx < 1) qx = 1; if (qy < 1) qy = 1;
            int pts[][2] = {
                {1, 1}, {nMineFieldWidth, 1}, {1, nMineFieldHeight}, {nMineFieldWidth, nMineFieldHeight},
                {cx, cy / 2}, {cx, nMineFieldHeight - cy / 2 + 1},
                {qx, qy}, {nMineFieldWidth - qx + 1, nMineFieldHeight - qy + 1},
                {cx, cy}
            };
            for (int i = 0; i < 9 && !changed; i++) {
                int tx = pts[i][0], ty = pts[i][1];
                if (tx < 1 || tx > nMineFieldWidth || ty < 1 || ty > nMineFieldHeight) continue;
                char cc = arrMineFieldData[32 * ty + tx];
                if (!(((unsigned char)cc & 0x40)) && (cc & 0x1F) != 14) {
                    HandleLeftClickOnCell(tx, ty);
                    changed = 1; stepCount++;
                    if (speedMs) Sleep(speedMs);
                }
            }
        }
        if (!changed) break;
    }
    #undef AP_MAX_UNK
    #undef AP_MAX_CONS
    return 0;
}

void StartAutoPlay(int maxSteps, int speedMs)
{
    if (!((g_gameStatusArray[0] & 0x01))) {
        ResetGame();
    }
    if (maxSteps <= 0) maxSteps = 500;
    if (speedMs < 0) speedMs = 50;

    AutoPlayParams* params = new AutoPlayParams();
    params->maxSteps = maxSteps;
    params->speedMs = speedMs;
    _beginthreadex(NULL, 0, AutoPlayThread, params, 0, NULL);
}

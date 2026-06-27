#!/usr/bin/env python3
"""
XPWinmine MCP Server - 扫雷游戏 MCP 服务端
通过 Named Pipe 与扫雷游戏通信，为 LLM 提供游戏控制和分析工具。

安装方式：
  直接运行或通过 OpenCode MCP 配置加载。

  工具列表：
  - ping                  : 检测与游戏的连接
  - get_game_state        : 获取完整游戏状态
  - get_board             : 获取雷区完整布局
  - get_cell              : 获取指定格子的详细信息
  - get_adjacent          : 获取指定格子周围邻居的信息
  - click_cell            : 左键点击翻格
  - flag_cell             : 右键标旗/取消标旗
  - chord_cell            : 中键扫雷（已打开数字格周围）
  - reset_game            : 重新开始游戏
  - set_difficulty        : 切换难度
  - analyze_board         : AI 分析雷区（找出确定安全/有雷的格子）
  - get_game_window_info  : 获取游戏窗口信息
  - save_snapshot         : 保存当前局面到指定文件路径
  - load_snapshot         : 从指定文件路径恢复局面
"""

import json
import os
import sys
import time
import struct
import asyncio
from pathlib import Path

HAS_PYWIN32 = False
try:
    import win32pipe
    import win32file
    import win32api
    import win32con
    import win32gui
    import win32process
    import win32ui
    HAS_PYWIN32 = True
except ImportError:
    HAS_PYWIN32 = False

PIPE_NAME = r"\\.\pipe\XPWinmineMCP"
PIPE_TIMEOUT = 5000
GAME_PATH = Path(__file__).parent.parent / "XPWinmine" / "Release" / "winmine.exe"


def find_game_window():
    """查找扫雷游戏窗口"""
    if not HAS_PYWIN32:
        return None
    try:
        hwnd = win32gui.FindWindow(None, "扫雷")
        if hwnd:
            return hwnd
        hwnd = win32gui.FindWindow("WinMine_Window", None)
        return hwnd
    except Exception:
        return None


class PipeClient:
    """持久化 Named Pipe 客户端，复用连接"""

    def __init__(self):
        self._pipe = None

    def _connect(self):
        if self._pipe is not None:
            try:
                win32file.CloseHandle(self._pipe)
            except Exception:
                pass
            self._pipe = None

        if not HAS_PYWIN32:
            return False

        try:
            self._pipe = win32file.CreateFile(
                PIPE_NAME,
                win32con.GENERIC_READ | win32con.GENERIC_WRITE,
                0, None, win32con.OPEN_EXISTING, 0, None
            )
            return True
        except Exception:
            self._pipe = None
            return False

    def send(self, cmd: str) -> str:
        for attempt in range(3):
            if self._pipe is None:
                if not self._connect():
                    if attempt < 2:
                        time.sleep(0.1)
                        continue
                    return json.dumps({"error": "无法连接到游戏，请确保游戏已启动"})

            try:
                cmd_bytes = (cmd.strip() + "\n").encode("utf-8")
                win32file.WriteFile(self._pipe, cmd_bytes)
                result, data = win32file.ReadFile(self._pipe, 65536)
                if result == 0:
                    return data.decode("utf-8").strip()
                else:
                    self._pipe = None
                    if attempt < 2:
                        time.sleep(0.1)
                        continue
                    return json.dumps({"error": f"读取响应失败: {result}"})
            except Exception as e:
                self._pipe = None
                if attempt < 2:
                    time.sleep(0.1)
                    continue
                return json.dumps({"error": f"通信异常: {e}"})

        return json.dumps({"error": "多次重试后仍无法通信"})

    def close(self):
        if self._pipe is not None:
            try:
                win32file.CloseHandle(self._pipe)
            except Exception:
                pass
            self._pipe = None


_pipe_client = PipeClient()


def send_command(cmd: str) -> str:
    """通过 Named Pipe 向游戏发送命令并接收响应"""
    return _pipe_client.send(cmd)


def _cmd(cmd: str) -> dict:
    resp = send_command(cmd)
    try:
        return json.loads(resp)
    except json.JSONDecodeError:
        return {"error": f"无法解析响应: {resp}"}


def screenshot_base64(hwnd=None) -> str | None:
    """截取游戏窗口截图，返回 base64 PNG"""
    if not HAS_PYWIN32:
        return None
    if not hwnd:
        hwnd = find_game_window()
    if not hwnd:
        return None
    try:
        left, top, right, bottom = win32gui.GetWindowRect(hwnd)
        width = right - left
        height = bottom - top

        hwnd_dc = win32gui.GetWindowDC(hwnd)
        mfc_dc = win32ui.CreateDCFromHandle(hwnd_dc)
        save_dc = mfc_dc.CreateCompatibleDC()

        bitmap = win32ui.CreateBitmap()
        bitmap.CreateCompatibleBitmap(mfc_dc, width, height)
        save_dc.SelectObject(bitmap)
        save_dc.BitBlt((0, 0), (width, height), mfc_dc, (0, 0), win32con.SRCCOPY)

        import base64
        import io
        bitmap_bits = bitmap.GetBitmapBits(True)
        from PIL import Image
        img = Image.frombuffer(
            "RGB", (width, height), bitmap_bits,
            "raw", "BGRX", 0, 1
        )
        buffer = io.BytesIO()
        img.save(buffer, format="PNG")
        b64 = base64.b64encode(buffer.getvalue()).decode()

        save_dc.DeleteDC()
        mfc_dc.DeleteDC()
        win32gui.ReleaseDC(hwnd, hwnd_dc)
        win32gui.DeleteObject(bitmap.GetHandle())

        return b64
    except Exception:
        return None


# ============================================================================
# MCP Server
# ============================================================================

from mcp.server.fastmcp import FastMCP

mcp = FastMCP("XPWinmine-MCP")

DISPLAY_NAMES = {
    0: "已打开(空白)",
    1: "数字1", 2: "数字2", 3: "数字3", 4: "数字4",
    5: "数字5", 6: "数字6", 7: "数字7", 8: "数字8",
    9: "问号", 10: "地雷", 11: "标记错误(叉)",
    12: "红色地雷(爆炸)", 13: "按下", 14: "旗子", 15: "未打开", 16: "边界"
}

DIFFICULTY_NAMES = {0: "初级", 1: "中级", 2: "高级", 3: "自定义"}


@mcp.tool()
async def ping() -> str:
    """检测与扫雷游戏的连接状态"""
    result = _cmd("PING")
    status = result.get("status", result.get("error", "unknown"))
    if status == "ok":
        hwnd = find_game_window()
        if hwnd:
            return f"连接正常。游戏窗口句柄: {hwnd}"
        return "连接正常，但未找到游戏窗口。"
    return f"连接失败: {status}"


@mcp.tool()
async def get_game_state() -> str:
    """获取完整的游戏状态信息：
    - 游戏状态(运行中/暂停/结束)
    - 难度、雷区大小、总雷数、剩余雷数
    - 已用时间、已打开格子数
    - 各难度最佳成绩
    - 笑脸按钮状态
    """
    result = _cmd("GET_STATE")
    if "error" in result:
        return f"错误: {result['error']}"

    game_status = result.get("game_status", "unknown")
    difficulty = DIFFICULTY_NAMES.get(result.get("difficulty", -1), "未知")
    width = result.get("board_width", 0)
    height = result.get("board_height", 0)
    total_mines = result.get("total_mines", 0)
    remaining = result.get("mines_remaining", 0)
    time_elapsed = result.get("time_elapsed", 0)
    opened = result.get("opened_cells", 0)
    safe_remaining = result.get("safe_cells_remaining", 0)
    smiley = result.get("smiley", "unknown")
    best_beginner = result.get("best_time_beginner", 999)
    best_intermediate = result.get("best_time_intermediate", 999)
    best_expert = result.get("best_time_expert", 999)

    lines = [
        f"游戏状态: {game_status}",
        f"难度: {difficulty}",
        f"雷区: {width} x {height}",
        f"地雷总数: {total_mines}",
        f"剩余雷数(显示): {remaining}",
        f"已用时间: {time_elapsed} 秒",
        f"已打开格子: {opened}",
        f"剩余安全格: {safe_remaining}",
        f"笑脸状态: {smiley}",
        f"最佳成绩: 初级={best_beginner}秒 中级={best_intermediate}秒 高级={best_expert}秒",
    ]
    return "\n".join(lines)


@mcp.tool()
async def get_board() -> str:
    """获取整个雷区的完整布局状态。
    返回每个格子的：
    - d: 显示状态(0-15)
    - m: 是否有地雷(0/1)
    - o: 是否已打开(0/1)
    - x: 是否爆炸(0/1)

    显示状态对照：
    0=空白, 1-8=数字, 9=问号, 10=地雷, 11=叉, 12=红色地雷, 13=按下, 14=旗子, 15=未打开
    """
    result = _cmd("GET_BOARD")
    if "error" in result:
        return f"错误: {result['error']}"

    board = result.get("board", [])
    if not board:
        return "雷区为空。"

    disp_map = {0: "·", 1: "1", 2: "2", 3: "3", 4: "4", 5: "5", 6: "6", 7: "7", 8: "8",
                9: "?", 10: "*", 11: "X", 12: "R", 13: "▾", 14: "F", 15: "█"}

    lines = []
    for y, row in enumerate(board):
        row_str = ""
        for cell in row:
            d = cell.get("d", 15)
            m = cell.get("m", 0)
            o = cell.get("o", 0)
            if o:
                row_str += disp_map.get(d, "?")
            elif d == 14:
                row_str += "F"
            else:
                row_str += "█"
        lines.append(f"  {y+1:2d}: {row_str}")

    header = "     "
    for x in range(1, len(board[0]) + 1 if board else 1):
        header += str(x % 10)
    return header + "\n" + "\n".join(lines)


@mcp.tool()
async def get_cell(x: int, y: int) -> str:
    """获取指定格子的详细信息。

    参数:
    - x: 列坐标 (从1开始)
    - y: 行坐标 (从1开始)

    返回格子的显示状态、是否有雷、是否已打开、周围雷数等。
    """
    result = _cmd(f"GET_CELL {x} {y}")
    if "error" in result:
        return f"错误: {result['error']}"

    d = result.get("display", -1)
    has_mine = result.get("has_mine", 0)
    is_opened = result.get("is_opened", 0)
    is_exploded = result.get("is_exploded", 0)
    adj = result.get("adjacent_mines", 0)
    disp_name = DISPLAY_NAMES.get(d, "未知")

    lines = [
        f"格子 ({x}, {y}):",
        f"  显示状态: {disp_name} (代码={d})",
        f"  是否有雷: {'是' if has_mine else '否'}",
        f"  是否打开: {'是' if is_opened else '否'}",
        f"  是否爆炸: {'是' if is_exploded else '否'}",
        f"  周围雷数: {adj}",
    ]
    return "\n".join(lines)


@mcp.tool()
async def get_adjacent(x: int, y: int) -> str:
    """获取指定格子周围 8 个邻居的信息。

    参数:
    - x: 列坐标 (从1开始)
    - y: 行坐标 (从1开始)
    """
    result = _cmd(f"GET_ADJACENT {x} {y}")
    if "error" in result:
        return f"错误: {result['error']}"

    cells = result.get("cells", [])
    if not cells:
        return "没有邻居信息。"

    lines = [f"格子 ({x}, {y}) 周围邻居:"]
    for c in cells:
        cx, cy = c["x"], c["y"]
        d = c.get("display", 15)
        has_mine = c.get("has_mine", 0)
        is_opened = c.get("is_opened", 0)
        disp_name = DISPLAY_NAMES.get(d, "未知")
        mine_str = "雷" if has_mine else "安全"
        open_str = "已开" if is_opened else "未开"
        lines.append(f"  ({cx}, {cy}): {disp_name} | {mine_str} | {open_str}")

    return "\n".join(lines)


@mcp.tool()
async def click_cell(x: int, y: int) -> str:
    """左键点击指定格子（翻开）。

    参数:
    - x: 列坐标 (从1开始)
    - y: 行坐标 (从1开始)

    注意：如果格子上有旗子，点击无效。如果点击到地雷，游戏结束！
    """
    result = _cmd(f"OPEN {x} {y}")
    if "error" in result:
        return f"错误: {result['error']}"
    return f"已点击格子 ({x}, {y})"


@mcp.tool()
async def flag_cell(x: int, y: int) -> str:
    """右键点击指定格子（标旗 / 问号 / 取消循环切换）。

    参数:
    - x: 列坐标 (从1开始)
    - y: 行坐标 (从1开始)

    在「标记模式」下切换：未打开 → 旗子 → 问号 → 未打开
    在非标记模式下切换：未打开 → 旗子 → 未打开
    """
    result = _cmd(f"FLAG {x} {y}")
    if "error" in result:
        return f"错误: {result['error']}"
    return f"已右键点击格子 ({x}, {y})"


@mcp.tool()
async def chord_cell(x: int, y: int) -> str:
    """中键/双键点击已打开的数字格子（扫雷/Chord）。
    当数字周围的旗子数量等于数字时，自动翻开周围未标记的格子。

    参数:
    - x: 列坐标 (从1开始)
    - y: 行坐标 (从1开始)

    警告：如果旗子标错了位置，此操作可能点爆地雷！
    """
    result = _cmd(f"CHORD {x} {y}")
    if "error" in result:
        return f"错误: {result['error']}"
    return f"已中键点击格子 ({x}, {y})"


@mcp.tool()
async def reset_game() -> str:
    """重新开始游戏（相当于点击笑脸按钮）。"""
    result = _cmd("RESET")
    if "error" in result:
        return f"错误: {result['error']}"
    return "游戏已重新开始。"


@mcp.tool()
async def set_difficulty(level: int) -> str:
    """切换游戏难度。

    参数:
    - level: 0=初级(9x9,10雷) 1=中级(16x16,40雷) 2=高级(16x30,99雷) 3=自定义
    """
    if level not in (0, 1, 2, 3):
        return "错误: level 必须是 0(初级), 1(中级), 2(高级), 3(自定义)"
    result = _cmd(f"SET_DIFFICULTY {level}")
    if "error" in result:
        return f"错误: {result['error']}"
    name = DIFFICULTY_NAMES.get(level, "未知")
    return f"难度已切换为: {name}"


@mcp.tool()
async def save_snapshot(file_path: str) -> str:
    """将当前扫雷局面保存到指定文件路径。

    参数:
    - file_path: 目标存档文件路径。不会弹出任何 UI 选择框。
    """
    if not file_path or not file_path.strip():
        return "错误: file_path 不能为空"

    result = _cmd(f"SAVE_SNAPSHOT {file_path}")
    if "error" in result:
        return f"错误: {result['error']}"

    saved_path = result.get("path", file_path)
    return f"已保存当前局面到: {saved_path}"


@mcp.tool()
async def load_snapshot(file_path: str) -> str:
    """从指定文件路径加载之前保存的扫雷局面。

    参数:
    - file_path: 要加载的存档文件路径。不会弹出任何 UI 选择框。
    """
    if not file_path or not file_path.strip():
        return "错误: file_path 不能为空"

    result = _cmd(f"LOAD_SNAPSHOT {file_path}")
    if "error" in result:
        return f"错误: {result['error']}"

    loaded_path = result.get("path", file_path)
    return f"已从文件恢复局面: {loaded_path}"


@mcp.tool()
async def auto_solve(max_steps: int = 200, speed_ms: int = 1000) -> str:
    """自动求解扫雷游戏。每一步有可视化延迟。

    使用基本规则（确定安全/地雷）+ Chord + 概率猜测自动完成游戏。

    参数:
    - max_steps: 最大步数限制 (默认200)
    - speed_ms: 每步之间的延迟毫秒数 (默认1000ms，便于观察)
    """
    result = _cmd(f"AUTOPLAY {max_steps} {speed_ms}")
    if "error" in result:
        return f"错误: {result['error']}"

    total = result.get("total_steps", 0)
    won = result.get("won", 0)
    time_elapsed = result.get("time", 0)
    opened = result.get("opened", 0)
    mines_left = result.get("mines_left", 0)
    steps = result.get("steps", [])
    final = result.get("final_state", {})

    lines = []

    if final:
        status = final.get("status", "unknown")
        smiley = final.get("smiley", "?")
        lines.append(f"=== Game Over: {'WIN' if won else 'LOST'} ===")
        lines.append(f"  Final status: {status}, Smiley: {smiley}")
        lines.append(f"  Board: {final.get('board_width',0)}x{final.get('board_height',0)}, "
                     f"Mines: {final.get('total_mines',0)}/{final.get('mines_remaining',0)}")
        lines.append(f"  Opened: {final.get('opened_cells',0)}, Safe remaining: {final.get('safe_cells_remaining',0)}")
        lines.append(f"  Time: {time_elapsed}s, Steps taken: {total}")
        if won:
            lines.append(f"\n  *** YOU WIN! ***")
        else:
            lines.append(f"\n  *** BOOM! Hit a mine. ***")

    if steps:
        lines.append(f"\nStep details ({len(steps)} total):")
        for s in steps[:20]:
            t = s.get("t", "?")
            x, y = s.get("x", 0), s.get("y", 0)
            p = s.get("p", 0)
            desc = {"open": "open", "flag": "flag", "chord": "chord", "guess": "guess"}.get(t, t)
            if p:
                lines.append(f"  {desc:6s} ({x},{y}) risk={p:.0%}")
            else:
                lines.append(f"  {desc:6s} ({x},{y})")
        if len(steps) > 20:
            lines.append(f"  ... and {len(steps) - 20} more steps")

    return "\n".join(lines)


@mcp.tool()
async def analyze_board() -> str:
    """AI 分析雷区状态，找出可以确定安全的格子和确定是地雷的格子。

    使用基本扫雷推理规则：
    - 安全格子：已打开数字格的周围旗子数 = 数字值，则其余未打开格都是安全的
    - 地雷格子：已打开数字格的周围未打开格数 = 数字值，则这些未打开格都是地雷
    """
    result = _cmd("ANALYZE")
    if "error" in result:
        return f"错误: {result['error']}"

    safe_cells = result.get("safe_cells", [])
    mine_cells = result.get("mine_cells", [])
    safe_count = result.get("safe_count", 0)
    mine_count = result.get("mine_count", 0)

    lines = []
    if safe_cells:
        lines.append(f"Determine safe cells ({safe_count}):")
        for c in safe_cells[:30]:
            lines.append(f"  ({c['x']}, {c['y']})")
        if safe_count > 30:
            lines.append(f"  ... and {safe_count - 30} more")
    else:
        lines.append("No certain safe cells found.")

    if mine_cells:
        lines.append(f"\nCertain mine cells ({mine_count}):")
        for c in mine_cells[:30]:
            lines.append(f"  ({c['x']}, {c['y']})")
        if mine_count > 30:
            lines.append(f"  ... and {mine_count - 30} more")
    else:
        lines.append("\nNo certain mine cells found.")

    lines.append(f"\nSuggestions:")
    if safe_cells:
        lines.append(f"  Use click_cell to open safe cells.")
    if mine_cells:
        lines.append(f"  Use flag_cell to mark mine cells.")

    if not safe_cells and not mine_cells:
        board = _cmd("GET_BOARD").get("board", [])
        h = len(board); w = len(board[0]) if board else 0

        risk = {}
        for y in range(h):
            for x in range(w):
                cell = board[y][x]
                if not cell['o'] and cell['d'] != 14:
                    risk[(x+1, y+1)] = 0.0

        for y in range(h):
            for x in range(w):
                cell = board[y][x]
                if cell['o'] and 1 <= cell['d'] <= 8:
                    num = cell['d']
                    flags = 0; unop = []
                    for dy in (-1, 0, 1):
                        for dx in (-1, 0, 1):
                            if dx == 0 and dy == 0: continue
                            nx, ny = x+dx, y+dy
                            if 0 <= nx < w and 0 <= ny < h:
                                nb = board[ny][nx]
                                if nb['d'] == 14: flags += 1
                                elif not nb['o']: unop.append((nx+1, ny+1))
                    remaining = num - flags
                    if remaining > 0 and unop:
                        p = remaining / len(unop)
                        for pos in unop:
                            risk[pos] = max(risk.get(pos, 0), p)

        if risk:
            sorted_risk = sorted(risk.items(), key=lambda r: r[1])
            lines.append("  No certain cells. Estimated mine probability for each unopened cell:")
            lines.append("  (Lower = safer, sorted by risk)")
            for (cx, cy), p in sorted_risk[:15]:
                bar = "#" * int(p * 10) + "." * (10 - int(p * 10))
                lines.append(f"  ({cx}, {cy}): {p:.1%} [{bar}]")
            if len(sorted_risk) > 15:
                lines.append(f"  ... and {len(sorted_risk) - 15} more")
        else:
            lines.append("  Current position requires guessing, no definite cells found.")

    return "\n".join(lines)


@mcp.tool()
async def get_game_window_info() -> str:
    """获取游戏窗口信息（窗口句柄、位置、大小等）。"""
    hwnd = find_game_window()
    if not hwnd:
        return "错误: 未找到扫雷游戏窗口。"

    try:
        rect = win32gui.GetWindowRect(hwnd)
        width = rect[2] - rect[0]
        height = rect[3] - rect[1]
        client_rect = win32gui.GetClientRect(hwnd)
        title = win32gui.GetWindowText(hwnd)
        class_name = win32gui.GetClassName(hwnd)
        _, pid = win32process.GetWindowThreadProcessId(hwnd)

        return (
            f"窗口句柄: {hwnd}\n"
            f"标题: {title}\n"
            f"类名: {class_name}\n"
            f"进程ID: {pid}\n"
            f"位置: ({rect[0]}, {rect[1]})\n"
            f"大小: {width} x {height}\n"
            f"客户区: {client_rect[2]} x {client_rect[3]}"
        )
    except Exception as e:
        return f"错误: {e}"


@mcp.tool()
async def screenshot() -> str:
    """截取游戏窗口的屏幕截图（返回 base64 编码的 PNG 图像）。

    可用于让 AI 观察游戏界面。
    """
    hwnd = find_game_window()
    if not hwnd:
        return "错误: 未找到扫雷游戏窗口，请先启动游戏。"

    b64 = screenshot_base64(hwnd)
    if b64:
        return f"[截图] data:image/png;base64,{b64[:200]}...(截取成功，共{len(b64)}字符)"
    return "截图失败。"


def main():
    mcp.run_stdio_async()


if __name__ == "__main__":
    import asyncio
    asyncio.run(mcp.run_stdio_async())

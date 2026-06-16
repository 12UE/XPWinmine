**English** | [中文](README.zh-CN.md)

# XPWinmine - Restored Windows XP SP3 Minesweeper

A source-level restoration of the classic Windows XP SP3 Minesweeper (`winmine.exe`) through reverse engineering, implemented in C++.

## Screenshot

![Minesweeper](https://upload.wikimedia.org/wikipedia/en/2/20/Minesweeper_Icon.png)

## Features

- Fully restored game logic and UI rendering of XP SP3 Minesweeper
- Three difficulty levels: Beginner (9×9, 10 mines), Intermediate (16×16, 40 mines), Expert (16×30, 99 mines)
- Custom minefield size and mine count
- Original sound effects, timer, and smiley button interaction
- Registry read/write for game settings and high scores
- XYZZY cheat code
- Help document invocation

## Project Structure

```
XPWinmine/
├── winmine.cpp          # Main source (game logic, window procedure, drawing, etc.)
├── Resource.h           # Resource ID definitions
├── XPWinmine.rc         # Resource script (menus, dialogs, bitmaps, icons, sounds)
├── XPWinmine.h          # Project header
├── framework.h          # Framework header
├── targetver.h          # Target Windows version
├── BITMAP*.bmp          # Original bitmap resources (cells, digits, smiley)
├── WAVE*.wav            # Original sound resources (win, lose, tick)
├── ICON100_1.ico        # Original icon
└── MANIFEST1_1.txt      # Application manifest
```

## Build

### Requirements

- Visual Studio 2019 or later
- Windows SDK

### Steps

1. Open `XPWinmine.sln` in Visual Studio
2. Select `Release` or `Debug` configuration
3. Click **Build → Build Solution** (Ctrl+Shift+B)

## Technical Details

This project was reverse-engineered from `winmine.exe` using tools such as IDA Pro. The following core modules were restored:

| Module | Description |
|--------|-------------|
| Window & Message Handling | `MainWinProc`, menu commands, mouse events |
| Minefield Logic | Mine placement, cell reveal, auto-expand blank cells, flagging, counting |
| UI Rendering | GDI drawing, bitmap resource loading, border rendering |
| Timer & Sound | Game timer, Win32 multimedia sound effects |
| Registry / INI | Persistent storage for game settings and high scores |
| Dialogs | Custom difficulty, high scores, player name, about |

## License

This project is licensed under the [MIT License](LICENSE).

# T13 Terminal Code Editor

A lightweight, blazing-fast, fullscreen-adaptive code editor built natively for UNIX-like environments (Linux/macOS) in pure C++.

## Core Features
- **Anchor-Positioned Cursor:** Employs precise hardware string rendering (`\033[s` and `\033[u`) to lock the blinking caret perfectly over characters, bypassing hidden ANSI color code offsets.
- **Dynamic Viewport Scrolling:** Supports vertical up/down sliding view frames to fluidly scroll through files past page boundaries.
- **Horizontal Left/Right Scrolling:** Automatically scrolls the screen horizontally when lines grow wider than your terminal borders.
- **Smart Modification Checking:** Features a "Dirty Flag" component—if no text edits are made, pressing `Ctrl + Q` exits back to the menu instantly without annoying prompts.
- **Smart Tabs:** Automatically converts Tab inputs into clean 4-space indents, with built-in macro-backspacing to delete 4 spaces at once.
- **C++ Syntax Highlighting:** Integrated real-time parsing engine dynamically colorizes keywords (red), comments (green), numbers (cyan), and strings (yellow).

## How to Compile & Run
Open your Linux/macOS terminal and run the following commands:

```bash
# Compile with high optimization
g++ -O3 txteditor.cpp -o "T13 text editor"

# Run the app
./"T13 text editor"
```

## Control Keys
- **Arrow Keys:** Move cursor in all 4 directions.
- **Enter:** Split line at cursor or insert new line.
- **Backspace:** Delete character / merge lines / smart tab delete.
- **Ctrl + Q:** Exit editor workspace back to main menu.

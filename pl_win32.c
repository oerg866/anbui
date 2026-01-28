/*
    AnbUI Miniature Text UI Lib for Burger Enjoyers(tm)

    pl_win32: Platform implementation for Win32 Consoles.

    (C) 2024 E. Voirin (oerg866) */

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <windows.h>
#include <unistd.h>
#include <conio.h>

#include "ad_priv.h"
#include "ad_hal.h"

static HANDLE               pl_win32_consoleHandle;
static COORD                pl_win32_consoleSize;
static DWORD                pl_win32_oldConsoleMode;
static CONSOLE_CURSOR_INFO  pl_win32_cursorInfo;

static void pl_win32_showCursor(bool show) {
    pl_win32_cursorInfo.bVisible = show;
    SetConsoleCursorInfo(pl_win32_consoleHandle, &pl_win32_cursorInfo);
}

void hal_initConsole(ad_ConsoleConfig *cfg) {
    CONSOLE_SCREEN_BUFFER_INFO screenInfo;

    pl_win32_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (GetConsoleScreenBufferInfo(pl_win32_consoleHandle, &screenInfo)) {
        cfg->width  = pl_win32_consoleSize.X = screenInfo.srWindow.Right - screenInfo.srWindow.Left + 1;
        cfg->height = pl_win32_consoleSize.Y = screenInfo.srWindow.Bottom - screenInfo.srWindow.Top + 1;
    } else {    
        cfg->width  = pl_win32_consoleSize.X = 80;  /* VGA text mode size 80x25 */
        cfg->height = pl_win32_consoleSize.Y = 25;
    }

    GetConsoleMode(pl_win32_consoleHandle, &pl_win32_oldConsoleMode);

    hal_restoreConsole();
}

void hal_restoreConsole(void) {
    DWORD newConsoleMode = pl_win32_oldConsoleMode & ~ENABLE_ECHO_INPUT & ~ENABLE_LINE_INPUT & ~ENABLE_VIRTUAL_TERMINAL_INPUT & ~ENABLE_WRAP_AT_EOL_OUTPUT;
    SetConsoleMode(pl_win32_consoleHandle, newConsoleMode);
    pl_win32_showCursor(false);
}

void hal_deinitConsole() {
    SetConsoleMode(pl_win32_consoleHandle, pl_win32_oldConsoleMode);
    pl_win32_showCursor(true);
}

inline void hal_setColor(uint8_t bg, uint8_t fg) {
    SetConsoleTextAttribute(pl_win32_consoleHandle, ((bg & 0x07) << 4) | (fg & 0x0F));
}

inline void hal_setCursorPosition(uint16_t x, uint16_t y) { 
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(pl_win32_consoleHandle, pos);
}

inline void hal_flush(void) {
    fflush(stdout); 
}

inline void hal_putString(const char *str) {
    fputs(str, stdout);
}

inline void hal_putChar(char c, size_t count) {
    while (count--) {
        putchar(c);
    }
}

uint32_t hal_getKey(void) {
    uint32_t c = (uint32_t) getch();

    /* Extended function */
    if (c == 0 || c == 0xe0) { c = ((uint32_t) getch()) << 16L; }

    switch (c) {
        case 0x0000001b: return AD_KEY_ESC;
        case 0x0000000d: return AD_KEY_ENTER;
        case 0x00490000: return AD_KEY_PGUP;
        case 0x00510000: return AD_KEY_PGDN;
        case 0x00480000: return AD_KEY_UP;
        case 0x00500000: return AD_KEY_DOWN;
        case 0x004B0000: return AD_KEY_LEFT;
        case 0x004D0000: return AD_KEY_RIGHT;
        
        default: return c;
    }    
}

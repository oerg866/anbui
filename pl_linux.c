/*
    AnbUI Miniature Text UI Lib for Burger Enjoyers(tm)

    pl_linux: Platform implementation for Linux/POSIX consoles using ANSI escape codes

    (C) 2024 E. Voirin (oerg866) */

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "ad_priv.h"
#include "ad_hal.h"

// Bold
#define PL_LINUX_CL_BLD "\033[1m"

#define PL_LINUX_CL_HID "\033[?25l"
#define PL_LINUX_CL_SHW "\033[?25h"

// Reset
#define PL_LINUX_CL_RST "\033[0m"

#define PL_LINUX_CH_ESCAPE '\033'
#define PL_LINUX_CH_SEQSTART '['

#define PL_LINUX_CURSOR_U     0x001b5b41
#define PL_LINUX_CURSOR_D     0x001b5b42
#define PL_LINUX_CURSOR_L     0x001b5b44
#define PL_LINUX_CURSOR_R     0x001b5b43

#define PL_LINUX_PAGE_U       0x1b5b357e
#define PL_LINUX_PAGE_D       0x1b5b367e

#define PL_LINUX_KEY_ENTER    0x0000000a
#define PL_LINUX_KEY_ESCAPE   0x00001b1b
#define PL_LINUX_KEY_ESCAPE2  0x0000001b

static struct termios s_originalTermios;
static const uint8_t colorLookup[]     = { 0, 4, 2, 6, 1, 5, 3, 7, 0, 4, 2, 6, 1, 5, 3, 7 };
static const uint8_t attributeLookup[] = { 22, 22, 22, 22, 22, 22, 22, 22, 1, 1, 1, 1, 1, 1, 1, 1 };

void hal_initConsole(ad_ConsoleConfig *cfg) {
    struct winsize w;

    cfg->width = 80;
    cfg->height = 25;

    tcgetattr(STDIN_FILENO, &s_originalTermios);

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        cfg->width = w.ws_col;
        cfg->height = w.ws_row;
    }

    hal_restoreConsole();
}

void hal_restoreConsole(void) {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    printf(PL_LINUX_CL_HID);
}

void hal_deinitConsole(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &s_originalTermios);
    printf(PL_LINUX_CL_SHW);
    printf("\n");
}

inline void hal_setColor(uint8_t bg, uint8_t fg) {
    AD_UNUSED_PARAMETER(attributeLookup);
    printf("\033[%u;%um\033[%u;%um", 0, colorLookup[bg] + 40, attributeLookup[fg], colorLookup[fg] + 30);
}

inline void hal_setCursorPosition(uint16_t x, uint16_t y) { 
    printf("\033[%u;%uH", (y + 1), (x + 1));
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
    int32_t ch = getchar();
    if ((ch & 0xff) == PL_LINUX_CH_ESCAPE)                      ch = (ch << 8) | (getchar() & 0xff);
    if ((ch & 0xff) == PL_LINUX_CH_SEQSTART)                    ch = (ch << 8) | (getchar() & 0xff);
    /* Special case for PGUp and Down, they have another 7e keycode at the end... */
    if ((ch & 0xff) == 0x35 || (ch & 0xff) == 0x36 )            ch = (ch << 8) | (getchar() & 0xff);

    switch (ch) {
        case PL_LINUX_KEY_ESCAPE:   return AD_KEY_ESC;
        case PL_LINUX_KEY_ESCAPE2:  return AD_KEY_ESC;
        case PL_LINUX_KEY_ENTER:    return AD_KEY_ENTER;
        case PL_LINUX_PAGE_U:       return AD_KEY_PGUP;
        case PL_LINUX_PAGE_D:       return AD_KEY_PGDN;
        case PL_LINUX_CURSOR_U:     return AD_KEY_UP;
        case PL_LINUX_CURSOR_D:     return AD_KEY_DOWN;
        case PL_LINUX_CURSOR_L:     return AD_KEY_LEFT;
        case PL_LINUX_CURSOR_R:     return AD_KEY_RIGHT;
        
        default: return (uint32_t)ch;
    }
}

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
#include <poll.h>
#include <sched.h>

#include "ad_priv.h"
#include "ad_hal.h"

// Bold
#define PL_LINUX_CL_BLD "\033[1m"

#define PL_LINUX_CL_HID "\033[?25l"
#define PL_LINUX_CL_SHW "\033[?25h"

// Reset
#define PL_LINUX_CL_RST "\033[0m"

#define PL_LINUX_CH_ESCAPE '\033'
#define PL_LINUX_CH_SEQSTART    0x00001b5b
#define PL_LINUX_F1234_SEQSTART 0x00001b4f

#define PL_LINUX_CURSOR_U     0x001b5b41
#define PL_LINUX_CURSOR_D     0x001b5b42
#define PL_LINUX_CURSOR_L     0x001b5b44
#define PL_LINUX_CURSOR_R     0x001b5b43

#define PL_LINUX_PAGE_U       0x001b5b35
#define PL_LINUX_PAGE_D       0x001b5b36

#define PL_LINUX_KEY_F1       0x001b4f50
#define PL_LINUX_KEY_F2       0x001b4f51
#define PL_LINUX_KEY_F3       0x001b4f52
#define PL_LINUX_KEY_F4       0x001b4f53
#define PL_LINUX_KEY_F5       0x1b5b3135
#define PL_LINUX_KEY_F6       0x1b5b3137
#define PL_LINUX_KEY_F7       0x1b5b3138
#define PL_LINUX_KEY_F8       0x1b5b3139
#define PL_LINUX_KEY_F9       0x1b5b3230
#define PL_LINUX_KEY_F10      0x1b5b3231
#define PL_LINUX_KEY_F11      0x1b5b3232
#define PL_LINUX_KEY_F12      0x1b5b3233

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

static inline bool keyAvailable(void) {
    struct pollfd pfd;

    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    pfd.revents = 0;

    return poll(&pfd, 1, 0) > 0;
}

static inline uint32_t getChar(void) {
    uint8_t ch = 0;
    while (read(STDIN_FILENO, &ch, 1) != 1) {
        sched_yield();
    }
    return ch;
}

// Shifts ch to the left by 8 bits and puts a new scancode in the lowest 8 bits
// Can be called with NULL to just consume a scancode if available
static inline void pushBackCharIfAvailable(uint32_t *ch) {
    if (keyAvailable()) {
        uint8_t new = getChar();
        if (ch != NULL) {
            *ch = (*ch << 8) | new;
        }
    }
}

uint32_t hal_getKey(void) {
    uint32_t ch = getChar();

    if (ch == PL_LINUX_KEY_ESCAPE2) {
        pushBackCharIfAvailable(&ch);

        // We are in an escape sequence, can be cursor, pgupdown, maybe f5-f12
        if (ch == PL_LINUX_CH_SEQSTART) {
            pushBackCharIfAvailable(&ch);

            uint8_t last = (ch & 0xff);

            if (last == 0x35 || last == 0x36) {
                /* Special case for PGUp and Down, they have another 7e keycode at the end... */
                pushBackCharIfAvailable(NULL);
            } else if (last == 0x31 || last == 0x32) {
                /* F5 - F12, get last character + extra 7e at the end*/
                pushBackCharIfAvailable(&ch);
                pushBackCharIfAvailable(NULL);
            }

        } else if (ch == PL_LINUX_F1234_SEQSTART) {
            // This is F1 to F4 potentially
            pushBackCharIfAvailable(&ch);
        }
    }

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
        case PL_LINUX_KEY_F1:       return AD_KEY_F1;
        case PL_LINUX_KEY_F2:       return AD_KEY_F2;
        case PL_LINUX_KEY_F3:       return AD_KEY_F3;
        case PL_LINUX_KEY_F4:       return AD_KEY_F4;
        case PL_LINUX_KEY_F5:       return AD_KEY_F5;
        case PL_LINUX_KEY_F6:       return AD_KEY_F6;
        case PL_LINUX_KEY_F7:       return AD_KEY_F7;
        case PL_LINUX_KEY_F8:       return AD_KEY_F8;
        case PL_LINUX_KEY_F9:       return AD_KEY_F9;
        case PL_LINUX_KEY_F10:      return AD_KEY_F10;
        case PL_LINUX_KEY_F11:      return AD_KEY_F11;
        case PL_LINUX_KEY_F12:      return AD_KEY_F12;
        
        default: return ch;
    }
}

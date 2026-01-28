/*
    AnbUI Miniature Text UI Lib for Burger Enjoyers(tm)

    pl_dos: Platform implementation for DOS & Compatible

    Supported compilers/runtimes:
        - OpenWatcom
        - Microsoft C 7.00

    (C) 2024 E. Voirin (oerg866) */

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dos.h>
#include <conio.h>

#include "ad_priv.h"
#include "ad_hal.h"

#if defined(_MSC_VER)
# include <intpack.h>
# include <standard.h>
# define callInt int86
#endif

#if defined(__WATCOMC__)
#include <i86.h>
# if defined(__386__) && defined(__DOS__)
#  define callInt int386
# else
#  define callInt int86
# endif
#endif

static uint8_t s_biosColor = 0x00;

static uint16_t s_consoleW;
static uint16_t s_consoleH;

static uint16_t s_oldCursorState = 0x0000;

#ifndef _far /* This is purely for VSCode to stop bugging out.... */
#define _far
#endif

typedef struct {
    char c;         /* character */
    uint8_t attr;   /* VGA color attribute */
} pl_dos_BiosChar;

static pl_dos_BiosChar _far *s_vgaMemory;
static pl_dos_BiosChar _far *s_vgaMemoryUpperBound;
static pl_dos_BiosChar _far *s_cursorPtr;
static inline void pl_dos_advanceCursor(uint16_t count) {
    s_cursorPtr = &s_cursorPtr[count];
    if (s_cursorPtr > s_vgaMemoryUpperBound) {
        s_cursorPtr = s_vgaMemoryUpperBound - s_consoleW;
    }
}

static inline void pl_dos_putString(const char *str, uint16_t length) {
    while (length--) {
        s_cursorPtr->c = *str;
        s_cursorPtr->attr = s_biosColor;
        s_cursorPtr++;
        str++;
    }
    pl_dos_advanceCursor(0);
}

static inline void pl_dos_setDisplayPage(uint8_t page) {
    __asm {
        mov ah, 0x05
        mov al, page
        int 0x10
    }
}

static inline void pl_dos_setCursorSize(uint16_t cursorSize) {
    __asm {
        mov ah, 0x01
        mov cx, cursorSize
        int 0x10
    }
}

static inline void pl_dos_saveCursorSize(void) {
    uint16_t retCx = 0;
    __asm {
        mov ah, 0x03
        mov bh, 0x00
        int 0x10
        mov retCx, cx
    }
    s_oldCursorState = retCx;
}

static inline void pl_dos_disableCursor(void) {
    pl_dos_setCursorSize(0x2000);
}

static inline void pl_dos_restoreCursor(void) {
    pl_dos_setCursorSize(s_oldCursorState);
}

static bool pl_dos_getVGADynamicStateTable(uint8_t _far *table) {
    uint16_t fpSeg = FP_SEG(table);
    uint16_t fpOff = FP_OFF(table);
    uint8_t  retAl = 0;
    __asm {
        mov ah, 0x1b
        mov bx, 0x0000
        mov es, fpSeg
        mov di, fpOff
        int 0x10
        mov retAl, al
    };
    return retAl == 0x1B;
}

void hal_initConsole(ad_ConsoleConfig *cfg) {
    uint8_t   dynamicVGAInfo[64];
    uint16_t *biosWidth  = (uint16_t *) &dynamicVGAInfo[0x05];
    uint8_t  *biosHeight = (uint8_t *)  &dynamicVGAInfo[0x22];

    pl_dos_saveCursorSize();

    /* Get Video BIOS Dynamic Functionality State Table(tm) */
    if (pl_dos_getVGADynamicStateTable((uint8_t _far *) dynamicVGAInfo)){
        s_consoleW  = *biosWidth;
        s_consoleH  = (uint16_t) *biosHeight;
    } else {
        s_consoleW  = 80;
        s_consoleH  = 25;
    }

    cfg->width  = s_consoleW;
    cfg->height = s_consoleH;

    memset(s_printBuffer, 0x00, sizeof(s_printBuffer));

    hal_restoreConsole();

}

void hal_restoreConsole(void) {
    s_vgaMemory = MK_FP(0xB800, 0x0000);
    s_vgaMemoryUpperBound = s_vgaMemory + s_consoleH * s_consoleW * 2;
    s_cursorPtr = s_vgaMemory;
    
    _fmemset(s_vgaMemory, 0, s_consoleH * s_consoleW * 2);

    pl_dos_setDisplayPage(0x00);
    
    pl_dos_disableCursor();
}

void hal_deinitConsole(void) {
    pl_dos_restoreCursor();
}

void hal_setColor(uint8_t bg, uint8_t fg) {
    s_biosColor = (bg & 0x07) << 4 | fg & 0x0F;
}

void hal_setCursorPosition(uint16_t x, uint16_t y) {
    s_cursorPtr = &s_vgaMemory[x];
    while (y--) {
        s_cursorPtr = &s_cursorPtr[s_consoleW];
    }
}

void hal_putString(const char *str) {
    pl_dos_putString(str, (uint16_t) strlen(str));
}

void hal_putChar(char c, size_t count) {
    pl_dos_BiosChar bc;
    bc.c = c;
    bc.attr = s_biosColor;
    while (count--) {
        *s_cursorPtr++ = bc;
    }
    pl_dos_advanceCursor(0);
}    

void hal_flush(void) {
    /* Nothing on DOS, it always displays everything immediately */
}

uint32_t hal_getKey(void) {
    uint32_t c = (uint32_t) getch();

    /* Extended function */
    if (c == 0) { c = ((uint32_t) getch()) << 16L; }
   
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

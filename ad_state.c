/*
    AnbUI Miniature Text UI Lib for Burger Enjoyers(tm)

    Intermediate rendering layer.
    The purpose of this is to always keep track of the
    internal contents of the screen, so that we can
    effectively save and restore screen contents at
    will.

    E.g. for important error popups, etc.

    These functions then get forwarded to the 

    (C) 2026 E. Voirin (oerg866) */

#include "anbui.h"
#include "ad_priv.h"
#include "ad_hal.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

typedef struct {
    uint8_t fg : 4; 
    uint8_t bg : 4;
} ad_Color;

typedef struct {
    ad_Color color;
    uint8_t ascii;
} ad_Char;

typedef struct {
    ad_Color color;
    uint16_t width;
    uint16_t height;
    uint16_t x;
    uint16_t y;
    size_t totalChars;
    size_t bufSize;
    ad_Char *data;
    ad_Char *dataLimit;
    ad_Char *data_backup;
    uint16_t x_backup;
    uint16_t y_backup;
    ad_Color color_backup;
} ad_ScreenState;

static ad_ScreenState state;

bool ad_initConsole(ad_ConsoleConfig *cfg) {
    hal_initConsole(cfg);

    memset(&state, 0, sizeof(ad_ScreenState));

    state.width = cfg->width;
    state.height = cfg->height;
    state.bufSize = state.width * state.height * sizeof(ad_Char);

    state.data = calloc(1, state.bufSize);
    state.data_backup = calloc(1, state.bufSize);
    state.dataLimit = &state.data[state.width * state.height];

    return (state.bufSize != 0 && state.data != NULL && state.data_backup == NULL);
}

void ad_deinitConsole(void) {
    hal_deinitConsole();
    free(state.data);
    free(state.data_backup);
}

void ad_screenSaveState(void) {
    FILE *a = fopen("state.bin", "wb");
    fwrite(state.data, 1, state.bufSize, a);
    fclose(a);
    memcpy(state.data_backup, state.data, state.bufSize);
    state.x_backup = state.x;
    state.y_backup = state.y;
    state.color_backup = state.color;
}

void ad_screenLoadState(void) {
    size_t totalChars = state.width * state.height;
    size_t i;

    memcpy(state.data, state.data_backup, state.bufSize);
    hal_setCursorPosition(0, 0);

    for (i = 0; i < totalChars; i++) {
        hal_setColor(state.data[i].color.bg, state.data[i].color.fg);
        hal_putChar(state.data[i].ascii, 1);
    }

    ad_setColor(state.color_backup.bg, state.color_backup.fg);
    ad_setCursorPosition(state.x_backup, state.y_backup);
}

#define ad_drawPtr() (&state.data[state.y * state.width + state.x])
#define ad_cr()             do { state.x = 0; }                                         while (0)
#define ad_lf()             do { ad_cr(); state.y++; }                                  while (0)
#define ad_advanceCursor()  do { state.x++; if (state.x > state.width) { ad_lf(); }; }   while (0)

void ad_setColor(uint8_t bg, uint8_t fg) {
    hal_setColor(bg, fg);
    state.color.bg = bg;
    state.color.fg = fg;
}

void ad_setCursorPosition(uint16_t x, uint16_t y) {
    hal_setCursorPosition(x, y);
    state.x = x;
    state.y = y;
}

void ad_putChar(char c, size_t count) {
    ad_Char *drawPtr = ad_drawPtr();
    hal_putChar(c, count);
    while (count-- && drawPtr < state.dataLimit) {
        assert(c >= ' ');
        drawPtr->color = state.color;
        drawPtr->ascii = c;
        drawPtr++;
        ad_advanceCursor();
    }
}

void ad_putString(const char *str) {
    while(*str) {
        ad_putChar(*str, 1);
        str++;
    }
}

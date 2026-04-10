/*
    AnbUI Miniature Text UI Lib for Burger Enjoyers(tm)

    ad_text: Text / String / Drawing code

    Tip of the day: Cheese your burger before the burger cheeses you.
    
    Burger first, cheese later, but always burger the cheese.

    (C) 2024 E. Voirin (oerg866) */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include "anbui.h"
#include "ad_priv.h"
#include "ad_hal.h"


void ad_fixedTextElementAssign (ad_FixedTextElement el, const char *text) {
    size_t length = AD_MIN(AD_TEXT_ELEMENT_SIZE-1, strlen(text));
    memcpy(el, text, length);
    el[length] = 0x00;
}

void ad_fixedTextElementAssignFormatted(ad_FixedTextElement el, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(el, AD_TEXT_ELEMENT_SIZE, format, args);
    el[AD_TEXT_ELEMENT_SIZE-1] = 0x00;
    va_end(args);
}

bool ad_stringArrayAddString(ad_StringArray *arr, const char *strStart, size_t len) {
    char **newAlloc = NULL;
    size_t newAllocSize = 0;
    char *newString;

    AD_RETURN_ON_NULL(arr, false);
    AD_RETURN_ON_NULL(strStart, false);

    /* try to make a new string first */
    newString = calloc(1, len + 1);

    AD_RETURN_ON_NULL(newString, false);

    /* String alloc OK, copy data */
    memcpy(newString, strStart, len);

    /* Now try to expand the string pointer array */
    newAllocSize = (arr->count + 1) * sizeof(char *);
    newAlloc = realloc(arr->items, newAllocSize);

    /* Failure; dispose of the string we just allocated because we don't have an array to hold its pointer in */
    if (newAlloc == NULL) {
        free(newAlloc);
        return false;
    }

    /* All good, assign the new data */
    arr->items = newAlloc;
    arr->count++;
    arr->items[arr->count - 1] = newString;
    return true;
}

char *ad_stringArrayGet(ad_StringArray *arr, size_t index) {
    AD_RETURN_ON_NULL(arr, NULL);
    if (index > arr->count) return NULL;
    return arr->items[index];
}

char *ad_stringArrayGetLast(ad_StringArray *arr) {
    return ad_stringArrayGet(arr, arr->count - 1);
}


size_t ad_stringArrayGetLongestLength(ad_StringArray *arr) {
    size_t max = 0;
    size_t curLen;
    size_t curItem;

    AD_RETURN_ON_NULL(arr, 0);

    for (curItem = 0; curItem < arr->count; curItem++) {
        curLen = strlen(arr->items[curItem]);
        if (curLen > max) max = curLen;
    }

    return max;
}


bool ad_stringArrayCreateFromString(ad_StringArray *dst, const char *str) {
    const char *upperBound;
    const char *curPos = str;

    dst->capacity = 0;
    dst->count = 0;
    dst->items = NULL;
    
    upperBound = str + strlen(str);

    while (curPos < upperBound) {
        /* String is from current position until newline */
        const char *curEnd = strchr(curPos, '\n');
        const size_t curLen = (curEnd != NULL) ? (size_t) (curEnd - curPos) : strlen(curPos);
        
        if (!ad_stringArrayAddString(dst, curPos, curLen)) {
            return false;
        }

        /* Deal with annoying \r\n stuff */
        if (curLen > 0 && curPos[curLen-1] == '\r') {
            ad_stringArrayGetLast(dst)[curLen-1] = 0x00;
        }

        /* Next string starts after \n */
        curPos += curLen + 1;
        
    }

    return true;
}

void ad_stringArrayDestroy(ad_StringArray *arr) {
    size_t i;

    if (arr == NULL) return;
    if (arr->items == NULL) return;
    for (i = 0; i < arr->count; i++) {
        if (arr->items[i] != NULL) free(arr->items[i]);
    }
    free(arr->items);
}

void ad_displayStringCropped(const char *str, uint16_t x, uint16_t y, size_t maxLen, uint8_t bg, uint8_t fg) {
    size_t strLen = strlen(str);
    size_t paddingRight = maxLen - strLen;

    ad_setColor(bg, fg);
    ad_setCursorPosition(x, y);

    if (strLen > maxLen) {
        maxLen -= 3;
        while (maxLen--) {
            ad_putChar(*str++, 1);
        }
        ad_putString("...");
    } else {
        ad_putString(str);
        ad_putChar(' ', paddingRight);
    }
}

void ad_displayStringArrayRange(const ad_StringArray *arr, size_t index, size_t count, uint16_t x, uint16_t y, size_t maximumWidth) {
    size_t i;
    for (i = index; i < count; i++) {
        if (index >= arr->count) return;
        ad_displayStringCropped(arr->items[i], x, y, maximumWidth, ad_s_con.objectBg, ad_s_con.objectFg);
        y++;
    }
    hal_flush();
}

void ad_displayStringArray(const ad_StringArray *arr, uint16_t x, uint16_t y, size_t maximumHeight, size_t maximumWidth) {
    ad_displayStringArrayRange(arr, 0, AD_MIN(maximumHeight, arr->count), x, y, maximumWidth);
}

void ad_printCenteredText(const char* str, uint16_t x, uint16_t y, uint16_t w, uint8_t colBg, uint8_t colFg) {
    size_t      strLen      = strlen(str);
    uint16_t    paddingL;
    uint16_t    paddingR;

    if (strLen < w) {
        ad_setCursorPosition(x, y);
        ad_setColor(colBg, colFg);

        paddingL = ad_getPadding(w, strLen);
        paddingR = w - strLen - paddingL;

        ad_putChar(' ', paddingL);
        ad_putString(str);
        ad_putChar(' ', paddingR);
    } else {
        ad_displayStringCropped(str, x, y, w, colBg, colFg);
    }

    hal_flush();
}


void ad_drawBackground(const char *title) {
    size_t y;

    ad_printCenteredText(title, 0, 0, ad_s_con.width, ad_s_con.headerBg, ad_s_con.headerFg);

    for (y = 1; y < ad_s_con.height; y++) {
        ad_fill(ad_s_con.width, ' ', 0, y, ad_s_con.backgroundFill, 0);
    }

    hal_flush();
}

void ad_fill(size_t length, char fill, uint16_t x, uint16_t y, uint8_t colBg, uint8_t colFg) {
    ad_setCursorPosition(x, y);
    ad_setColor(colBg, colFg);
    ad_putChar(fill, length);
}

size_t ad_getPadding(size_t totalLength, size_t lengthToPad) {
    return (totalLength - lengthToPad) / 2;
}

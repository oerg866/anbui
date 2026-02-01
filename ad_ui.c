/*
    AnbUI Miniature Text UI Lib for Burger Enjoyers(tm)

    ad_ui: UI Components Code

    Tip of the day: Ever tried burgering your burger with burger cheese
    on top of burger? The secret is in the cheese burgering itself.
    When you cheese the burger, the burger burgers harder, and the cheese?

    It just cheeses more.

    (C) 2024 E. Voirin (oerg866) */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "anbui.h"
#include "ad_priv.h"
#include "ad_hal.h"

static void ad_textFileBoxDestroy(ad_TextFileBox *tfb);

static void ad_menuSelectItemAndDraw(ad_Menu *menu, size_t newSelection) {
    assert(menu);
    ad_displayStringCropped(menu->items[menu->currentSelection].text,   menu->itemX, menu->itemY + menu->currentSelection, menu->itemWidth, ad_s_con.objectBg, ad_s_con.objectFg);
    ad_displayStringCropped(menu->items[newSelection].text,             menu->itemX, menu->itemY + newSelection,           menu->itemWidth, ad_s_con.objectFg, ad_s_con.objectBg);
    menu->currentSelection = newSelection;
    hal_flush();
}

static bool ad_menuPaint(ad_Menu *menu) {
    size_t maximumContentWidth = ad_objectGetMaximumContentWidth();
    size_t maximumPromptWidth = 0;
    size_t maximumItemWidth;
    size_t windowContentWidth;
    size_t promptHeight = (menu->prompt != NULL) ? menu->prompt->lineCount : 0;
    
    AD_RETURN_ON_NULL(menu, false);

    /* Get the length of the longest menu item */
    maximumItemWidth = ad_textElementArrayGetLongestLength(menu->itemCount, menu->items);
    windowContentWidth = maximumItemWidth + 2 * AD_MENU_ITEM_PADDING_H;
    
    /* Factor in the prompt length into window width calculation */
    if (menu->prompt) {
        maximumPromptWidth = ad_textElementArrayGetLongestLength(menu->prompt->lineCount, menu->prompt->lines);
        windowContentWidth = AD_MAX(windowContentWidth, maximumPromptWidth);
    }

    /* Cap it at the maximum width of displayable content in an Object */
    windowContentWidth = AD_MIN(windowContentWidth, maximumContentWidth);
    menu->itemWidth = windowContentWidth - 2 * AD_MENU_ITEM_PADDING_H;

    ad_objectInitialize(&menu->object, windowContentWidth, menu->itemCount + 1 + promptHeight); /* +2 because of prompt*/
    ad_objectPaint(&menu->object);

    menu->itemX = ad_objectGetContentX(&menu->object);
    menu->itemY = ad_objectGetContentY(&menu->object);

    /* Print prompt if it exists */
    if (menu->prompt) {   
        ad_displayTextElementArray(menu->itemX, menu->itemY, ad_objectGetContentWidth(&menu->object), menu->prompt->lineCount, menu->prompt->lines);
        menu->itemY += 1 + menu->prompt->lineCount;
    }

    /* Print the menu items */

    menu->itemX += AD_MENU_ITEM_PADDING_H;

    ad_displayTextElementArray(menu->itemX, menu->itemY, menu->itemWidth, menu->itemCount, menu->items);

    ad_menuSelectItemAndDraw(menu, 0);

    return true;
}

ad_Menu *ad_menuCreate(const char *title, const char *prompt, bool cancelable) {
    ad_Menu *menu = calloc(1, sizeof(ad_Menu));
    assert(menu);

    menu->cancelable = cancelable;
    menu->prompt = ad_multiLineTextCreate(prompt);
    
    ad_textElementAssign(&menu->object.footer, menu->cancelable ? AD_FOOTER_MENU_CANCELABLE : AD_FOOTER_MENU);
    ad_textElementAssign(&menu->object.title, title);

    return menu;
}

void ad_menuAddItemFormatted(ad_Menu *obj, const char *format, ...) {
    va_list args;

    assert(obj);    
    obj->itemCount++;
    obj->items = ad_textElementArrayResize(obj->items, obj->itemCount);
    assert(obj->items);

    va_start(args, format);
    vsnprintf(obj->items[obj->itemCount-1].text, AD_TEXT_ELEMENT_SIZE, format, args);
    va_end(args);
}

bool ad_menuGetItemText(ad_Menu *obj, size_t index, char *dst, size_t dstSize) {
    AD_RETURN_ON_NULL(obj, false);
    AD_RETURN_ON_NULL(dst, false);
    if (index > obj->itemCount) return false;
    strncpy(dst, obj->items[index].text, dstSize - 1);
    dst[dstSize - 1] = 0x00;
    return true;    
}

inline size_t ad_menuGetItemCount(ad_Menu *menu) {
    return menu ? menu->itemCount : 0;
}

int32_t ad_menuExecute(ad_Menu *menu) {
    uint32_t ch;

    ad_menuPaint(menu);

    while (true) {
        ch = hal_getKey();

        if          (ch == AD_KEY_UP) {
            ad_menuSelectItemAndDraw(menu, (menu->currentSelection > 0) ? menu->currentSelection - 1 : menu->itemCount - 1);
        } else if   (ch == AD_KEY_DOWN) {
            ad_menuSelectItemAndDraw(menu, (menu->currentSelection + 1) % menu->itemCount);
        } else if   (ch == AD_KEY_ENTER) {
            return menu->currentSelection;
        } else if   (menu->cancelable && (ch == AD_KEY_ESC)) {
            return AD_CANCELED;
        } 
#if DEBUG
        else {
            printf("unhandled key: %08x\n", ch);
        }
#endif
    }
}

void ad_menuDestroy(ad_Menu *menu) {
    if (menu) {
        ad_objectUnpaint(&menu->object);
        ad_multiLineTextDestroy(menu->prompt);
        free(menu->items);
        free(menu);
    }
}

static int32_t ad_menuExecuteDirectlyInternal(const char *title, bool cancelable, size_t optionCount, const char *options[], const char *prompt) {
    ad_Menu        *menu = NULL;
    int32_t         ret  = 0;
    size_t          idx;

    AD_RETURN_ON_NULL(options, AD_ERROR);
    AD_RETURN_ON_NULL(prompt, AD_ERROR);

    menu = ad_menuCreate(title, prompt, cancelable);
    AD_RETURN_ON_NULL(menu, AD_ERROR);

    for (idx = 0; idx < optionCount; idx++) {
        ad_menuAddItemFormatted(menu, "%s", options[idx]);
    }

    ret = ad_menuExecute(menu);
    ad_menuDestroy(menu);

    return ret;
}

int32_t ad_menuExecuteDirectly(const char *title, bool cancelable, size_t optionCount, const char *options[], const char *promptFormat, ...) {
    char tmpPrompt[1024];
    va_list args;
    va_start(args, promptFormat);
    vsnprintf(tmpPrompt, sizeof(tmpPrompt), promptFormat, args);
    va_end(args);
    return ad_menuExecuteDirectlyInternal(title, cancelable, optionCount, options, tmpPrompt);
}

int32_t ad_yesNoBox(const char *title, bool cancelable, const char *promptFormat, ...) {
    const char *options[] = {"Yes", "No"};
    char tmpPrompt[1024];
    va_list args;
    va_start(args, promptFormat);
    vsnprintf(tmpPrompt, sizeof(tmpPrompt), promptFormat, args);
    va_end(args);
    return ad_menuExecuteDirectlyInternal(title, cancelable, AD_ARRAY_SIZE(options), options, tmpPrompt);
}

int32_t ad_okBox(const char *title, bool cancelable, const char *promptFormat, ...) {
    const char *options[] = {"OK"};
    char tmpPrompt[1024];
    va_list args;
    va_start(args, promptFormat);
    vsnprintf(tmpPrompt, sizeof(tmpPrompt), promptFormat, args);
    va_end(args);
    return ad_menuExecuteDirectlyInternal(title, cancelable, AD_ARRAY_SIZE(options), options, tmpPrompt);
}

static size_t ad_progressBoxGetLongestLabelLength(ad_ProgressBox *pb) {
    size_t i = 0;
    size_t length = 0;
    for (i = 0; i < pb->itemCount; i++) {
        length = AD_MAX(length, strlen(pb->items[i].label.text));
    }
    return length;
}

bool ad_progressBoxPaint (ad_ProgressBox *pb) {
    size_t expectedWidth;
    size_t promptWidth;
    size_t promptHeight;
    size_t labelWidth;
    size_t pbIndex;
    
    AD_RETURN_ON_NULL(pb, false);

    /* Get the length of the longest Prompt line */
    promptHeight = (pb->prompt != NULL) ? pb->prompt->lineCount : 0;
    promptWidth = (pb->prompt != NULL) ? ad_textElementArrayGetLongestLength(pb->prompt->lineCount, pb->prompt->lines) : 0;

    labelWidth = ad_progressBoxGetLongestLabelLength(pb);

    /* Standard width = 50 + margin
       Maximum width = text length + margin, capped to maximum object width */
    expectedWidth = AD_MAX(50, promptWidth);

    ad_objectInitialize(&pb->object, expectedWidth, promptHeight + pb->itemCount + 1);
    ad_objectPaint(&pb->object);

    pb->labelX = ad_objectGetContentX(&pb->object);
    pb->boxY = ad_objectGetContentY(&pb->object);
    pb->boxWidth = ad_objectGetContentWidth(&pb->object);

    if (pb->itemCount == 1) {
        pb->boxX = pb->labelX;
    } else {
        /* If this is a multi-item box, the box starts *after* the label. */
        pb->boxX = pb->labelX + labelWidth + 1 + 1;
        pb->boxWidth -= (labelWidth + 1);
    }

    if (pb->prompt) {   
        ad_displayTextElementArray(pb->labelX, pb->boxY, ad_objectGetContentWidth(&pb->object), pb->prompt->lineCount, pb->prompt->lines);
        pb->boxY += 1 + pb->prompt->lineCount;
    }

    /* Draw the actual bar(s) */
    for (pbIndex = 0; pbIndex < pb->itemCount; pbIndex++) {
        uint16_t y = pb->boxY + pbIndex;
        ad_fill(pb->boxWidth,                   ad_s_con.progressChar, pb->boxX, pb->boxY + pbIndex, ad_s_con.progressBlankBg, ad_s_con.progressBlankFg);
        ad_fill(pb->items[pbIndex].currentX,    ad_s_con.progressChar, pb->boxX, pb->boxY + pbIndex, ad_s_con.progressFillBg,  ad_s_con.progressFillFg);

        /* Draw the bar labels ONLY if it's a multi-item box */
        if (pb->itemCount > 1) {
            ad_displayStringCropped(pb->items[pbIndex].label.text, pb->labelX, y, labelWidth, ad_s_con.objectBg, ad_s_con.objectFg);
        }
    }

    hal_flush();

    return true;
}

ad_ProgressBox *ad_progressBoxSingleCreate(const char *title, uint32_t maxProgress, const char *promptFormat, ...) {
    ad_ProgressBox *pb = NULL;
    char tmpPrompt[1024];
    va_list args;
    va_start(args, promptFormat);
    vsnprintf(tmpPrompt, sizeof(tmpPrompt), promptFormat, args);
    va_end(args);

    pb = ad_progressBoxMultiCreate(title, "%s", tmpPrompt);

    if (pb) {
        ad_progressBoxAddItem(pb, "", maxProgress);
        ad_progressBoxPaint(pb);
    }
    
    return pb;
}

ad_ProgressBox *ad_progressBoxMultiCreate(const char *title, const char *promptFormat, ...) {
    ad_ProgressBox *pb = NULL;
    char tmpPrompt[1024];
    va_list args;

    AD_RETURN_ON_NULL(title, NULL);
    AD_RETURN_ON_NULL(promptFormat, NULL);
    pb = calloc(1, sizeof(ad_ProgressBox));
    AD_RETURN_ON_NULL(pb, NULL);

    va_start(args, promptFormat);
    vsnprintf(tmpPrompt, sizeof(tmpPrompt), promptFormat, args);
    va_end(args);

    pb->prompt = ad_multiLineTextCreate(tmpPrompt);
    ad_textElementAssign(&pb->object.title, title);
    return pb;
}

void ad_progressBoxDestroy(ad_ProgressBox *pb) {
    if (pb) {
        ad_objectUnpaint(&pb->object);
        ad_multiLineTextDestroy(pb->prompt);
        free(pb->items);
        free(pb);
    }
}

void ad_progressBoxMultiUpdate(ad_ProgressBox *pb, size_t index, uint32_t progress) {
    uint16_t newX;
    uint16_t newPaintLength;
    ad_Progress *prog;

    if (pb == NULL) {
        return;
    }

    prog = &pb->items[index];

    /*  round / lround for values > 1 in MUSL gets clipped to 1.0 ?????? am I stupid?
        Anyway this hack is here until I get some sleep.. */
    newX = AD_ROUND_HACK_WTF(uint16_t, ((double) pb->boxWidth * (double) progress) / ((double) prog->outOf));

    if (newX == prog->currentX) {
        return;
    }

    newPaintLength = newX - prog->currentX;

    ad_setCursorPosition(pb->boxX + prog->currentX, pb->boxY + index);
    ad_setColor(ad_s_con.progressFillBg, ad_s_con.progressFillFg);

    ad_putChar(ad_s_con.progressChar, newPaintLength);

    hal_flush();
    
    prog->currentX = newX;
}

void ad_progressBoxUpdate(ad_ProgressBox *pb, uint32_t progress) {
    ad_progressBoxMultiUpdate(pb, 0, progress);
}

void ad_progressBoxSetCharAndColor(char fillChar, uint8_t colorBlankBg, uint8_t colorBlankFg, uint8_t colorFillBg, uint8_t colorFillFg) {
    ad_s_con.progressChar       = fillChar;
    ad_s_con.progressBlankBg    = colorBlankBg;
    ad_s_con.progressBlankFg    = colorBlankFg;
    ad_s_con.progressFillBg     = colorFillBg;
    ad_s_con.progressFillFg     = colorFillFg;
}

static ad_Progress *ad_progressArrayResize(ad_Progress *ptr, size_t newCount) {
    ptr = realloc(ptr, newCount * sizeof(ad_Progress));
    if (ptr) memset(&ptr[newCount-1], 0, sizeof(ad_Progress));
    return ptr;
}

void ad_progressBoxAddItem(ad_ProgressBox *obj, const char *label, uint32_t maxProgress) {
    if (obj == NULL || label == NULL ) return;

    obj->itemCount++;
    obj->items = ad_progressArrayResize(obj->items, obj->itemCount);
    
    assert(obj->items);

    obj->items[obj->itemCount-1].outOf = maxProgress;
    ad_textElementAssign(&obj->items[obj->itemCount-1].label, label);
}

void ad_progressBoxSetMaxProgress(ad_ProgressBox *obj, size_t index, uint32_t maxProgress) {
    if (obj == NULL || index >= obj->itemCount) return;
    obj->items[index].outOf = maxProgress;
}

static inline void ad_textFileBoxRedrawLines(ad_TextFileBox *tfb) {
    ad_displayTextElementArray(tfb->textX, tfb->textY, (size_t) tfb->lineWidth, (size_t) tfb->linesOnScreen, &tfb->lines->lines[tfb->currentIndex]);
}

static bool ad_textFileBoxPaint(ad_TextFileBox *tfb) {
    size_t lineWidth = 0;

    AD_RETURN_ON_NULL(tfb, false);

    /* Get the length of the longest Text line */
    lineWidth = ad_textElementArrayGetLongestLength(tfb->lines->lineCount, tfb->lines->lines);

    ad_objectInitialize(&tfb->object, lineWidth, tfb->lines->lineCount);

    tfb->textX = ad_objectGetContentX(&tfb->object);
    tfb->textY = ad_objectGetContentY(&tfb->object);
    tfb->lineWidth = ad_objectGetContentWidth(&tfb->object);
    tfb->linesOnScreen = ad_objectGetContentHeight(&tfb->object);
    tfb->highestIndex = tfb->lines->lineCount - tfb->linesOnScreen;

    ad_objectPaint(&tfb->object);

    ad_textFileBoxRedrawLines(tfb);

    return true;    
}

static ad_TextFileBox *ad_textFileBoxCreate(const char *title, const char *fileName) {
    ad_TextFileBox *tfb         = NULL;
    FILE           *inFile      = NULL;
    long            fileSize    = 0;
    char           *fileBuffer  = NULL;
    size_t          bytesRead   = 0;
    
    AD_RETURN_ON_NULL(title, NULL);
    AD_RETURN_ON_NULL(fileName, NULL);

    inFile = fopen(fileName, "rb");

    AD_RETURN_ON_NULL(inFile, NULL);

    /* Get File Size */

    fseek(inFile, 0, SEEK_END);
    fileSize = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);

    if (ferror(inFile) != 0 || fileSize <= 0) {
        goto error;
    }

    /* Read whole file into buffer */

    fileBuffer = malloc((size_t) fileSize + 1);
    fileBuffer[fileSize] = 0x00;

    while (bytesRead < (size_t) fileSize) {
        bytesRead += fread(&fileBuffer[bytesRead], 1, (size_t) fileSize - bytesRead, inFile);
        if (ferror(inFile)) {
            goto error;
        }
    }

    fclose(inFile);
    inFile = NULL;

    /* OK now we can actually do something with this */

    tfb = calloc(1, sizeof(ad_TextFileBox));

    if (tfb == NULL) {
        goto error;
    }
    
    ad_textElementAssign(&tfb->object.title, title);
    ad_textElementAssign(&tfb->object.footer, AD_FOOTER_TEXTFILEBOX);
    
    tfb->lines = ad_multiLineTextCreate(fileBuffer);

    if (tfb->lines == NULL) {
        goto error;
    }

    ad_textFileBoxPaint(tfb);

    free(fileBuffer);

    return tfb;

error:
    fclose(inFile);
    free(fileBuffer);
    ad_textFileBoxDestroy(tfb);
    return NULL;

}

static void ad_textFileBoxMove(ad_TextFileBox *tpb, int32_t positionsToMoveV) {
    tpb->currentIndex += positionsToMoveV;

    /* Clip in both directions */
    tpb->currentIndex = AD_MAX(tpb->currentIndex, 0);
    tpb->currentIndex = AD_MIN(tpb->currentIndex, tpb->highestIndex);

    ad_textFileBoxRedrawLines(tpb);
}

static int32_t ad_textFileBoxExecute(ad_TextFileBox *tfb) {
    uint32_t ch;

    AD_RETURN_ON_NULL(tfb, AD_ERROR);

    ad_textFileBoxRedrawLines(tfb);

    while (true) {
        ch = hal_getKey();

        if          (ch == AD_KEY_UP) {
            ad_textFileBoxMove(tfb, -1);
        } else if   (ch == AD_KEY_DOWN) {
            ad_textFileBoxMove(tfb, +1);
        } else if   (ch == AD_KEY_PGUP) {
            ad_textFileBoxMove(tfb, -tfb->linesOnScreen);
        } else if   (ch == AD_KEY_PGDN) {
            ad_textFileBoxMove(tfb, +tfb->linesOnScreen);
        } else if   (ch == AD_KEY_ENTER) {
            return 0;
        } /*else if   (menu->cancelable && (ch == AD_KEY_ESCAPE || ch == AD_KEY_ESCAPE2)) {
            return AD_CANCELED;
        } */
#if 0
        else {
            printf("unhandled key: %08x\n", ch);
        }
#endif
    }

    return 0;
}

static void ad_textFileBoxDestroy(ad_TextFileBox *tfb) {
    if (tfb) {
        ad_multiLineTextDestroy(tfb->lines);
        ad_objectUnpaint(&tfb->object);
        free(tfb);
    }
}

int32_t ad_textFileBox(const char *title, const char *fileName) {
    ad_TextFileBox *tfb = ad_textFileBoxCreate(title, fileName);
    int ret;
    AD_RETURN_ON_NULL(tfb, AD_ERROR);
    ret = ad_textFileBoxExecute(tfb);
    ad_textFileBoxDestroy(tfb);
    return ret;
}

#if defined(AD_HAL_HAS_POPEN)

static void ad_commandBoxRedraw(const ad_TextElement *lines, size_t lineCount, uint16_t contentWidth, size_t index, uint16_t x, uint16_t y) {
    size_t curLine;

    for (curLine = 0; curLine < lineCount; curLine++) {
        ad_displayStringCropped(lines[index % lineCount].text, x, y + curLine, contentWidth, ad_s_con.objectBg, ad_s_con.objectFg);
        index++;
    }
}

#ifndef WEXITSTATUS
#define WEXITSTATUS(x) ((x) & 0xff)
#endif

int32_t ad_runCommandBox(const char *title, const char *command) {
    ad_Object       obj;
    ad_TextElement *lines = NULL;
    size_t          visibleLines = ad_objectGetMaximumContentHeight() * 60 / 100;
    size_t          lineWidth = ad_objectGetMaximumContentWidth() * 80 / 100;
    size_t          lineDisplayIndex = 0;
    size_t          lineWriteIndex = 0;
    uint16_t        outputX = 0;
    uint16_t        outputY = 0;
    FILE*           pipe = NULL;
    char           *newLineChar = NULL;

    AD_RETURN_ON_NULL(command, AD_ERROR);
    AD_RETURN_ON_NULL(title, AD_ERROR);

    ad_textElementAssign(&obj.title, title);
    ad_textElementAssignFormatted(&obj.footer, "Running: '%s'...", command);
    ad_objectInitialize(&obj, lineWidth, visibleLines);

    lineWidth = ad_objectGetContentWidth(&obj);
    visibleLines = ad_objectGetContentHeight(&obj);
    lineWriteIndex = visibleLines - 1; /* Start writing at bottommost line */
    outputX = ad_objectGetContentX(&obj);
    outputY = ad_objectGetContentY(&obj);

    ad_objectPaint(&obj);

    lines = calloc(visibleLines, sizeof(ad_TextElement));
    AD_RETURN_ON_NULL(lines, AD_ERROR);

    /* Run the actual command */
    pipe = popen(command, "r");

    while (fgets(lines[lineWriteIndex % visibleLines].text, AD_TEXT_ELEMENT_SIZE, pipe) != NULL) {
        /* Fix newline */
        newLineChar = strchr(lines[lineWriteIndex % visibleLines].text, '\n');
        if (newLineChar) *newLineChar = 0x00;
        lineWriteIndex++;
        lineDisplayIndex++;
        ad_commandBoxRedraw(lines, visibleLines, lineWidth, lineDisplayIndex, outputX, outputY);
    }

    ad_objectUnpaint(&obj);

    return WEXITSTATUS(pclose(pipe));
}
#else

#error _POSIX_C_SOURCE
int32_t ad_runCommandBox(const char *title, const char *command) {
    AD_UNUSED_PARAMETER(title);
    AD_UNUSED_PARAMETER(command);
    return -1;
}
#endif

static size_t ad_multiSelectorOptionsGetLongestLength(ad_MultiSelector *menu) {
    size_t i = 0;
    size_t length = 0;
    for (i = 0; i < menu->itemCount; i++) {
        ad_MultiSelectorItem *op = &menu->itemOptions[i];
        length = AD_MAX(length, ad_textElementArrayGetLongestLength(op->optionCount, op->options));
    }
    return length;
}

ad_MultiSelectorItem *ad_multiSelectorOptionArrayResize(ad_MultiSelectorItem *ptr, size_t newCount) {
    ptr = realloc(ptr, newCount * sizeof(ad_MultiSelectorItem));
    if (ptr) memset(&ptr[newCount-1], 0, sizeof(ad_MultiSelectorItem));
    return ptr;
}

static void ad_displayMultiSelectorOptions(ad_MultiSelector *menu) {
    size_t i;
    size_t y = menu->itemY;
    size_t x = menu->optionX;
    size_t maximumWidth = menu->optionWidth;
    for (i = 0; i < menu->itemCount; i++) {
        size_t selectedIndex = menu->itemOptions[i].selected;
        ad_displayStringCropped(menu->itemOptions[i].options[selectedIndex].text, x, y, maximumWidth, ad_s_con.objectBg, ad_s_con.objectFg);
        y++;
    }
    hal_flush();
}

static const char *ad_multiSelectorOptionText(ad_MultiSelector *menu, size_t itemIndex) {
    ad_MultiSelectorItem *item = &menu->itemOptions[itemIndex];
    assert(item);
    return item->options[item->selected].text;
}

static void ad_multiSelectorSelectOptionAndDraw(ad_MultiSelector *menu, size_t newSelection) {
    assert(menu);
    ad_displayStringCropped(ad_multiSelectorOptionText(menu, menu->currentSelection),   menu->optionX, menu->optionY + menu->currentSelection, menu->optionWidth, ad_s_con.objectBg, ad_s_con.objectFg);
    ad_displayStringCropped(ad_multiSelectorOptionText(menu, newSelection),             menu->optionX, menu->optionY + newSelection,           menu->optionWidth, ad_s_con.objectFg, ad_s_con.objectBg);
    menu->currentSelection = newSelection;
    hal_flush();
}


static bool ad_multiSelectorPaint(ad_MultiSelector *menu) {
    size_t maximumContentWidth = ad_objectGetMaximumContentWidth();
    size_t maximumPromptWidth = 0;
    size_t maximumItemWidth;
    size_t windowContentWidth;
    size_t promptHeight = (menu->prompt != NULL) ? menu->prompt->lineCount : 0;
    
    AD_RETURN_ON_NULL(menu, false);

    /* Get the length of the longest menu item */
    maximumItemWidth = ad_textElementArrayGetLongestLength(menu->itemCount, menu->items);
      /* Get the length of the longest option item */
    menu->optionWidth = ad_multiSelectorOptionsGetLongestLength(menu);
    maximumItemWidth += 1 + menu->optionWidth;

    windowContentWidth = maximumItemWidth + 2 * AD_MENU_ITEM_PADDING_H;

    /* Factor in the prompt length into window width calculation */
    if (menu->prompt) {
        maximumPromptWidth = ad_textElementArrayGetLongestLength(menu->prompt->lineCount, menu->prompt->lines);
        windowContentWidth = AD_MAX(windowContentWidth, maximumPromptWidth);
    }

    /* Cap it at the maximum width of displayable content in an Object */
    windowContentWidth = AD_MIN(windowContentWidth, maximumContentWidth);

    /* The width of the *labels* is the content width minus padding minus the maximum options width minus the space inbetween */
    menu->itemWidth = windowContentWidth - menu->optionWidth - 1 - 2 * AD_MENU_ITEM_PADDING_H;

    ad_objectInitialize(&menu->object, windowContentWidth, menu->itemCount + 1 + promptHeight); /* +2 because of prompt*/
    ad_objectPaint(&menu->object);

    menu->optionX = ad_objectGetContentX(&menu->object) + AD_MENU_ITEM_PADDING_H;
    menu->optionY = ad_objectGetContentY(&menu->object);
    menu->itemX = menu->optionX + menu->optionWidth + 1; // +1 due to space inbetween option and item
    menu->itemY = menu->optionY;

    /* Print prompt if it exists */
    if (menu->prompt) {   
        uint16_t promptX = ad_objectGetContentX(&menu->object);
        ad_displayTextElementArray(promptX, menu->itemY, ad_objectGetContentWidth(&menu->object), menu->prompt->lineCount, menu->prompt->lines);
        menu->itemY   += 1 + menu->prompt->lineCount;
        menu->optionY += 1 + menu->prompt->lineCount;
    }

    /* Print the menu items */

    ad_displayTextElementArray(menu->itemX, menu->itemY, menu->itemWidth, menu->itemCount, menu->items);
    ad_displayMultiSelectorOptions(menu);

    ad_multiSelectorSelectOptionAndDraw(menu, 0);

    return true;
}

ad_MultiSelector *ad_multiSelectorCreate(const char *title, const char *prompt, bool cancelable) {
    ad_MultiSelector *menu = calloc(1, sizeof(ad_MultiSelector));
    assert(menu);

    menu->cancelable = cancelable;
    menu->prompt = ad_multiLineTextCreate(prompt);
    
    ad_textElementAssign(&menu->object.footer, menu->cancelable ? AD_FOOTER_MULTISELECTOR_CANCELABLE : AD_FOOTER_MULTISELECTOR);
    ad_textElementAssign(&menu->object.title, title);

    return menu;
}


int32_t ad_multiSelectorExecute(ad_MultiSelector *menu) {
    uint32_t ch;

    ad_multiSelectorPaint(menu);

    while (true) {
        ad_MultiSelectorItem *curItem = &menu->itemOptions[menu->currentSelection];
        ch = hal_getKey();

        if          (ch == AD_KEY_UP) {
            ad_multiSelectorSelectOptionAndDraw(menu, (menu->currentSelection > 0) ? menu->currentSelection - 1 : menu->itemCount - 1);
        } else if   (ch == AD_KEY_DOWN) {
            ad_multiSelectorSelectOptionAndDraw(menu, (menu->currentSelection + 1) % menu->itemCount);

            /* MultiSelector handles Right/left in addition to the menu */
        } else if   (ch == AD_KEY_RIGHT) {
            curItem->selected = (curItem->selected == (curItem->optionCount - 1)) ? 0 : curItem->selected + 1;
            ad_multiSelectorSelectOptionAndDraw(menu, menu->currentSelection);
        } else if   (ch == AD_KEY_LEFT) {
            curItem->selected = (curItem->selected == 0) ? curItem->optionCount - 1 : curItem->selected - 1;
            ad_multiSelectorSelectOptionAndDraw(menu, menu->currentSelection);
        } else if   (ch == AD_KEY_ENTER) {
            return 0;
        } else if   (menu->cancelable && (ch == AD_KEY_ESC)) {
            return AD_CANCELED;
        } 
#if DEBUG
        else {
            printf("unhandled key: %08x\n", ch);
        }
#endif
    }
}

void ad_multiSelectorAddItem(ad_MultiSelector *obj, const char *label, size_t optionCount, size_t defaultOption, const char *options[]) {
    ad_MultiSelectorItem *newItem;
    size_t optionIndex;

    if (obj == NULL || label == NULL ) return;

    /* Compared to menu, we have to assign *both* the item label and *all* the options for it */

    obj->itemCount++;
    obj->items = ad_textElementArrayResize(obj->items, obj->itemCount);
    obj->itemOptions = ad_multiSelectorOptionArrayResize(obj->itemOptions, obj->itemCount);
    
    assert(obj->items);
    assert(obj->itemOptions);

    ad_textElementAssign(&obj->items[obj->itemCount-1], label);
    
    newItem = &obj->itemOptions[obj->itemCount-1];
    newItem->optionCount = optionCount;
    newItem->selected = defaultOption;
    newItem->options = ad_textElementArrayResize(newItem->options, optionCount);

    assert(newItem->options);

    for (optionIndex = 0; optionIndex < optionCount; optionIndex++) {
        ad_textElementAssign(&newItem->options[optionIndex], options[optionIndex]);
    }
}

void ad_multiSelectorDestroy(ad_MultiSelector *menu) {
    if (menu) {
        size_t itemIndex;
        ad_objectUnpaint(&menu->object);
        ad_multiLineTextDestroy(menu->prompt);
        if (menu->items) {
            /* Compared to the regular menus, here we must also free the option textelements */
            for (itemIndex = 0; itemIndex < menu->itemCount; itemIndex++) {
                free(menu->itemOptions[itemIndex].options);
            }
            free(menu->items);
        }
        free(menu);
    }
}

size_t ad_multiSelectorGet(ad_MultiSelector *menu, size_t index) {
    AD_RETURN_ON_NULL(menu, 0);

    if (index > menu->itemCount) {
        return 0;
    }

    return menu->itemOptions[index].selected;
}

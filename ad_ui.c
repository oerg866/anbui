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

#define TMP_PROMPT_SIZE (512)

char *ad_teaGetTextPtr(const ad_StringArray *arr, size_t index){
    AD_RETURN_ON_NULL(arr, NULL);
    AD_RETURN_ON_NULL(arr->items[index], NULL);
    return arr->items[index];
}

static void ad_textFileBoxDestroy(ad_TextFileBox *tfb);

static void ad_menuSelectItemAndDraw(ad_Menu *menu, size_t newSelection) {
    assert(menu);
    ad_displayStringCropped(ad_teaGetTextPtr(&menu->items, menu->currentSelection), menu->itemX, menu->itemY + menu->currentSelection, menu->itemWidth, ad_s_con.objectBg, ad_s_con.objectFg);
    ad_displayStringCropped(ad_teaGetTextPtr(&menu->items, newSelection),           menu->itemX, menu->itemY + newSelection,           menu->itemWidth, ad_s_con.objectFg, ad_s_con.objectBg);
    menu->currentSelection = newSelection;
    hal_flush();
}

static bool ad_menuPaint(ad_Menu *menu) {
    size_t maximumContentWidth = ad_objectGetMaximumContentWidth();
    size_t maximumPromptWidth = 0;
    size_t maximumItemWidth;
    size_t windowContentWidth;
    size_t promptHeight = menu->prompt.count;
    
    AD_RETURN_ON_NULL(menu, false);

    /* Get the length of the longest menu item */
    maximumItemWidth = ad_stringArrayGetLongestLength(&menu->items);
    windowContentWidth = maximumItemWidth + 2 * AD_MENU_ITEM_PADDING_H;
    
    /* Factor in the prompt length into window width calculation */
    if (promptHeight) {
        maximumPromptWidth = ad_stringArrayGetLongestLength(&menu->prompt);
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
    if (promptHeight) {   
        ad_displayStringArray(&menu->prompt, menu->itemX, menu->itemY, promptHeight, ad_objectGetContentWidth(&menu->object));
        menu->itemY += 1 + promptHeight;
    }

    /* Print the menu items */

    menu->itemX += AD_MENU_ITEM_PADDING_H;

    ad_displayStringArray(&menu->items, menu->itemX, menu->itemY, menu->items.count, menu->itemWidth);

    ad_menuSelectItemAndDraw(menu, menu->currentSelection);

    return true;
}

ad_Menu *ad_menuCreate(const char *title, bool cancelable, bool enableFKeys, const char *promptFormat, ...) {
    ad_Menu *menu = calloc(1, sizeof(ad_Menu));
    AD_FORMAT_BUFFER(tmpPrompt, TMP_PROMPT_SIZE, promptFormat);

    AD_RETURN_ON_NULL(menu, NULL);
    AD_RETURN_ON_NULL(title, NULL);
    AD_RETURN_ON_NULL(promptFormat, NULL);

    menu->cancelable = cancelable;
    menu->enableFKeys = enableFKeys;

    if (!ad_stringArrayCreateFromString(&menu->prompt, tmpPrompt)) {
        ad_menuDestroy(menu);
        return NULL;
    }
    
    ad_fixedTextElementAssign(menu->object.footer, menu->cancelable ? AD_FOOTER_MENU_CANCELABLE : AD_FOOTER_MENU);
    ad_fixedTextElementAssign(menu->object.title, title);

    return menu;
}

int ad_menuAddItemFormatted(ad_Menu *obj, const char *format, ...) {
    char tmp[AD_TEXT_ELEMENT_SIZE];
    va_list args;
    
    AD_RETURN_ON_NULL(obj, AD_ERROR);


    va_start(args, format);
    vsnprintf(tmp, AD_TEXT_ELEMENT_SIZE-1, format, args);
    tmp[AD_TEXT_ELEMENT_SIZE-1] = 0x00;
    va_end(args);

    if (!ad_stringArrayAddString(&obj->items, tmp, strlen(tmp))) {
        return AD_ERROR;
    }

    obj->itemCount++;
    return obj->itemCount - 1;
}

size_t ad_menuGetSelectedItem(ad_Menu *obj) {
    return obj->currentSelection;
}

bool ad_menuGetItemText(ad_Menu *obj, size_t index, char *dst, size_t dstSize) {
    AD_RETURN_ON_NULL(obj, false);
    AD_RETURN_ON_NULL(dst, false);
    if (index >= obj->itemCount) return false;
    strncpy(dst, ad_teaGetTextPtr(&obj->items, index), dstSize - 1);
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
            ad_objectUnpaint(&menu->object);
            return menu->currentSelection;
        } else if   (menu->enableFKeys && AD_IS_F_KEY(ch)) {
            uint32_t fKeyIndex = ch - AD_KEY_F1;
            ad_objectUnpaint(&menu->object);
            return AD_F_KEY((int32_t) (fKeyIndex));
        } else if   (menu->cancelable && (ch == AD_KEY_ESC)) {
            ad_objectUnpaint(&menu->object);
            return AD_CANCELED;
        }
#if DEBUG
        else {
            printf("unhandled key: %08x\n", ch);
        }
#endif
    }

    ad_objectUnpaint(&menu->object);
}

void ad_menuDestroy(ad_Menu *menu) {
    if (menu) {
        ad_objectUnpaint(&menu->object);
        ad_stringArrayDestroy(&menu->prompt);
        ad_stringArrayDestroy(&menu->items);
        free(menu);
    }
}

static int32_t ad_menuExecuteDirectlyInternal(const char *title, bool cancelable, size_t optionCount, const char *options[], const char *prompt) {
    ad_Menu        *menu = NULL;
    int32_t         ret  = 0;
    size_t          idx;

    AD_RETURN_ON_NULL(options, AD_ERROR);
    AD_RETURN_ON_NULL(prompt, AD_ERROR);

    menu = ad_menuCreate(title, cancelable, false, "%s", prompt); /* prompt is already formatted */
    AD_RETURN_ON_NULL(menu, AD_ERROR);

    for (idx = 0; idx < optionCount; idx++) {
        ad_menuAddItemFormatted(menu, "%s", options[idx]);
    }

    ret = ad_menuExecute(menu);
    ad_menuDestroy(menu);

    return ret;
}

int32_t ad_menuExecuteDirectly(const char *title, bool cancelable, size_t optionCount, const char *options[], const char *promptFormat, ...) {
    AD_FORMAT_BUFFER(tmpPrompt, TMP_PROMPT_SIZE, promptFormat);
    return ad_menuExecuteDirectlyInternal(title, cancelable, optionCount, options, tmpPrompt);
}

int32_t ad_yesNoBox(const char *title, bool cancelable, const char *promptFormat, ...) {
    const char *options[] = {"Yes", "No"};
    AD_FORMAT_BUFFER(tmpPrompt, TMP_PROMPT_SIZE, promptFormat);
    return ad_menuExecuteDirectlyInternal(title, cancelable, AD_ARRAY_SIZE(options), options, tmpPrompt);
}

int32_t ad_okBox(const char *title, bool cancelable, const char *promptFormat, ...) {
    const char *options[] = {"OK"};
    AD_FORMAT_BUFFER(tmpPrompt, TMP_PROMPT_SIZE, promptFormat);
    return ad_menuExecuteDirectlyInternal(title, cancelable, AD_ARRAY_SIZE(options), options, tmpPrompt);
}

static size_t ad_progressBoxGetLongestLabelLength(ad_ProgressBox *pb) {
    size_t i = 0;
    size_t length = 0;
    for (i = 0; i < pb->itemCount; i++) {
        length = AD_MAX(length, strlen(pb->items[i].label));
    }
    return length;
}

bool ad_progressBoxPaint (ad_ProgressBox *pb) {
    size_t expectedWidth;
    size_t promptWidth;
    size_t promptHeight;
    size_t pbIndex;
    size_t pbDefaultWidth = ad_objectGetMaximumContentWidth() * 60 / 100;
    
    AD_RETURN_ON_NULL(pb, false);

    /* Get the length of the longest Prompt line */
    promptHeight = pb->prompt.count;
    promptWidth = promptHeight ? ad_stringArrayGetLongestLength(&pb->prompt) : 0;

    pb->labelWidth = ad_progressBoxGetLongestLabelLength(pb);

    /* Standard width = label width + 60% of screen for the bar
       Maximum width = text length + margin, capped to maximum object width */
    expectedWidth = promptWidth + pbDefaultWidth;

    ad_objectInitialize(&pb->object, expectedWidth, promptHeight + pb->itemCount + 1);
    ad_objectPaint(&pb->object);

    pb->labelX = ad_objectGetContentX(&pb->object);
    pb->boxY = ad_objectGetContentY(&pb->object);
    pb->boxWidth = ad_objectGetContentWidth(&pb->object);

    if (pb->itemCount == 1) {
        pb->boxX = pb->labelX;
    } else {
        /* If this is a multi-item box, the box starts *after* the label. */
        pb->boxX = pb->labelX + pb->labelWidth + 1 + 1;
        pb->boxWidth -= (pb->labelWidth + 1);
    }

    if (promptHeight) {   
        ad_displayStringArray(&pb->prompt, pb->labelX, pb->boxY, promptHeight, ad_objectGetContentWidth(&pb->object));
        pb->boxY += 1 + promptHeight;
    }

    /* Draw the actual bar(s) */
    for (pbIndex = 0; pbIndex < pb->itemCount; pbIndex++) {
        uint16_t y = pb->boxY + pbIndex;

        /* Need to set it again to trigger X calculation */
        ad_progressBoxMultiUpdate(pb, pbIndex, pb->items[pbIndex].rawValue);
        ad_fill(pb->boxWidth,                   ad_s_con.progressChar, pb->boxX, pb->boxY + pbIndex, ad_s_con.progressBlankBg, ad_s_con.progressBlankFg);
        ad_fill(pb->items[pbIndex].currentX,    ad_s_con.progressChar, pb->boxX, pb->boxY + pbIndex, ad_s_con.progressFillBg,  ad_s_con.progressFillFg);

        /* Draw the bar labels ONLY if it's a multi-item box */
        if (pb->itemCount > 1) {
            ad_displayStringCropped(pb->items[pbIndex].label, pb->labelX, y, pb->labelWidth, ad_s_con.objectBg, ad_s_con.objectFg);
        }
    }

    hal_flush();

    return true;
}

ad_ProgressBox *ad_progressBoxSingleCreate(const char *title, uint32_t maxProgress, const char *promptFormat, ...) {
    ad_ProgressBox *pb = NULL;
    AD_FORMAT_BUFFER(tmpPrompt, TMP_PROMPT_SIZE, promptFormat);

    pb = ad_progressBoxMultiCreate(title, "%s", tmpPrompt);

    if (pb) {
        ad_progressBoxAddItem(pb, "", maxProgress);
        ad_progressBoxPaint(pb);
    }
    
    return pb;
}

ad_ProgressBox *ad_progressBoxMultiCreate(const char *title, const char *promptFormat, ...) {
    ad_ProgressBox *pb = NULL;
    AD_FORMAT_BUFFER(tmpPrompt, TMP_PROMPT_SIZE, promptFormat);

    AD_RETURN_ON_NULL(title, NULL);
    AD_RETURN_ON_NULL(promptFormat, NULL);
    pb = calloc(1, sizeof(ad_ProgressBox));
    AD_RETURN_ON_NULL(pb, NULL);

    if (!ad_stringArrayCreateFromString(&pb->prompt, tmpPrompt)) {
        ad_progressBoxDestroy(pb);
        return NULL;
    }

    ad_fixedTextElementAssign(pb->object.title, title);
    return pb;
}

void ad_progressBoxDestroy(ad_ProgressBox *pb) {
    if (pb) {
        ad_objectUnpaint(&pb->object);
        ad_stringArrayDestroy(&pb->prompt);
        if (pb->items) {
            free(pb->items);
        }
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

    /*  Cap the progress bar */
    progress = AD_MIN(prog->outOf, progress);

    /*  Save raw value, it might be useful somewhere later */
    prog->rawValue = progress;

    /*  round / lround for values > 1 in MUSL gets clipped to 1.0 ?????? am I stupid?
        Anyway this hack is here until I get some sleep.. */
    newX = AD_ROUND_HACK_WTF(uint16_t, ((double) pb->boxWidth * (double) progress) / ((double) prog->outOf));

    if (newX == prog->currentX) {
        return;
    }


    if (newX > prog->currentX) {
        newPaintLength = newX - prog->currentX;

        ad_setCursorPosition(pb->boxX + prog->currentX, pb->boxY + index);
        ad_setColor(ad_s_con.progressFillBg, ad_s_con.progressFillFg);
        ad_putChar(ad_s_con.progressChar, newPaintLength);
    } else {
        newPaintLength = prog->currentX - newX;
        ad_setCursorPosition(pb->boxX + newX, pb->boxY + index);
        ad_setColor(ad_s_con.progressBlankBg, ad_s_con.progressBlankFg);
        ad_putChar(ad_s_con.progressChar, newPaintLength);
    }

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

size_t ad_progressBoxAddItem(ad_ProgressBox *obj, const char *label, uint32_t maxProgress) {
    if (obj == NULL || label == NULL ) return SIZE_MAX;

    obj->itemCount++;
    obj->items = ad_progressArrayResize(obj->items, obj->itemCount);

    assert(obj->items);

    obj->items[obj->itemCount-1].outOf = maxProgress;
    ad_fixedTextElementAssign(obj->items[obj->itemCount-1].label, label);

    return obj->itemCount - 1;
}

void ad_progressBoxSetMaxProgress(ad_ProgressBox *obj, size_t index, uint32_t maxProgress) {
    if (obj == NULL || index >= obj->itemCount) return;
    obj->items[index].outOf = maxProgress;
}

ad_Mixer *ad_mixerCreate(const char *title, const char *prompt, ...) {
    AD_FORMAT_BUFFER(tmpPrompt, TMP_PROMPT_SIZE, prompt);
    return (ad_Mixer *) ad_progressBoxMultiCreate(title, "%s", tmpPrompt);
}

void ad_mixerDestroy(ad_Mixer *mix) {
    ad_progressBoxDestroy((ad_ProgressBox *) mix);
}

size_t ad_mixerAddItem(ad_Mixer *mix, const char *label, uint32_t startValue, uint32_t maxValue) {
    size_t item = ad_progressBoxAddItem((ad_ProgressBox *) mix, label, maxValue);

    if (item != SIZE_MAX) {
        ad_mixerSetValue(mix, item, startValue);
    }

    return item;
}

static void ad_mixerSelectItemAndDraw(ad_Mixer *mix, size_t newSelection) {
    size_t curSelection = mix->currentSelection;
    uint16_t curY = mix->boxY + curSelection;
    uint16_t newY = mix->boxY + newSelection;
    
    ad_displayStringCropped(mix->items[curSelection].label, mix->labelX, curY, mix->labelWidth, ad_s_con.objectBg, ad_s_con.objectFg);
    ad_displayStringCropped(mix->items[newSelection].label, mix->labelX, newY, mix->labelWidth, ad_s_con.objectFg, ad_s_con.objectBg);
    mix->currentSelection = newSelection;
    hal_flush();
}

bool ad_mixerPaint(ad_Mixer *mix) {
    /* first draw the box as we would a progress box */
    bool ret = ad_progressBoxPaint((ad_ProgressBox *) mix);

    AD_RETURN_ON_NULL(mix, false);

    /* then we simply select the item */
    if (mix->itemCount == 0) return false;

    /* Highlight the current selection */
    ad_mixerSelectItemAndDraw(mix, mix->currentSelection);

    return ret;
}

void ad_mixerSetValue(ad_Mixer *mix, size_t index, uint32_t value) {
    ad_progressBoxMultiUpdate((ad_ProgressBox *) mix, index, value);
}

void ad_mixerSetCharAndColor(char fillChar, uint8_t colorBlankBg, uint8_t colorBlankFg, uint8_t colorFillBg, uint8_t colorFillFg) {
    ad_progressBoxSetCharAndColor(fillChar, colorBlankBg, colorBlankFg, colorFillBg, colorFillFg);
}

int32_t ad_mixerExecute(ad_Mixer *mix, bool enableFKeys, ad_MixerCallback cb, void *cbUserData) {
    ad_mixerPaint(mix);

    while (true) {
        ad_Progress *curItem = &mix->items[mix->currentSelection];
        uint32_t ch = hal_getKey();

        if          (ch == AD_KEY_UP) {
            size_t newSelection = CAPPED_SUB(mix->currentSelection, 1, 0); 
            ad_mixerSelectItemAndDraw(mix, newSelection);
        } else if   (ch == AD_KEY_DOWN) {
            size_t newSelection = CAPPED_ADD(mix->currentSelection, 1, mix->itemCount - 1);
            ad_mixerSelectItemAndDraw(mix, newSelection);
        } else if   (ch == AD_KEY_LEFT) {
            uint32_t newValue = CAPPED_SUB(curItem->rawValue, 1UL, 0UL);
            ad_mixerSetValue(mix, mix->currentSelection, newValue);
            if (cb != NULL) {
                cb(cbUserData, mix->currentSelection, curItem->rawValue);
            }
        } else if   (ch == AD_KEY_RIGHT) {
            uint32_t newValue = CAPPED_ADD(curItem->rawValue, 1UL, curItem->outOf);
            ad_mixerSetValue(mix, mix->currentSelection, newValue);
            if (cb != NULL) {
                cb(cbUserData, mix->currentSelection, curItem->rawValue);
            }
        } else if   (enableFKeys && AD_IS_F_KEY(ch)) {
            uint32_t fKeyIndex = ch - AD_KEY_F1;
            ad_objectUnpaint(&mix->object);
            return AD_F_KEY((int32_t) (fKeyIndex));
        } else if   (ch == AD_KEY_ENTER) {
            ad_objectUnpaint(&mix->object);
            return (int32_t) mix->currentSelection;
        } else if   (ch == AD_KEY_ESC) {
            ad_objectUnpaint(&mix->object);
            return AD_CANCELED;
        } 
#if DEBUG
        else {
            printf("unhandled key: %08x\n", ch);
        }
#endif
    }
}

uint32_t ad_mixerGetValue (ad_Mixer *mix, size_t index) {
    AD_RETURN_ON_NULL(mix, UINT32_MAX);
    if (index >= mix->itemCount) return UINT32_MAX;
    return mix->items[index].rawValue;
}


static inline void ad_textFileBoxRedrawLines(ad_TextFileBox *tfb) {
    ad_displayStringArrayRange(&tfb->lines, (size_t)tfb->currentIndex, (size_t) tfb->linesOnScreen, tfb->textX, tfb->textY, (size_t) tfb->lineWidth);
}

static bool ad_textFileBoxPaint(ad_TextFileBox *tfb) {
    size_t lineWidth = 0;

    AD_RETURN_ON_NULL(tfb, false);

    /* Get the length of the longest Text line */
    lineWidth = ad_stringArrayGetLongestLength(&tfb->lines);

    ad_objectInitialize(&tfb->object, lineWidth, tfb->lines.count);

    tfb->textX = ad_objectGetContentX(&tfb->object);
    tfb->textY = ad_objectGetContentY(&tfb->object);
    tfb->lineWidth = ad_objectGetContentWidth(&tfb->object);
    tfb->linesOnScreen = ad_objectGetContentHeight(&tfb->object);
    tfb->highestIndex = tfb->lines.count - tfb->linesOnScreen;

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

    if (fileBuffer == NULL) {
        goto error;
    }

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
    
    ad_fixedTextElementAssign(tfb->object.title, title);
    ad_fixedTextElementAssign(tfb->object.footer, AD_FOOTER_TEXTFILEBOX);
    
    if (!ad_stringArrayCreateFromString(&tfb->lines, fileBuffer)) goto error;

    ad_textFileBoxPaint(tfb);

    free(fileBuffer);

    return tfb;

error:
    if (inFile != NULL) fclose(inFile);
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
        ad_stringArrayDestroy(&tfb->lines);
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

#if defined(POPEN)

static void ad_commandBoxRedraw(const ad_StringArray *lines, size_t lineCount, uint16_t contentWidth, size_t index, uint16_t x, uint16_t y) {
    size_t curLine;

    for (curLine = 0; curLine < lineCount; curLine++) {
        ad_displayStringCropped(ad_teaGetTextPtr(lines, index % lineCount), x, y + curLine, contentWidth, ad_s_con.objectBg, ad_s_con.objectFg);
        index++;
    }
}

#ifndef WEXITSTATUS
#define WEXITSTATUS(x) ((x) & 0xff)
#endif

int32_t ad_runCommandBox(const char *title, const char *command) {
    ad_Object           obj;
    ad_StringArray      lines = { 0 };
    size_t              visibleLines = ad_objectGetMaximumContentHeight() * 60 / 100;
    size_t              lineWidth = ad_objectGetMaximumContentWidth() * 80 / 100;
    size_t              lineDisplayIndex = 0;
    size_t              lineWriteIndex = 0;
    uint16_t            outputX = 0;
    uint16_t            outputY = 0;
    FILE*               pipe = NULL;
    char                tmp[AD_TEXT_ELEMENT_SIZE] = "";
    char               *newLineChar = NULL;
    size_t              i;

    AD_RETURN_ON_NULL(command, AD_ERROR);
    AD_RETURN_ON_NULL(title, AD_ERROR);

    memset(&obj, 0, sizeof(obj));

    ad_fixedTextElementAssign(obj.title, title);
    ad_fixedTextElementAssignFormatted(obj.footer, "Running: '%s'...", command);
    ad_objectInitialize(&obj, lineWidth, visibleLines);

    lineWidth = ad_objectGetContentWidth(&obj);
    visibleLines = ad_objectGetContentHeight(&obj);
    lineWriteIndex = visibleLines - 1; /* Start writing at bottommost line */
    outputX = ad_objectGetContentX(&obj);
    outputY = ad_objectGetContentY(&obj);

    ad_objectPaint(&obj);

    /* Create the TEA with our height */

    for (i = 0; i < visibleLines; i++) {
        if (!ad_stringArrayAddString(&lines, tmp, AD_TEXT_ELEMENT_SIZE)) {
            ad_objectUnpaint(&obj);
            ad_stringArrayDestroy(&lines);
            return AD_ERROR;
        }
    }
    
    /* Run the actual command */
    pipe = POPEN(command, "r");

    while (fgets(ad_stringArrayGet(&lines, lineWriteIndex % visibleLines), AD_TEXT_ELEMENT_SIZE, pipe) != NULL) {
        /* Fix newline */
        newLineChar = strchr(ad_stringArrayGet(&lines, lineWriteIndex % visibleLines), '\n');
        if (newLineChar) *newLineChar = 0x00;
        lineWriteIndex++;
        lineDisplayIndex++;
        ad_commandBoxRedraw(&lines, visibleLines, lineWidth, lineDisplayIndex, outputX, outputY);
    }

    ad_objectUnpaint(&obj);
    ad_stringArrayDestroy(&lines);
    return WEXITSTATUS(PCLOSE(pipe));
}
#else

int32_t ad_runCommandBox(const char *title, const char *command) {
    AD_UNUSED_PARAMETER(title);
    AD_UNUSED_PARAMETER(command);
    assert(false && "runCommandBox not available on this platform");
    return AD_ERROR;
}
#endif

static size_t ad_multiSelectorOptionsGetLongestLength(ad_MultiSelector *menu) {
    size_t i = 0;
    size_t length = 0;
    for (i = 0; i < menu->itemCount; i++) {
        ad_MultiSelectorItem *op = &menu->itemOptions[i];
        length = AD_MAX(length, ad_stringArrayGetLongestLength(&op->options));
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
        ad_displayStringCropped(ad_teaGetTextPtr(&menu->itemOptions[i].options, selectedIndex), x, y, maximumWidth, ad_s_con.objectBg, ad_s_con.objectFg);
        y++;
    }
    hal_flush();
}

static const char *ad_multiSelectorOptionText(ad_MultiSelector *menu, size_t itemIndex) {
    ad_MultiSelectorItem *item = &menu->itemOptions[itemIndex];
    assert(item);
    return ad_teaGetTextPtr(&item->options, item->selected);
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
    size_t promptHeight = menu->prompt.count;
    
    AD_RETURN_ON_NULL(menu, false);

    /* Get the length of the longest menu item */
    maximumItemWidth = ad_stringArrayGetLongestLength(&menu->items);
      /* Get the length of the longest option item */
    menu->optionWidth = ad_multiSelectorOptionsGetLongestLength(menu);
    maximumItemWidth += 1 + menu->optionWidth;

    windowContentWidth = maximumItemWidth + 2 * AD_MENU_ITEM_PADDING_H;

    /* Factor in the prompt length into window width calculation */
    if (promptHeight) {
        maximumPromptWidth = ad_stringArrayGetLongestLength(&menu->prompt);
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
    if (promptHeight) {   
        uint16_t promptX = ad_objectGetContentX(&menu->object);
        ad_displayStringArray(&menu->prompt, promptX, menu->itemY, promptHeight, ad_objectGetContentWidth(&menu->object));
        menu->itemY   += 1 + promptHeight;
        menu->optionY += 1 + promptHeight;
    }

    /* Print the menu items */

    ad_displayStringArray(&menu->items, menu->itemX, menu->itemY, menu->items.count, menu->itemWidth);
    ad_displayMultiSelectorOptions(menu);

    ad_multiSelectorSelectOptionAndDraw(menu, 0);

    return true;
}

ad_MultiSelector *ad_multiSelectorCreate(const char *title, const char *prompt, bool cancelable) {
    ad_MultiSelector *menu = calloc(1, sizeof(ad_MultiSelector));
    assert(menu);

    menu->cancelable = cancelable;

    if (!ad_stringArrayCreateFromString(&menu->prompt, prompt)) {
        ad_multiSelectorDestroy(menu);
        return NULL;
    }
    
    ad_fixedTextElementAssign(menu->object.footer, menu->cancelable ? AD_FOOTER_MULTISELECTOR_CANCELABLE : AD_FOOTER_MULTISELECTOR);
    ad_fixedTextElementAssign(menu->object.title, title);

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
            ad_objectUnpaint(&menu->object);
            return 0;
        } else if   (menu->cancelable && (ch == AD_KEY_ESC)) {
            ad_objectUnpaint(&menu->object);
            return AD_CANCELED;
        } 
#if DEBUG
        else {
            printf("unhandled key: %08x\n", ch);
        }
#endif
    }
}

size_t ad_multiSelectorAddItem(ad_MultiSelector *obj, const char *label, size_t optionCount, size_t defaultOption, const char **options) {
    ad_MultiSelectorItem newItem;
    ad_MultiSelectorItem *newItemOptionsAlloc;
    size_t optionIndex;

    if (obj == NULL || label == NULL ) return SIZE_MAX;

    /* Compared to menu, we have to assign *both* the item label and *all* the options for it */

    /* Attempt to do a new item first */
    memset(&newItem, 0, sizeof(newItem));
    newItem.optionCount = optionCount;
    newItem.selected = defaultOption;

    for (optionIndex = 0; optionIndex < optionCount; optionIndex++) {
        if (!ad_stringArrayAddString(&newItem.options, options[optionIndex], strlen(options[optionIndex]))) {
            /* Alloc failed, try to salvage what we have */
            goto error;
        }
    }

    /* Try to alloc and add a string to the multiselector items string array */
    if (!ad_stringArrayAddString(&obj->items, label, strlen(label))) {
        goto error;
    }

    /* Now let's try adding the itemOptions */
    newItemOptionsAlloc = ad_multiSelectorOptionArrayResize(obj->itemOptions, obj->itemCount + 1);

    if (newItemOptionsAlloc == NULL) {
        goto error;
    }

    obj->itemCount++;
    obj->itemOptions = newItemOptionsAlloc;
    obj->itemOptions[obj->itemCount - 1] = newItem;
    return obj->itemCount-1;
error:
    ad_stringArrayDestroy(&newItem.options);
    return SIZE_MAX;
}

void ad_multiSelectorDestroy(ad_MultiSelector *menu) {
    if (menu) {
        ad_objectUnpaint(&menu->object);
        ad_stringArrayDestroy(&menu->prompt);

        if (menu->itemOptions) {
            size_t item;
            for (item = 0; item < menu->itemCount; item++) {
                ad_stringArrayDestroy(&menu->itemOptions[item].options);
            }
            
            free(menu->itemOptions);
        }

        ad_stringArrayDestroy(&menu->items);
        free(menu);
    }
}

size_t ad_multiSelectorGet(ad_MultiSelector *menu, size_t index) {
    AD_RETURN_ON_NULL(menu, 0);

    if (index >= menu->itemCount) {
        return 0;
    }

    return menu->itemOptions[index].selected;
}

/*
    AnbUI Miniature Text UI Lib for Burger Enjoyers(tm)
    
    Tip of the day: Did you know that when you burger cheese on burger,
    taste cheeseburger cheese on you?
    This, *this* is because burger cheese burger taste cheese on burger(*).

    (*)Cheese as reference to taste burger on your cheese.

    (C) 2024 E. Voirin (oerg866) */

#ifndef _ANBUI_H_
#define _ANBUI_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define AD_YESNO_YES    (0)
#define AD_YESNO_NO     (1)
#define AD_CANCELED     (-1)
/* Return code for F-Keys */
#define AD_F_KEY(x)     (-(10+(x)))
#define AD_ERROR        (-INT32_MAX)

#define COLOR_BLACK 0
#define COLOR_BLUE  1
#define COLOR_GREEN 2
#define COLOR_CYAN  3
#define COLOR_RED   4
#define COLOR_MAGNT 5
#define COLOR_BROWN 6
#define COLOR_DGRAY 7
#define COLOR_GRAY  8
#define COLOR_LBLUE 9
#define COLOR_LGREN 10
#define COLOR_LCYAN 11
#define COLOR_LRED  12
#define COLOR_LMGNT 13
#define COLOR_YELLO 14
#define COLOR_WHITE 15

typedef struct ad_TextFileBox   ad_TextFileBox;
typedef struct ad_ProgressBox   ad_ProgressBox;
typedef struct ad_Menu          ad_Menu;
typedef struct ad_MultiSelector ad_MultiSelector;
typedef struct ad_ConsoleConfig ad_ConsoleConfig;

/*  Initializes AnbUI.
    This call is REQUIRED before using *ANY* other functions declared here. */
void            ad_init                 (const char *title);
/*  Restores AnbUI's text frontend.
    This is helpful if the user intends to run other commands in the same text display which outputs text on the screen. */
void            ad_restore              (void);
/*  Deinitializes AnbUI and restores the system's original text console state. */
void            ad_deinit               (void);
/*  Sets the footer text on the screen */
void            ad_setFooterText        (const char *footer);
/*  Clears the footer on the screen*/
void            ad_clearFooter          (void);

/*  Create a menu with given title and prompt.
    Cancelable means the menu can be cancelled using the ESC key.
    enableFKeys means that menuExecute will return if F1-F12 are pressed with that value.
    Must be deallocated with ad_menuDestroy */
ad_Menu        *ad_menuCreate           (const char * title, const char *prompt, bool cancelable, bool enableFKeys);
/*  Adds an item to a menu. Returns AD_ERROR on error or the index of the newly added item on success. */
int             ad_menuAddItemFormatted (ad_Menu *menu, const char *format, ...);
/*  Returns the item label for a menu */
bool            ad_menuGetItemText      (ad_Menu *obj, size_t index, char *dst, size_t dstSize);
/*  Returns the amount of selectable items a menu has */
size_t          ad_menuGetItemCount     (ad_Menu *menu);
/*  Displays the menu and lets the user make a choice.
    It is safe to call this function repeatedly.
    Returns values: 1) the index of the chosen item
                    2) An F-Key that was pressed if the menu was created with enableFKeys = True
                       Check this with AD_F_KEY(x) where x is from 0 to 11 (= F1 to F12)
                    3) AD_CANCELED for a cancelled menu (if menu was created as 'cancelable')
                    4) AD_ERROR if something blew up (null pointer or something) */
int32_t         ad_menuExecute          (ad_Menu *menu);
/*  Deallocates menu. */
void            ad_menuDestroy          (ad_Menu *menu);
/*  Launches a menu directly with the given options array and a formatted prompt. No (de)allocations need to be made.
    Return values are identical to ad_menuExecute.*/
int32_t         ad_menuExecuteDirectly  (const char *title, bool cancelable, size_t optionCount, const char *options[], const char *promptFormat, ...);

/*  Launches a Yes/No selection menu with the given title and a formatted prompt.
    Returns AD_YESNO_YES or AD_YESNO_NO, other return values like ad_menuExecute. */
int32_t         ad_yesNoBox             (const char *title, bool cancelable, const char *promptFormat, ...);
/*  Launches a message/info box with the given title and a formatted prompt.
    Returns 0 after user confirmation, other return values like ad_menuExecute. */
int32_t         ad_okBox                (const char *title, bool cancelable, const char *promptFormat, ...);

/*  Creates a Progress-bar display box with the given title and prompt.
    maxProgress is the maximum progress value, i.e. the progress value that yields a filled bar.
    Must be deallocated with ad_progressBoxDestroy. */
ad_ProgressBox *ad_progressBoxSingleCreate (const char *title, uint32_t maxProgress, const char *prompt, ...);

/*  Creates a Progress-bar display box with the given title and prompt.
    This call should be used if the progress box should have multiple progress bars with labels.
    This call also does not *show* the progress box automatically.
    You must add progress bar items before calling ad_ProgressBoxPaint.
    Must be deallocated with ad_progressBoxDestroy. */
ad_ProgressBox *ad_progressBoxMultiCreate  (const char *title, const char *prompt, ...);
/*  Deallocates the progress box */
void            ad_progressBoxDestroy   (ad_ProgressBox *pb);
/*  Adds a row to the progress box. I.e. a label + a progress bar. */
void            ad_progressBoxAddItem   (ad_ProgressBox *pb, const char *label, uint32_t maxProgress);
/*  This must be called once before calling progressBoxUpdate so that the actual box gets drawn after adding items */
bool            ad_progressBoxPaint     (ad_ProgressBox *pb);
/*  Updates the progress box with the given progress value. The fill level is calculated as progress-out-of-maxProgress. */
void            ad_progressBoxUpdate    (ad_ProgressBox *pb, uint32_t progress);
/*  Updates the n-th progress box with the given progress value. The fill level is calculated as progress-out-of-maxProgress. */
void            ad_progressBoxMultiUpdate(ad_ProgressBox *pb, size_t index, uint32_t progress);
/*  Intended for multi-item progress bars. Sets the maximum progress value for the bar at <index>.
    Example: useful if the total size of a file copy isnt known at the box's creation */
void            ad_progressBoxSetMaxProgress(ad_ProgressBox *obj, size_t index, uint32_t maxProgress);
/*  Set the character and colors used for filling the progress bar (normally this is is a space) */
void            ad_progressBoxSetCharAndColor(char fillChar, uint8_t colorBlankBg, uint8_t colorBlankFg, uint8_t colorFillBg, uint8_t colorFillFg);

/*  Displays a scrollable display box which contains the contents of the text file pointed to by fileName.
    It does NOT support horizontal scrolling, lines that are too long will be cut off and truncated with a "..." suffix.
    The file should not contain unicode characters, as I'm too lazy to handle these correctly.
    The entire file will be loaded into memory, so be mindful with the sizes.
    Returns AD_ERROR if there was a problem (bad file, allocation failure, etc.) */
int32_t         ad_textFileBox          (const char *title, const char *fileName);
/*  Displays a display box that shows the output of the given command line (which includes all parameters)
    NOTE:   This is ONLY available on platforms which support pipes and popen!
            (aka. pretty much everything other than DOS) */
int32_t         ad_runCommandBox        (const char *title, const char *command);

/*  Save the screen state internally so it can be recalled later. Doing this twice will overwrite the first backup.
    This can be used to, for example, display an error message box and restore the previously displayed UI
    after the error was handled. */
void            ad_screenSaveState      (void);
/*  Restores the screen after a previous "screenSaveState" call. */
void            ad_screenLoadState      (void);

/*  Create a multi selector menu with given title and prompt.
    Cancelable means the menu can be cancelled using the ESC key.
    Must be deallocated with ad_multiSelectorDestroy */
ad_MultiSelector *ad_multiSelectorCreate(const char *title, const char *prompt, bool cancelable);
/*  Displays the multi selector menu and lets the user modify options.
    Returns values: 1) 0 if the user pressed enter
                    2) AD_CANCELED for a cancelled menu (if menu was created as 'cancelable')
                    3) AD_ERROR if something blew up (null pointer or something) */
int32_t         ad_multiSelectorExecute (ad_MultiSelector *menu);
/*  Adds an item + options to the multi slelector menu */
void            ad_multiSelectorAddItem (ad_MultiSelector *obj, const char *label, size_t optionCount, size_t defaultOption, const char *options[]);
/*  Deallocates the multi selector menu */
void            ad_multiSelectorDestroy (ad_MultiSelector *menu);
/*  Get the selected option for a multi selector item. */
size_t          ad_multiSelectorGet     (ad_MultiSelector *menu, size_t index);

#endif

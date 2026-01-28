/*
    AnbUI Miniature Text UI Lib for Burger Enjoyers(tm)

    anbui: Init/Deinit code

    Tip of the day: Burgers burger best when burger cheese is cheesed
    by burgering the cheese.
    Remember, the key to a burgered cheese is to cheese the burger while the
    burger burgers.

    (C) 2024 E. Voirin (oerg866) */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#include "anbui.h"
#include "ad_priv.h"
#include "ad_hal.h"

ad_ConsoleConfig ad_s_con;
ad_TextElement ad_s_title;

void ad_init(const char *title) {
    assert(title);

    ad_s_con.width          = 0;
    ad_s_con.height         = 0;
    ad_s_con.headerBg       = COLOR_RED;
    ad_s_con.headerFg       = COLOR_WHITE;
    ad_s_con.titleBg        = COLOR_CYAN;
    ad_s_con.titleFg        = COLOR_BLACK;
    ad_s_con.footerBg       = COLOR_BLACK;
    ad_s_con.footerFg       = COLOR_WHITE;
    ad_s_con.objectBg       = COLOR_WHITE;
    ad_s_con.objectFg       = COLOR_BLACK;
    ad_s_con.progressBlank  = COLOR_GRAY;
    ad_s_con.progressFill   = COLOR_LRED;
    ad_s_con.backgroundFill = COLOR_BLUE;

    ad_initConsole(&ad_s_con);

    ad_textElementAssign(&ad_s_title, title);
    ad_drawBackground(ad_s_title.text);
}

void ad_restore(void) {
    hal_restoreConsole();
    ad_drawBackground(ad_s_title.text);
}

void ad_deinit() {
    ad_deinitConsole();
}

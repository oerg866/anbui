/*
    AnbUI Miniature Text UI Lib for Burger Enjoyers(tm)

    ad_test: AnbUI Example Test Application

    Tip of the day: Cheese burgers and bacon burgers are like ying and yang
    except you get to enjoy the ying and yang with cheese and bacon without
    having to compromise on the other.

    (C) 2024 E. Voirin (oerg866) */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "anbui.h"

void sleep(double seconds) {
    clock_t start_time = clock();
    while (clock() < start_time + (seconds * CLOCKS_PER_SEC));
}

int main(int argc, char *argv[]) {
    ad_Menu        *menu = NULL;
    ad_ProgressBox *prog = NULL;
    size_t          i;
    size_t          j;

    (void) argc;
    (void) argv;

    ad_init("AnbUI Super Burger Edition - The Test Application(tm)");

    /* test multi selector */
    {
        ad_MultiSelector *sel = ad_multiSelectorCreate("Select Burger Ingredients", "Please select your burger ingredients",false);
        const char *buns[] = { "Sesame", "Brioche" };
        const char *bacon[] = { "Yes", "No" };
        const char *cheese[] = { "Cheddar", "American", "None" };
        const char *pickles[] = { "Yes", "No" };
        
        ad_multiSelectorAddItem(sel, "What kind of bun?", 2, buns);
        ad_multiSelectorAddItem(sel, "Do you want bacon?", 2, bacon);
        ad_multiSelectorAddItem(sel, "What kind of cheese?", 3, cheese);
        ad_multiSelectorAddItem(sel, "Do you want pickles?", 2, pickles);

        ad_multiSelectorExecute(sel);

        ad_multiSelectorDestroy(sel);
    }

    ad_yesNoBox("Burger Selection", true,
        "Do you want cheese on burger cheese taste on you?\n"
        "Refer to Anby Demara's Burger Handbook for more\n"
        "information.");

    ad_okBox("Another Burger Selection", true, "Cheese is taste on burger cheese on you.");

    ad_runCommandBox("Updating my burger to have burger cheese on burger", "apt update 2>&1");

    /* Test Text File Box */

    ad_textFileBox("demara.txt", "demara.txt");

    menu = ad_menuCreate("Selector of death",
        "Select your favorite philosophy:\n"
        "Please note that your burgering is dependent\n"
        "on taste of burger cheese on you.",
        true);

    for (i = 0; i < 10; i++) {
        ad_menuAddItemFormatted(menu, "Item %zu: Burger Cheese is Cheese on Burger", i);
    }

    ad_menuAddItemFormatted(menu, "Item 9000: All the cheesing of burger taste on you. LONG SCHLONG 1231445982139582092385092830");

    ad_menuExecute(menu);
    ad_menuDestroy(menu);

    /* Test Progress Box */
    prog = ad_progressBoxSingleCreate("Vorwaerts immer, Rueckwaerts nimmer", 10,
        "Please wait while we burger your cheese.\n"
        "Also: Burgering can not be tasted.");

    for (i = 0; i <= 10; i++) {
        ad_progressBoxUpdate(prog, i);

        if (i == 6) {
            ad_screenSaveState();
            ad_restore();
            ad_yesNoBox("Oh no!", false, 
                "An error has occured!\n"
                "Eat burgers?");
            ad_screenLoadState();
        }

        sleep(0.1);
    }

    ad_progressBoxDestroy(prog);

    /* Test Multi Progress Box */

    prog = ad_progressBoxMultiCreate("Preparing your order...",
        "Please wait while we make your burger.");

    ad_progressBoxAddItem(prog, "Cutting veggies", 10);
    ad_progressBoxAddItem(prog, "Preparing Patty", 10);
    ad_progressBoxAddItem(prog, "Cooking patty", 10);
    ad_progressBoxAddItem(prog, "Toasting bun", 10);
    ad_progressBoxAddItem(prog, "Finishing", 10);

    ad_progressBoxPaint(prog);

    for (i = 0; i < 5; i++) {
        for (j = 0; j <= 10; j++) {
            ad_progressBoxMultiUpdate(prog, i, j);
            sleep(0.1);
        }
    }

    ad_progressBoxDestroy(prog);

    ad_deinit();

    return 0;
}

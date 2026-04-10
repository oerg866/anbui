# AnbUI - The Burger Enjoyers' Minimal Text UI Library In C

This does pretty much what it says on the tin... It's a miniature text library in C!

It's designed with absolute ease of use in mind. One or two function calls to get the kind of TUI element you want.

## What can it do?

* Menus
    * Menus with arbitrary items, generated at run time
    * Yes/No Selectors
    * OK message boxes
* Text file display boxes
* Command output display boxes
* Progress bar boxes

A lot of functions support variadic arguments so you don't need to prepare strings to pass to it via temporary buffers and sprintfs.

As I said, minimal code & ease of use is the focus here.

## Limitations

* Probably currently not very thread-safe (at all (please don't))

# Platforms

## MS-DOS

### Open Watcom (`makefile.wcd`)

  Build test application with `wmake -f makefile.wcd`

### Microsoft C 7.00 (`makefile.c7d`)

  Build test application with `nmake makefile.c7d`

## Linux

### GCC

  `gcc -D_ANBUI_TEST_ -O3 -s -Wall -Wextra -pedantic -Werror -oanbui_test pl_linux.c ad_ui.c ad_obj.c ad_text.c ad_state.c anbui.c ad_test.c`

## Windows

### MinGW

  `gcc -D_ANBUI_TEST_ -O3 -s -Wall -Wextra -pedantic -Werror -oanbui_win.exe pl_win32.c ad_ui.c ad_obj.c ad_text.c ad_state.c anbui.c ad_test.c`

## API Reference

Please look at [`anbui.h`](anbui.h).

## Fixed Text Element size override

Some things have a fixed text element size, which, if the platform is known to require more or less than the default, can be overridden with a compilier define (`AD_TEXT_ELEMENT_SIZE_OVERRIDE`)

## Console state buffering

If you plan to overlay elements arbitrarily, you may want to save the screen state.

This is something the API allows for, but this can cause slowdowns on some platforms/systems.

If you wish to disable this for performance reasons, define `AD_DISABLE_CONSOLE_STATE`

## What's with the name...?

My partner plays a video game called Zenless Zone Zero. I It's not my type of game, but it has a character named Anby Demara. This character has an unholy obsession with burgers, which I relate to :D

That's where AnbUI and all the references come from.

## Screenshots

![](images/1.png)
![](images/2.png)
![](images/3.png)
![](images/4.png)
![](images/5.png)

## License

I've been a professional software engineer for quite a few years now and I still don't know sh*t about licensing.

CC-BY-NC 3.0 says use it, copy it, tell people who made it and don't sell it. Pretty please? :3
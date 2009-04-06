@echo off
@echo Makefile for compiling pineappletracker.exe with MinGW

set CC=gcc
set LIB_CURSES=-lpdcurses
set LIB_SDL=-lSDLmain -lSDL
set LIB_CACA=-lcaca

SET INCLUDES=-I.
SET DEFINES=-D_WINDOWS

set CFLAGS=-O2 %INCLUDES% %DEFINES%
SET LIBS=%LIB_CURSES% %LIB_SDL% %LIB_CACA%

set COMPILE_CMD=%CC% -c %CFLAGS%

erase *.o
%COMPILE_CMD% chip.c
rem figfont: mini
ECHO "\   _   /    _ |_  o ._     _"
ECHO " \ (_) /    (_ | | | |_) o (_"
ECHO "                     |       "

%COMPILE_CMD% gui.c %LIB_CURSES%
ECHO "\   _   /    _      o    _"
ECHO " \ (_) /    (_| |_| | o (_"
ECHO "             _|           "

%COMPILE_CMD% player.c %LIB_SDL% %LIB_CACA%
ECHO "\   _   /   ._  |  _.     _  ._   _"
ECHO " \ (_) /    |_) | (_| \/ (/_ | o (_"
ECHO "            |         /            "

%CC% -s -o player.exe *.o %LIB_SDL% %LIB_CACA%
rem figfont: speed
ECHO "___         _________        ______                           "
ECHO "__ \  ____________/_ ___________  ______ _____  ______________"
ECHO "___ \ _  __ ____/_/  ___  __ __  /_  __ `__  / / _  _ __  ___/"
ECHO "____ \/ /_/ __/_/    __  /_/ _  / / /_/ /_  /_/ //  ___  /    "
ECHO "______\____//_/      _  .___//_/  \__,_/ _\__, / \___//_/     "
ECHO "                     /_/                 /____/               "

%COMPILE_CMD% main.c %LIB_SDL%
ECHO "\   _   /   ._ _   _. o ._     _"
ECHO " \ (_) /    | | | (_| | | | o (_"

%CC% -s -o pineappletracker.exe main.o gui.o chip.o %LIB_SDL% %LIB_CURSES%
ECHO "___         _________                        _____ "
ECHO "__ \  ____________/_ __________________________  /_"
ECHO "___ \ _  __ ____/_/  ___  __ ___  __ ___  __ _  __/"
ECHO "____ \/ /_/ __/_/    __  /_/ __  /_/ __  /_/ / /_  "
ECHO "______\____//_/      _  .___/_  .___/_  .___/\__/  "
ECHO "                     /_/     /_/     /_/           "

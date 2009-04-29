@echo off
@echo Makefile for compiling pineappletracker.exe with MinGW

set CC=gcc
set LIB_CURSES=-lpdcurses
set LIB_SDL=-lSDLmain -lSDL
set LIB_CACA=-lcaca

set INCLUDES=-I.
set DEFINES=-DWINDOWS

set CFLAGS=-march=pentiumpro -mthreads -std=c99 -O2 %INCLUDES% %DEFINES%
set LIBS=%LIB_CURSES% %LIB_SDL% %LIB_CACA%

set COMPILE_CMD=%CC% -c %CFLAGS%

erase *.o


%COMPILE_CMD% actions.c
echo "\   _   /    _.  _ _|_ o  _  ._   _    _"
echo " \ (_) /    (_| (_  |_ | (_) | | _> o (_"

%COMPILE_CMD% modes.c
echo "\   _   /   ._ _   _   _|  _   _    _"
echo " \ (_) /    | | | (_) (_| (/_ _> o (_"

%COMPILE_CMD% gui.c
echo "\   _   /    _      o    _"
echo " \ (_) /    (_| |_| | o (_"
echo "             _|           "

%COMPILE_CMD% chip.c
rem figfont: mini
echo "\   _   /    _ |_  o ._     _"
echo " \ (_) /    (_ | | | |_) o (_"
echo "                     |       "

rem %COMPILE_CMD% player.c %LIB_SDL% %LIB_CACA%
rem echo "\   _   /   ._  |  _.     _  ._   _"
rem echo " \ (_) /    |_) | (_| \/ (/_ | o (_"
rem echo "            |         /            "

rem %CC% -s -o player.exe *.o %CFLAGS% %LIB_SDL% %LIB_CACA%
rem rem figfont: speed
rem echo ___         _________        ______                           
rem echo __ \  ____________/_ ___________  ______ _____  ______________
rem echo ___ \ _  __ ____/_/  ___  __ __  /_  __ `__  / / _  _ __  ___/
rem echo ____ \/ /_/ __/_/    __  /_/ _  / / /_/ /_  /_/ //  ___  /    
rem echo ______\____//_/      _  .___//_/  \__,_/ _\__, / \___//_/     
rem echo                      /_/                 /____/               

%COMPILE_CMD% main.c
echo "\   _   /   ._ _   _. o ._     _"
echo " \ (_) /    | | | (_| | | | o (_"

set WORKAROUND=-Wl,-u,_WinMain@16
%CC% -s -o pineappletracker.exe main.o gui.o chip.o modes.o actions.o %WORKAROUND% %CFLAGS% %LIB_SDL% %LIB_CURSES%
echo ___         _________                        _____ 
echo __ \  ____________/_ __________________________  /_
echo ___ \ _  __ ____/_/  ___  __ ___  __ ___  __ _  __/
echo ____ \/ /_/ __/_/    __  /_/ __  /_/ __  /_/ / /_  
echo ______\____//_/      _  .___/_  .___/_  .___/\__/  
echo                      /_/     /_/     /_/           

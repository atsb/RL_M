@echo off
echo Building Larn for DOS (DJGPP)

if not exist bin mkdir bin

echo.
echo Compiling and linking all sources
echo [CC] action.c bill.c config.c create.c data.c diag.c display.c fortune.c global.c help.c inventory.c io.c main.c monster.c moreobj.c movem.c nap.c object.c regen.c savelev.c scores.c spells.c spheres.c store.c tok.c

gcc -O2 -Wall -Wextra -IC:\<PATH>\pdcurses -DMULTIPLE_SCORE_ENTRY  ^
	action.c bill.c config.c create.c data.c diag.c display.c fortune.c ^
	global.c help.c inventory.c io.c main.c monster.c moreobj.c movem.c  ^
	nap.c object.c regen.c savelev.c scores.c spells.c spheres.c store.c  ^
	tok.c C:\<PATH>\pdcurses.a -o larn.exe

if errorlevel 1 (
    echo.
    echo Build FAILED.
    exit /b 1
)

echo.
echo Build complete: bin\larn.exe

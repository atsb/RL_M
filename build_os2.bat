@echo off
echo Building Larn for OS/2 (OpenWatcom)

if not exist bin mkdir bin

echo.
echo Compiling and linking all sources
echo [CC] action.c bill.c config.c create.c data.c diag.c display.c fortune.c global.c help.c inventory.c io.c main.c monster.c moreobj.c movem.c nap.c object.c regen.c savelev.c scores.c spells.c spheres.c store.c tok.c

wcl386 -bt=os2v2 -l=os2v2_pm -IC:\WATCOM\pdcurses -ox -w4 -we -wx -za -ei -fp3 ^
  -DMULTIPLE_SCORE_ENTRY ^
  action.c bill.c config.c create.c data.c diag.c display.c fortune.c ^
  global.c help.c inventory.c io.c main.c monster.c moreobj.c movem.c ^
  nap.c object.c regen.c savelev.c scores.c spells.c spheres.c store.c tok.c ^
  C:\WATCOM\pdcurses\os2\pdcurses.lib ^
  -fe=bin\larn.exe

if errorlevel 1 (
    echo.
    echo Build FAILED.
    exit /b 1
)

echo.
echo Build complete: bin\larn.exe

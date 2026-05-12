/*
* this is hackjob that translates the ANSI escape sequences used by larn
* to Curses API calls
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <curses.h>
#include "includes/ansiterm.h"



/********************************************
*              CURSES BACK-END             *
********************************************/

static int llgetch (void);

void
ansiterm_init (void)
{
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	start_color();
	use_default_colors();
	init_pair(1, COLOR_WHITE, COLOR_RED);
	curs_set(0);
	refresh();

#if defined PDC_KEY_MODIFIER_SHIFT
  PDC_save_key_modifiers (1);
#endif
}

void
ansiterm_clean_up (void)
{
  nocbreak ();
  nl ();
  echo ();
  endwin ();
}

/*
* get char
*/
int
ansiterm_getch (void)
{
  return llgetch ();
}

/*
* get char (with echo)
*/
int
ansiterm_getche (void)
{
  int key;
  echo ();
  key = llgetch();
  noecho ();
  return key;
}

/* wgetch() is the modern way. -Gibbon */
static int
llgetch (void)
{
  int key;
  key = wgetch(stdscr);

#ifdef PDC_KEY_MODIFIER_SHIFT
  if (PDC_get_key_modifiers () & PDC_KEY_MODIFIER_SHIFT)
    {
      switch (key)
	{
	case '1':
	  return 'B';
	case '2':
	  return 'J';
	case '3':
	  return 'N';
	case '4':
	  return 'H';
	case '5':
	  return '.';
	case '6':
	  return 'L';
	case '7':
	  return 'Y';
	case '8':
	  return 'K';
	case '9':
	  return 'U';
	}
    }
#endif
  switch (key)
    {
    case KEY_UP:
      return 'k';
    case KEY_DOWN:
      return 'j';
    case KEY_LEFT:
      return 'h';
    case KEY_RIGHT:
      return 'l';
    case KEY_A1:
      return 'y';
    case KEY_A3:
      return 'u';
    case KEY_C1:
      return 'b';
    case KEY_C3:
      return 'n';
    case KEY_B2:
      return '.';
    case KEY_ENTER:
      return 13;
    default:
      return key;
    }
}

void
ansiterm_delch (void)
{
  delch ();
}



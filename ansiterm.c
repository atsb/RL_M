/*
* this is hackjob that translates the ANSI escape sequences used by larn
* to Curses API calls
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <curses.h>
#include "includes/ansiterm.h"

#define ANSITERM_ESC	27

static void ansiterm_command (int, const char *, const char *);
static void ansiterm_putchar (int);
static void llrefresh (void);

/*
* writes to the terminal
*/
void
ansiterm_out (const char *output_buffer, int n_chars)
{
  int i;

  i = 0;

  while (i < n_chars)
    {

      if (output_buffer[i] == ANSITERM_ESC)
	{
	  char ansi_param[10];
	  char param1[5];
	  char param2[5];
	  int ansi_cmd;
	  int j;

	  *param1 = '\0';
	  *param2 = '\0';

	  ++i;
	  ++i;

	  j = 0;

	  while (!isalpha (output_buffer[i]))
	    {

	      ansi_param[j] = output_buffer[i];

	      ++i;
	      ++j;
	    }

	  ansi_param[j] = '\0';

	  ansi_cmd = output_buffer[i];
	  ++i;

	  if (*ansi_param != '\0')
	    {
	      char *p;
	      int k;

	      p = ansi_param;

	      k = 0;
	      while (*p != ';' && *p != '\0')
		{

		  param1[k] = *p;
		  ++p;
		  ++k;
		}
	      param1[k] = '\0';

	      if (*p == ';')
		++p;

	      k = 0;
	      while (*p != ';' && *p != '\0')
		{

		  param2[k] = *p;
		  ++p;
		  ++k;
		}

	      param2[k] = '\0';

	    }

	  ansiterm_command (ansi_cmd, param1, param2);

	}
      else
	{

	  ansiterm_putchar (output_buffer[i]);
	  ++i;
	}
    }

  /*
     ESC[;H
     ESC[y;xH   ESC[24;01H
     ESC[2J
     ESC[1m standout on
     ESC[m standout off
   */
  llrefresh ();
}

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

#if defined WINDOWS_VS
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

#ifdef WINDOWS_VS
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
#ifdef WINDOWS_VS
    case KEY_A2:
      return 'k';
    case KEY_B1:
      return 'h';
    case KEY_B3:
      return 'l';
    case KEY_C2:
      return 'j';
    case PADENTER:
      return 13;
#endif
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

static void
ansiterm_command (int ansi_cmd, const char *param1, const char *param2)
{
  if (ansi_cmd == 'H')
    {
      int y, x;

      if (*param1 == '\0')
	{

	  y = 0;

	}
      else
	{

	  y = atoi (param1) - 1;
	}

      if (*param2 == '\0')
	{

	  x = 0;

	}
      else
	{

	  x = atoi (param2) - 1;
	}

      move (y, x);

    }
  else if (ansi_cmd == 'J')
    {

      clear();

    }
  else if (ansi_cmd == 'M')
    {
      int i, n_lines;

      if (*param1 != '\0')
	{

	  n_lines = 1;

	}
      else
	{

	  n_lines = atoi (param1);
	}

      for (i = 0; i < n_lines; i++)
	{

	  move (0, 0);
	  clrtoeol ();
	}

    }
  else if (ansi_cmd == 'K')
    {

      clrtoeol ();

    }
  else if (ansi_cmd == 'm')
    {
      int attribute;

      if (*param1 == '\0')
	{

	  attribute = 0;

	}
      else
	{

	  attribute = atoi (param1);
	}

      if (attribute == 0)
	{

	  attrset (A_NORMAL);

	}
      else if (attribute == 1)
	{

	  attrset (A_REVERSE);
	}

    }
  else
    {

      exit (EXIT_FAILURE);
    }
}

static void
ansiterm_putchar (int c)
{
  if (c == '\n')
    {
      int y, x;
      getyx(stdscr, y, x);
      move(y + 1, x = 0);
      return;
    }
  if (c == '\t')
    {
      addstr ("    ");
      return;
    }
  addch(c);
}

void
llrefresh (void)
{
  refresh();
}

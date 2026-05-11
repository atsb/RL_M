/* io.c
*
*  setupvt100()    Subroutine to set up terminal in correct mode for game
*  clearvt100()    Subroutine to clean up terminal when the game is over
*  ttgetch()       Routine to read in one character from the terminal
*  scbr()          Function to set cbreak -echo for the terminal
*  sncbr()         Function to set -cbreak echo for the terminal
*  newgame()       Subroutine to save the initial time and seed rnd()
*
*  FILE OUTPUT ROUTINES
*
*  lprintf(format,args . . .)  printf to the output buffer
*  lprint(integer)         send binary integer to output buffer
*  lwrite(buf,len)         write a buffer to the output buffer
*  lprcat(str)         sent string to output buffer
*
*  FILE OUTPUT MACROS
*
*  lprc(character)         put the character into the output buffer
*
*  FILE INPUT ROUTINES
*
*  int lgetc()            read one character from input buffer
*  int larint()            read one integer from input buffer
*  lrfill(address,number)      put input bytes into a buffer
*  char *lgetw()           get a whitespace ended word from input
*  char *lgetl()           get a \n or EOF ended line from input
*
*  FILE OPEN / CLOSE ROUTINES
*
*  lcreat(filename)        create a new file for write
*  lopen(filename)         open a file for read
*  lappend(filename)       open for append to an existing file
*  lrclose()           close the input file
*  lwclose()           close output file
*  lflush()            flush the output buffer
*
*  Other Routines
*
*  cursor(x,y)     position cursor at [x,y]
*  cursors()       position cursor at [1,24] (saves memory)
*  cl_line(x,y)            Clear line at [1,y] and leave cursor at [x,y]
*  cl_up(x,y)          Clear screen from [x,1] to current line.
*  cl_dn(x,y)      Clear screen from [1,y] to end of display. 
*  lstandout(str)       Print the string in standout mode.
*  set_score_output()  Called when output should be literally printed.
** ttputch(ch)     Print one character in decoded output buffer.
** flush_buf()     Flush buffer with decoded output.
** init_term()     Terminal initialization
*
* Note: ** entries are available only in termcap mode.
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <curses.h>

#if defined WINDOWS_VS
#define _INC_CONIO
#include <io.h>
#endif

#include <sys/stat.h>
#include <setjmp.h>
#include <fcntl.h>		/* For O_BINARY */

#include "includes/larncons.h"
#include "includes/larndata.h"
#include "includes/larnfunc.h"
#include "includes/ansiterm.h"

#if defined NIX
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include "includes/display.h"
#include "includes/global.h"
#include "includes/io.h"
#include "includes/monster.h"
#include "includes/scores.h"
#include "includes/tgoto.h"

#define LINBUFSIZE 128		/* size of the lgetw() and lgetl() buffer       */
int lfd = 0;		/*  output file numbers     */
int fd;				/*  input file numbers      */
static int curx = 0;
static int cury = 0;



static int ipoint = MAXIBUF, iepoint = MAXIBUF;	/*  input buffering pointers    */
static char lgetwbuf[LINBUFSIZE];	/* get line (word) buffer               */


static int (*getchfn) (void);

static void flush_buf (void);

/*
*  Subroutine to set up terminal in correct mode for game
*
*  Attributes off, clear screen, set scrolling region, set tty mode 
*/
void
setupvt100 (void)
{

  screen_clear();

  setscroll ();

  scbr ();			/* system("stty cbreak -echo"); */
}



/*
*  Subroutine to clean up terminal when the game is over
*
*  Attributes off, clear screen, unset scrolling region, restore tty mode 
*/
void
clearvt100 (void)
{

  ansiterm_clean_up ();

  resetscroll ();

  /* clear(); *//* why does this routine need to clear() ? */

  sncbr ();			/* system("stty -cbreak echo"); */
}



/*
* ttgetch()       Routine to read in one character from the terminal
*/
char
ttgetch (void)
{
  char byt;

#ifdef EXTRA
  c[BYTESIN]++;
#endif

  lflush ();			/* be sure output buffer is flushed */

  byt = (char) (*getchfn) ();

  if (byt == '\r')
    {

      byt = '\n';
    }

  return byt;
}


/*
* scbr()      Function to set cbreak -echo for the terminal
*
* like: system("stty cbreak -echo")
*/
void
scbr (void)
{

  /* 
   * Set up to use the direct console input call which may
   * read from the keypad;
   */
  getchfn = ansiterm_getch;
  curs_set(0);
}



/*
* sncbr()     Function to set -cbreak echo for the terminal
*
* like: system("stty -cbreak echo")
*/
void
sncbr (void)
{

  /* 
   * Set up to use the direct console input call with echo, getche()
   */
  getchfn = ansiterm_getche;
  curs_set(1);
}


/*
* newgame()       Subroutine to save the initial time and seed rnd()
*/
void
newgame (void)
{
  long *p, *pe;

  for (p = c, pe = c + 100; p < pe; p++)
    *p = 0;

  time (&initialtime);

  srand ((unsigned)initialtime);

  lcreat ((char *) 0);		/* open buffering for output to terminal */
}



/*
*  lprintf(format,args . . .)      printf to the output buffer
*      char *format;
*      ??? args . . .
*
*  Enter with the format string in "format", as per printf() usage
*      and any needed arguments following it
*  Note: lprintf() only supports %s, %c and %d, with width modifier and left
*      or right justification.
*  No correct checking for output buffer overflow is done, but flushes 
*      are done beforehand if needed.
*  Returns nothing of value.
*/
void
lprintf (const char *fmt, ...)
{
  va_list vl;
  char buffer[STRING_BUFFER_SIZE];
  const char *p;

  va_start (vl, fmt);
  vsprintf (buffer, fmt, vl);
  va_end (vl);

  p = buffer;

  while (*p != '\0')
    {
      lprc (*p);
      ++p;
    }
}

/*
*  lprint(int-integer)                send binary integer to output buffer
*      int integer;
*
*      +---------+---------+---------+---------+
*      |   high  |         |         |   low   |
*      |  order  |         |         |  order  |
*      |   byte  |         |         |   byte  |
*      +---------+---------+---------+---------+
*     31  ---  24 23 --- 16 15 ---  8 7  ---   0
*
*  The save order is low order first, to high order (4 bytes total)
*      and is written to be system independent.
*  No checking for output buffer overflow is done, but flushes if needed!
*  Returns nothing of value.
*/
void
lprint (int x)
{

  if (lpnt >= lpend)
    {

      lflush ();
    }

  *lpnt++ = 255 & x;
  *lpnt++ = 255 & (x >> 8);
  *lpnt++ = 255 & (x >> 16);
  *lpnt++ = 255 & (x >> 24);

}


static int scrline = 18;	/* line # for wraparound instead of scrolling if no DL */

/* 
* output one byte to the output buffer 
*/
void
lprc (char ch)
{
    if (lfd > 2) {
      *lpnt++ = ch;

      if (lpnt >= lpend)
        {

          lflush ();
        }
    }
    else {
       if (ch == '\n' && (cury == 23) && enable_scroll) {
        if (++scrline > 23)
          {

        scrline = 19;
          }
          
        move(scrline+1, 0);
        clrtoeol();
        move(scrline, 0);
        clrtoeol();
       }
       else if (ch=='\t')
           addstr("    ");
       else
           addch(ch);
#ifdef EXTRA
       c[BYTESOUT]++;
#endif
    }
}







/*
*  lwrite(buf,len)         write a buffer to the output buffer
*      char *buf;
*      int len;
*  
*  Enter with the address and number of bytes to write out
*  Returns nothing of value
*/
void
lwrite (char *buf, int len)
{
  char *str;
  int num2;

  if (len > 399)		/* don't copy data if can just write it */
    {
#ifdef EXTRA
      c[BYTESOUT] += len;
#endif

      for (str = buf; len > 0; --len)
	lprc (*str++);
    }
  else
    while (len)
      {
	if (lpnt >= lpend)
	  lflush ();		/* if buffer is full flush it   */
	num2 = lpbuf + BUFBIG - lpnt;	/*  # bytes left in output buffer   */
	if (num2 > len)
	  num2 = len;
	str = lpnt;
	len -= num2;
	while (num2--)
	  *str++ = *buf++;	/* copy in the bytes */
	lpnt = str;
      }
}


/*
*  int lgetc()        Read one character from input buffer
*
*  Returns 0 if EOF, otherwise the character
*/
char
lgetc (void)
{
  int i;

  if (ipoint != iepoint)
    return (inbuffer[ipoint++]);
  if (iepoint != MAXIBUF)
    return (0);
#if defined WINDOWS_VS
	if ((i = _read(fd, inbuffer, MAXIBUF)) <= 0)
#endif
#if defined NIX
	if ((i = read(fd, inbuffer, MAXIBUF)) <= 0)
#endif
    {
      if (i != 0)
	fprintf (stderr, "error reading from input file\n");
      iepoint = ipoint = 0;
      return (0);
    }
  ipoint = 1;
  iepoint = i;
  return (*inbuffer);
}


/*
*  int larint()            Read one integer from input buffer
*
*      +---------+---------+---------+---------+
*      |   high  |         |         |   low   |
*      |  order  |         |         |  order  |
*      |   byte  |         |         |   byte  |
*      +---------+---------+---------+---------+
*     31  ---  24 23 --- 16 15 ---  8 7  ---   0
*
*  The save order is low order first, to high order (4 bytes total)
*  Returns the int read
*/
int
larint (void)
{
  int i;
  i = 255 & lgetc ();
  i |= (255 & lgetc ()) << 8;
  i |= (255 & lgetc ()) << 16;
  i |= (255 & lgetc ()) << 24;
  return (i);
}



/*
*  lrfill(address,number)          put input bytes into a buffer
*      char *address;
*      int number;
*
*  Reads "number" bytes into the buffer pointed to by "address".
*  Returns nothing of value
*/
void
lrfill (char *adr, int num)
{
  char *pnt;
  int num2;

  while (num)
    {
      if (iepoint == ipoint)
	{
	  if (num > 5)		/* fast way */
	    {
#if defined WINDOWS_VS
	if (_read(fd, adr, num) != num)
#endif
#if defined NIX
	if (read(fd, adr, num) != num)
#endif
		fprintf (stderr, "error reading from input file\n");
	      num = 0;
	    }
	  else
	    {
	      *adr++ = lgetc();
	      --num;
	    }
	}
      else
	{
	  num2 = iepoint - ipoint;	/*  # of bytes left in the buffer   */
	  if (num2 > num)
	    num2 = num;
	  pnt = inbuffer + ipoint;
	  num -= num2;
	  ipoint += num2;
	  while (num2--)
	    *adr++ = *pnt++;
	}
    }
}



/*
*  char *lgetw()           Get a whitespace ended word from input
*
*  Returns pointer to a buffer that contains word.  If EOF, returns a NULL
*/
char *
lgetw (void)
{
  char *lgp, cc;
  int n = LINBUFSIZE, quote = 0;

  lgp = lgetwbuf;
  do
    cc = lgetc ();
  while ((cc <= 32) && (cc > '\0'));	/* eat whitespace */
  for (;; --n, cc = lgetc ())
    {
      if ((cc == '\0') && (lgp == lgetwbuf))
	return (NULL);		/* EOF */
      if ((n <= 1) || ((cc <= 32) && (quote == 0)))
	{
	  *lgp = '\0';
	  return lgetwbuf;
	}
      if (cc != '"')
	*lgp++ = cc;
      else
	quote ^= 1;
    }
}



/*
*  char *lgetl()       Function to read in a line ended by newline or EOF
*
*  Returns pointer to a buffer that contains the line.  If EOF, returns NULL
*/
char *
lgetl (void)
{
  int i = LINBUFSIZE;
  char ch;
  char *str = lgetwbuf;

  for (;; --i)
    {
      *str++ = ch = lgetc ();
      if (ch == 0)
	{
	  if (str == lgetwbuf + 1)
	    return (NULL);	/* EOF */
	ot:*str = 0;
	  return (lgetwbuf);	/* line ended by EOF */
	}
      if ((ch == '\n') || (i <= 1))
	goto ot;		/* line ended by \n */
    }
}

/* Just for the record, I removed all those _setmodes for *nix
 * systems a while ago.  No need to declare these
 * things to make it work on them,
 * these systems just don't require it. ~Gibbon
*/

/*
*  lcreat(filename)            Create a new file for write
*      char *filename;
*
*  lcreat((char*)0); means to the terminal
*  Returns -1 if error, otherwise the file descriptor opened.
*  
*  Modernised this function and made it cleaner. ~Gibbon
*/
int
lcreat (char *str)
{
  lpnt = lpbuf;
  lpend = lpbuf + BUFBIG;
  if (str == NULL)
    return (lfd = 1);
#if defined WINDOWS_VS
		if ((lfd = _creat(str, S_IWRITE)) < 0)
#endif
#if defined NIX
	if ((lfd = open(str, O_RDWR | O_CREAT, 0666)) < 0)
#endif
	{
		lfd = 1;
		lprintf("error creating file <%s>\n", str);
		lflush();
		return(-1);
	}
#if defined WINDOWS_VS
  _setmode (lfd, O_BINARY);
#endif
  return lfd;
}



/*
*  lopen(filename)         Open a file for read
*      char *filename;
*
*  lopen(0) means from the terminal
*  Returns -1 if error, otherwise the file descriptor opened.
*/
int
lopen (char *str)
{
  ipoint = iepoint = MAXIBUF;

  if (str == NULL)
    return (fd = 0);
#if defined WINDOWS_VS
	if ((fd = _open(str, 0)) < 0)
#endif
#if defined NIX
	if ((fd = open(str, 0)) < 0)
#endif
    {
      lwclose ();
      lfd = 1;
      lpnt = lpbuf;
      return (-1);
    }
#if defined WINDOWS_VS
  _setmode (fd, O_BINARY);
#endif
  return fd;
}



/*
*  lappend(filename)       Open for append to an existing file
*      char *filename;
*
*  lappend(0) means to the terminal
*  Returns -1 if error, otherwise the file descriptor opened.
*/
int
lappend (char *str)
{
  lpnt = lpbuf;
  lpend = lpbuf + BUFBIG;
  if (str == NULL)
    return (lfd = 1);
#if defined WINDOWS_VS
	if ((lfd = _open(str, 2)) < 0)
#endif
#if defined NIX
	if ((lfd = open(str, 2)) < 0)
#endif
    {
      lfd = 1;
      return (-1);
    }
#if defined WINDOWS_VS
  _setmode (lfd, O_BINARY);
  _lseek (lfd, 0L, 2);		/* seek to end of file */
#endif
#if defined NIX
	lseek (lfd, 0L, 2);		/* seek to end of file */
#endif
  return lfd;
}

/*
*  lrclose()                       close the input file
*
*  Returns nothing of value.
*/
void
lrclose (void)
{
  if (fd > 0)
    {
#if defined WINDOWS_VS
		_close(fd);
#endif
#if defined NIX
		close(fd);
#endif
    }
}



/*
*  lwclose()                       close output file flushing if needed
*
*  Returns nothing of value.
*/
void
lwclose (void)
{
  lflush();
  if (lfd > 2)
    {
#if defined WINDOWS_VS
		_close(lfd);
#endif
#if defined NIX
		close(lfd);
#endif
    }
}

/*
*  lprcat(string)                  append a string to the output buffer
*                                  avoids calls to lprintf (time consuming)
*/
void
lprcat (char *str)
{
    if (lfd > 2) {
      char *str2;
      
      if (lpnt >= lpend)
        {
          lflush();
        }
      str2 = lpnt;

      while ((*str2++ = *str++) != '\0')
        ;
      lpnt = str2 - 1;
      lflush();
    }
    else {
        char* p = str;
        char c;
        while ((c = *p++)) {
            lprc(c);
        }
    }
}

/*
* cursor(x,y)    Put cursor at specified coordinates staring at [1,1] (termcap)
*/
void
cursor (int x, int y)
{
  curx = x-1;
  cury = y-1;
  move(cury, curx);
}

/*
*  Routine to position cursor at beginning of 24th line
*/
void
cursors (void)
{
  curs_set(0);
  cursor(1, 24);
}

/*
* Warning: ringing the bell is control code 7. Don't use in defines.
* Don't change the order of these defines.
* Also used in helpfiles. Codes used in helpfiles should be \E[1 to \E[7 with
* obvious meanings.
*/

/* translated output buffer */
static char *outbuf = NULL;


/*
* init_term()      Terminal initialization
*/
void
init_term (void)
{
  /* get memory for decoded output buffer */
  outbuf = malloc (BUFBIG + 16);
  if (outbuf == NULL)
    {
      fprintf (stderr, "Error malloc'ing memory for decoded output buffer\n");
      /* malloc() failure */
      died (-285);
    }
  ansiterm_init ();
}

/*
* cl_line(x,y)  Clear the whole line indicated by 'y' and leave cursor at [x,y]
*/
void
cl_line (int x, int y)
{
  move (y-1, 0);
  clrtoeol();
  cursor (x, y);
}

/*
* cl_up(x,y) Clear screen from [x,1] to current position. Leave cursor at [x,y]
*/
void
cl_up (int x, int y)
{
  int i;
  for (i = 1; i <= y; i++)
    {
        move (i-1,0);
        clrtoeol();
    }
  cursor (x, y);
}

/*
* cl_dn(x,y)   Clear screen from [1,y] to end of display. Leave cursor at [x,y]
*/
void
cl_dn (int x, int y)
{
  move (y-1,0);
  clrtobot();
  cursor (x, y);
}

/*
* lstandout(str)    Print the argument string in inverse video (standout mode).
*/
void
lstandout (char *str)
{
    attron(A_REVERSE);
    lprcat(str);
    attroff(A_REVERSE);
}

/*
* set_score_output()   Called when output should be literally printed.
*/
void
set_score_output (void)
{
	enable_scroll = -1;
}

/*
*  lflush()                        Flush the output buffer
*
*  Returns nothing of value.
*  for termcap version: Flush output in output buffer according to output
*                       status as indicated by `enable_scroll'
*/


void
lflush (void)
{
    int lpoint;
      if (lfd > 2)
	{
        if ((lpoint = lpnt - lpbuf) > 0) {
#ifdef EXTRA
        c[BYTESOUT] += lpoint;
#endif
#if defined WINDOWS_VS
            if (_write(lfd, lpbuf, lpoint) != lpoint)
#endif
#if defined NIX
            if (write(lfd, lpbuf, lpoint) != lpoint)
#endif
            {
              fprintf(stderr,"Error writing output file\n");
            }
          lpnt = lpbuf;		/* point back to beginning of buffer */
            }
	  flush_buf(); 

	  /* Catch write errors on save files
	   */
	  return;
	}
    else {
        refresh();
    }
}

static int io_index = 0;


/*
* flush_buf()          Flush buffer with decoded output.
*/
static void
flush_buf (void)
{
      if (lfd > 2)
	{
#if defined WINDOWS_VS
		_write(lfd, outbuf, io_index);
#endif
#if defined NIX
		write(lfd, outbuf, io_index);
#endif
	}
    io_index = 0;
}

/*
*  flushall()  Function to flush all type-ahead in the input buffer
*  
*/

void
lflushall (void)
{
    flushinp();
}



void
enter_name (void)
{
  int i;

  lprcat ("\n\nEnter character name:\n");

  sncbr ();

  i = 0;

  do
    {
      char c;

      c = ttgetch ();

      if (c == '\n')
	break;
      if (c == 8)
	{
	  if (i > 0)
	    {
	      --i;
	      ansiterm_delch ();
	    }
	}
      else if (isprint (c))
	{
	  logname[i] = c;
	  ++i;
	}

    }
  while (i < LOGNAMESIZE - 1);

  logname[i] = '\0';

  scbr ();
}

void
cursor_block(void)
{
    curs_set(0);
    attron(A_REVERSE);
    addch(' ');
    attroff(A_REVERSE);
}
